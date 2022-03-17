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

World::TileLoc World::NextTileInInputDirection(World::Direction direct, World::TileLoc tile)
{
    switch (direct)
    {
    case World::Direction::EAST:
        tile.x++;
        break;
    case World::Direction::WEST:
        tile.x--;
        break;
    case World::Direction::NORTH:
        tile.z++;
        break;
    case World::Direction::SOUTH:
        tile.z--;
        break;
    default:
        break;
    }
    return tile;
}

World::RetrievePermission World::retrievePermission(World::LevelID level, World::Direction direction, World::TileLoc loc)
{
    std::vector<World::MovementPermissions>* permissions = World::Level::queryPermissions(level);
    World::LevelDimensions dimensions = World::Level::queryDimensions(level);
    World::TileLoc tileLookup = World::NextTileInInputDirection(direction, loc);

    //if outside level bounds, check current tile instead
    bool leavingLevel = false;
    if (tileLookup.x < 0 || tileLookup.x >= dimensions.levelW || tileLookup.z < 0 || tileLookup.z >= dimensions.levelH)
    {
        tileLookup.x = loc.x;
        tileLookup.z = loc.z;
        leavingLevel = true;
    }

    //Lookup permission for next tile
    unsigned int lookupIndex = tileLookup.x * dimensions.levelH + tileLookup.z;
    return { World::Level::queryPermissions(level)->at(lookupIndex), leavingLevel };
}

