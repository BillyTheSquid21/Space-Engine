#include "game/level/World.h"

//World functions
static void RotateTileCorner(TextureQuad* quad, float angle) {
    TextureVertex v0 = quad->at(0);
    float x = v0.position.a + (World::TILE_SIZE / 2);
    float z = v0.position.c + (World::TILE_SIZE / 2);
    RotateShape<TextureVertex>(quad, { x, 0.0f, z }, angle, Shape::QUAD, Axis::Y);
}

void World::tileLevel(TextureQuad* quad, WorldLevel level) {
    TranslateShape<TextureVertex>((void*)quad, 0.0f, ((float)level / sqrt(2)) * World::TILE_SIZE, 0.0f, Shape::QUAD);
}

void World::SlopeTile(TextureQuad* quad, World::Direction direction) {
    //Get index's to slope - 3 is max
    unsigned int verticeIndex[3];
    unsigned int verticesToSlope = 0;

    switch (direction)
    {
    case Direction::NORTH:
        verticeIndex[0] = 0;
        verticeIndex[1] = 1;
        verticesToSlope = 2;
        break;
    case Direction::SOUTH:
        verticeIndex[0] = 2;
        verticeIndex[1] = 3;
        verticesToSlope = 2;
        break;
    case Direction::EAST:
        verticeIndex[0] = 1;
        verticeIndex[1] = 2;
        verticesToSlope = 2;
        break;
    case Direction::WEST:
        verticeIndex[0] = 0;
        verticeIndex[1] = 3;
        verticesToSlope = 2;
        break;
    //To keep slope consistency, all corners will be northwest rotated
    //Due to how quads are formed from tri's
    case Direction::NORTHWEST:
        verticeIndex[0] = 0;
        verticesToSlope = 1;
        break;
    case Direction::NORTHEAST:
        verticeIndex[0] = 0;
        verticesToSlope = 1;
        //Rotate
        RotateTileCorner(quad, -90.0f);
        break;
    case Direction::SOUTHWEST:
        verticeIndex[0] = 0;
        verticesToSlope = 1;
        RotateTileCorner(quad, -270.0f);
        break;
    case Direction::SOUTHEAST:
        verticeIndex[0] = 0;
        verticesToSlope = 1;
        RotateTileCorner(quad, -180.0f);
        break;
    default:
        verticesToSlope = 0;
        break;
    }

    for (int i = 0; i < verticesToSlope; i++) {
        TranslateShapeVertex<TextureVertex>((void*)quad, verticeIndex[i], 0.0f, TILE_SIZE / sqrt(2), 0.0f);
    }
}

//Level - data is defined back to front with top left being 0,0 not bottom left
void World::Level::buildLevel(unsigned int tilesX, unsigned int tilesY, Renderer<TextureVertex>* planeRenderer, TileMap* tileMapPointer)
{
    m_TileMapPointer = tileMapPointer;

    //Allocate permission array
    m_LevelTilesX = tilesX; m_LevelTilesY = tilesY;
    m_LevelTotalTiles = m_LevelTilesX * m_LevelTilesY;
    m_Permissions.resize(m_LevelTotalTiles);

    //Allocate height array
    m_Heights.resize(m_LevelTotalTiles);

    //Create plane
    plane.generatePlaneXZ(m_XOffset, m_YOffset, tilesX * World::TILE_SIZE, tilesY * World::TILE_SIZE, World::TILE_SIZE);
    plane.setRenderer(planeRenderer);

    //Texture all with tex 0,0
    UVData texData = m_TileMapPointer->uvTile(0, 0);
    plane.texturePlane(texData.uvX, texData.uvY, texData.uvWidth, texData.uvHeight);
   
    //For now fill permissions with clear
    //For now load level 0
    LevelData data = ParseLevel();

    unsigned int xFirstIndex = 0;
    unsigned int yFirstIndex = 0;
    for (unsigned int y = 0; y < m_LevelTilesY; y++) {
        for (unsigned int x = 0; x < m_LevelTilesX; x++) {
            //Calculate index for an x first search and a y first search
            xFirstIndex = x * m_LevelTilesY + y;
            yFirstIndex = y * m_LevelTilesX + x;

            //Fill permissions with clear - TODO make take input
            if (!(xFirstIndex >= m_LevelTotalTiles)) {
                m_Permissions[xFirstIndex] = World::MovementPermissions::CLEAR;

                //Adjust heights
                m_Heights[xFirstIndex] = data.planeHeights[yFirstIndex];
                tileLevel(plane.accessQuad(x, y), m_Heights[xFirstIndex]);
            }

            //Slope tiles
            if (data.planeDirections[yFirstIndex] != World::Direction::DIRECTION_NULL) {
                World::SlopeTile(plane.accessQuad(x, y), data.planeDirections[yFirstIndex]);
            }

            //Texture tiles
            TileTexture tileTex = data.planeTextures[yFirstIndex];
            if (!(tileTex.textureX == 0 && tileTex.textureY == 0)) {
                texData = m_TileMapPointer->uvTile(tileTex.textureX, tileTex.textureY);
                SetQuadUV((TextureVertex*)plane.accessQuad(x, y), texData.uvX, texData.uvY, texData.uvWidth, texData.uvHeight);
            }
        }
    }
}