void World::ModifyTilePerm(World::LevelID level, World::Direction direction, World::TileLoc loc)
{
    //Get level data
    World::LevelDimensions dim = World::Level::queryDimensions(level);
    std::vector<World::MovementPermissions>* perm = World::Level::queryPermissions(level);

    //Clear tile leaving
    unsigned int index = loc.x * dim.levelH + loc.z;
    perm->at(index) = World::MovementPermissions::CLEAR;

    //Block tile entering
    World::TileLoc tileLookup = World::NextTileInInputDirection(direction, { loc.x, loc.z });
    index = tileLookup.x * dim.levelH + tileLookup.z;
    perm->at(index) = World::MovementPermissions::SPRITE_BLOCKING;
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

std::unordered_map<World::LevelID, std::vector<World::MovementPermissions>*> World::Level::s_MovementPermissionsCache;

//Level - data is defined back to front with top left being 0,0 not bottom left
void World::Level::buildLevel(Render::Renderer<TextureVertex>* planeRenderer, TileMap* tileMapPointer)
{
    LevelData data = ParseLevel();
    m_TileMapPointer = tileMapPointer;

    //Allocate permission array
    m_LevelTilesX = data.width; m_LevelTilesY = data.height;
    m_LevelTotalTiles = m_LevelTilesX * m_LevelTilesY;
    m_Permissions.resize(m_LevelTotalTiles);

    //Allocate height array
    m_Heights.resize(m_LevelTotalTiles);

    //Create plane
    m_Plane.generatePlaneXZ(data.originX, data.originY, data.width * World::TILE_SIZE, data.height * World::TILE_SIZE, World::TILE_SIZE);
    m_Plane.setRenderer(planeRenderer);

    //Texture all with tex 0,0
    UVData texData = m_TileMapPointer->uvTile(0, 0);
    m_Plane.texturePlane(texData.uvX, texData.uvY, texData.uvWidth, texData.uvHeight);

    unsigned int xFirstIndex = 0;
    unsigned int yFirstIndex = 0;
    for (unsigned int y = 0; y < m_LevelTilesY; y++) {
        for (unsigned int x = 0; x < m_LevelTilesX; x++) {
            //Calculate index for an x first search and a y first search
            xFirstIndex = x * m_LevelTilesY + y;
            yFirstIndex = y * m_LevelTilesX + x;

            //Do heights for every tile, other things can init to 0
            if (!(xFirstIndex >= m_LevelTotalTiles)) {
                //Adjust heights
                m_Heights[xFirstIndex] = data.planeHeights[yFirstIndex];
                tileLevel(m_Plane.accessQuad(x, y), m_Heights[xFirstIndex]);
            }

            //Slope tiles
            if (data.planeDirections[yFirstIndex] != World::Direction::DIRECTION_NULL) {
                World::SlopeTile(m_Plane.accessQuad(x, y), data.planeDirections[yFirstIndex]);
            }

            //Permissions
            if (data.planePermissions[yFirstIndex] != World::MovementPermissions::CLEAR) {
                m_Permissions[xFirstIndex] = data.planePermissions[yFirstIndex];
            }

            //Texture tiles
            TileTexture tileTex = data.planeTextures[yFirstIndex];
            if (!(tileTex.textureX == 0 && tileTex.textureY == 0)) {
                texData = m_TileMapPointer->uvTile(tileTex.textureX, tileTex.textureY);
                SetQuadUV((TextureVertex*)m_Plane.accessQuad(x, y), texData.uvX, texData.uvY, texData.uvWidth, texData.uvHeight);
            }
        }
    }

    //Store pointer to permissions - updates every time level loaded
    Level::s_MovementPermissionsCache[this->m_ID] = &this->m_Permissions;
}

void World::Level::render() {
    m_Plane.render();
}

//Declare unordered maps to cache constant level values
std::unordered_map<World::LevelID, Component2f> World::Level::s_LevelOriginCache;
std::unordered_map<World::LevelID, World::LevelDimensions> World::Level::s_LevelDimensionCache;

World::LevelData World::ParseLevel() {
    //Setup stream
    std::ifstream ifs("res/level/level0.json");
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);

    //Info
    LevelID id;
    unsigned int width; unsigned int height;
    float levelOX; float levelOZ;

    //Vectors
    std::vector<WorldLevel> planeHeights;
    std::vector<Direction> planeDirections;
    std::vector<MovementPermissions> planePermissions;
    std::vector<TileTexture> planeTextures;

    //Assertions are to check if is a valid level file
    assert(doc.HasMember("levelID"));
    assert(doc.HasMember("levelWidth"));
    assert(doc.HasMember("levelHeight"));
    assert(doc.HasMember("levelOriginX"));
    assert(doc.HasMember("levelOriginZ"));
    assert(doc.HasMember("planeHeightsR0"));
    assert(doc.HasMember("planeDirectionsR0"));
    assert(doc.HasMember("planePermissionsR0"));
    assert(doc.HasMember("planeTexturesR0"));

    //Get simple values
    id = (LevelID)doc["levelID"].GetInt();
    width = doc["levelWidth"].GetInt();
    height = doc["levelHeight"].GetInt();
    levelOX = doc["levelOriginX"].GetFloat();
    levelOZ = doc["levelOriginZ"].GetFloat();

    //Read in

    //const properties tags per row
    const std::string HEIGHT_ROW = "planeHeightsR";
    const std::string DIRECTIONS_ROW = "planeDirectionsR";
    const std::string PERMISSIONS_ROW = "planePermissionsR";
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

    //Read permissions
    for (int y = height - 1; y >= 0; y--) {
        //Make member name string
        currentRow = PERMISSIONS_ROW + std::to_string(y);
        //Check member exists
        assert(doc.HasMember(currentRow.c_str()));
        //Reads data
        std::stringstream stream(doc[currentRow.c_str()].GetString());
        while (std::getline(stream, currentSegment, '|'))
        {
            planePermissions.push_back((MovementPermissions)std::stoi(currentSegment));
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
            int posY = pos + 1;
            int tileY = std::stoi(currentSegment.substr(posY));
            int tileX = std::stoi(currentSegment.substr(0, pos));

            planeTextures.push_back({(unsigned int)tileX, (unsigned int)tileY});
        }
    }
    //Store level origin if not loaded before
    if (Level::s_LevelOriginCache.find(id) == Level::s_LevelOriginCache.end()) {
        Level::s_LevelOriginCache[id] = { levelOX, levelOZ };
    }

    //Store level size if not loaded before
    if (Level::s_LevelDimensionCache.find(id) == Level::s_LevelDimensionCache.end()) {
        Level::s_LevelDimensionCache[id] = { width, height };
    }

    LevelData data = { id, width, height, levelOX, levelOZ, planeHeights, planeDirections, planePermissions, planeTextures };
    return data;
}