void World::Level::render() {
    plane.render();
}

World::LevelData World::ParseLevel() {
    //Setup stream
    std::ifstream ifs("res/level/level0.json");
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    //Info
    LevelID id;
    unsigned int width; unsigned int height;

    //Vectors
    std::vector<WorldLevel> planeHeights;
    std::vector<Direction> planeDirections;
    std::vector<TileTexture> planeTextures;

    //Assertions are to check if is a valid level file
    assert(doc.HasMember("levelID"));
    assert(doc.HasMember("levelWidth"));
    assert(doc.HasMember("levelHeight"));
    assert(doc.HasMember("planeHeightsR0"));
    assert(doc.HasMember("planeDirectionsR0"));
    assert(doc.HasMember("planeTexturesR0"));

    //Get simple values
    id = (LevelID)doc["levelID"].GetInt();
    width = doc["levelWidth"].GetInt();
    height = doc["levelHeight"].GetInt();

    //Read in

    //const properties tags per row
    const std::string HEIGHT_ROW = "planeHeightsR";
    const std::string DIRECTIONS_ROW = "planeDirectionsR";
    const std::string TEXTURES_ROW = "planeTexturesR";

    //Current row and segments
    std::string currentRow;
    std::string currentSegment;

    //Read heights - Order for reading goes up the list
    for (int y = height - 1; y >= 0; y--) {
        //Make member name string
        currentRow = HEIGHT_ROW + std::to_string(y);
        //Check member exists
        assert(doc.HasMember(currentRow.c_str()));
        //Reads data
        std::stringstream stream(doc[currentRow.c_str()].GetString());
        while (std::getline(stream, currentSegment, '|'))
        {
            planeHeights.push_back((WorldLevel)std::stoi(currentSegment));
        }
    }

    //Read directions
    for (int y = height - 1; y >= 0; y--) {
        //Make member name string
        currentRow = DIRECTIONS_ROW + std::to_string(y);
        //Check member exists
        assert(doc.HasMember(currentRow.c_str()));
        //Reads data
        std::stringstream stream(doc[currentRow.c_str()].GetString());
        while (std::getline(stream, currentSegment, '|'))
        {
            planeDirections.push_back((Direction)std::stoi(currentSegment));
        }
    }

    //Read textures
    for (int y = height - 1; y >= 0; y--) {
        //Make member name string
        currentRow = TEXTURES_ROW + std::to_string(y);
        //Check member exists
        assert(doc.HasMember(currentRow.c_str()));
        //Reads data
        std::stringstream stream(doc[currentRow.c_str()].GetString());
        while (std::getline(stream, currentSegment, '|'))
        {
            int pos = currentSegment.find_first_of('-');
            int tileY = std::stoi(currentSegment.substr(pos + 1));
            int tileX = std::stoi(currentSegment.substr(0, pos));

            planeTextures.push_back({(unsigned int)tileX, (unsigned int)tileY});
        }
    }
    LevelData data = { id, width, height, planeHeights, planeDirections, planeTextures };
    return data;
}