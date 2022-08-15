#include "game/level/World.h"

//World functions
static void RotateTileCorner(Norm_Tex_Quad* quad, float angle) {
    NormalTextureVertex v0 = quad->at(0);
    float x = v0.position.x + (World::TILE_SIZE / 2);
    float z = v0.position.z + (World::TILE_SIZE / 2);
    AxialRotate<NormalTextureVertex>(quad, { x, 0.0f, z }, angle, Shape::QUAD, Axis::Y);
}

void World::tileLevel(Norm_Tex_Quad* quad, WorldHeight level) {
    Translate<NormalTextureVertex>((void*)quad, 0.0f, ((float)level / sqrt(2)) * World::TILE_SIZE, 0.0f, Shape::QUAD);
}

World::Direction World::GetDirection(std::string dir) 
{
    if (dir == "NORTH")
    {
        return World::Direction::NORTH;
    }
    else if (dir == "SOUTH")
    {
        return World::Direction::SOUTH;
    }
    else if (dir == "EAST")
    {
        return World::Direction::EAST;
    }
    else if (dir == "WEST")
    {
        return World::Direction::WEST;
    }
    return World::Direction::SOUTH; //default
}

World::Tile World::NextTileInDirection(World::Direction direct, World::Tile tile)
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

World::Direction World::DirectionOfAdjacentTile(World::Tile src, World::Tile dest)
{
    World::Tile result = src - dest;
    if (result.x >= 1)
    {
        return World::Direction::WEST;
    }
    else if (result.x <= -1)
    {
        return World::Direction::EAST;
    }
    else if (result.z >= 1)
    {
        return World::Direction::SOUTH;
    }
    else if (result.z <= -1)
    {
        return World::Direction::NORTH;
    }
    return World::Direction::DIRECTION_NULL;
}

bool World::CheckPlayerInteracting(World::Tile player, World::Tile script, World::Direction playerFacing)
{
    World::Tile nextTile = NextTileInDirection(playerFacing, player);
    if (script.x == nextTile.x && script.z == nextTile.z)
    {
        return true;
    }
    return false;
}

World::Direction World::OppositeDirection(World::Direction dir)
{
    switch (dir)
    {
    case World::Direction::EAST:
        return World::Direction::WEST;
    case World::Direction::WEST:
        return World::Direction::EAST;
    case World::Direction::NORTH:
        return World::Direction::SOUTH;
    case World::Direction::SOUTH:
        return World::Direction::NORTH;
    }
    return World::Direction::DIRECTION_NULL;
}

//Retrive in dir
World::LevelPermission World::RetrievePermission(World::LevelID level, World::Direction direction, World::Tile loc, WorldHeight height)
{
    Level::PermVectorFragment permissions = World::Level::queryPermissions(level, height);
    World::LevelDimensions dimensions = World::Level::queryDimensions(level);
    World::Tile tileLookup = World::NextTileInDirection(direction, loc);

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
    return { permissions.pointer[lookupIndex], &permissions.pointer[lookupIndex], leavingLevel };
}

//Retrive on spot
World::LevelPermission World::RetrievePermission(World::LevelID level, World::Tile loc, WorldHeight height)
{
    Level::PermVectorFragment permissions = World::Level::queryPermissions(level, height);
    World::LevelDimensions dimensions = World::Level::queryDimensions(level);

    //if outside level bounds, check current tile instead
    bool leavingLevel = false;
    if (loc.x < 0 || loc.x >= dimensions.levelW || loc.z < 0 || loc.z >= dimensions.levelH)
    {
        loc.x = loc.x;
        loc.z = loc.z;
        leavingLevel = true;
    }

    //Lookup permission for next tile
    unsigned int lookupIndex = loc.x * dimensions.levelH + loc.z;
    return { permissions.pointer[lookupIndex], &permissions.pointer[lookupIndex], leavingLevel };
}

void World::ModifyTilePerm(World::LevelID level, World::Direction direction, World::Tile loc, WorldHeight height, MovementPermissions& lastPerm, MovementPermissions*& lastPermPtr)
{
    //Ensure data is not nullptr
    if (!lastPermPtr)
    {
        return;
    }

    //Get level data
    World::LevelDimensions dim = World::Level::queryDimensions(level);
    Level::PermVectorFragment perm = World::Level::queryPermissions(level, height);

    //Clear tile leaving
    *lastPermPtr = lastPerm;

    //Block tile entering
    World::Tile tileLookup = World::NextTileInDirection(direction, { loc.x, loc.z });
    World::MovementPermissions* permission = World::GetTilePermission(level, tileLookup, height);
    lastPerm = *permission;
    lastPermPtr = permission;
    *permission = World::MovementPermissions::SPRITE_BLOCKING;
}

World::MovementPermissions* World::GetTilePermission(World::LevelID level, World::Tile loc, WorldHeight height)
{
    World::LevelDimensions dim = World::Level::queryDimensions(level);
    World::Level::PermVectorFragment perm = World::Level::queryPermissions(level, height);
    
    if (loc.x < 0)
    {
        loc.x = 0;
    }
    else if (loc.x >= dim.levelW)
    {
        loc.x = dim.levelW - 1;
    }

    if (loc.z < 0)
    {
        loc.z = 0;
    }
    else if (loc.z >= dim.levelW)
    {
        loc.z = dim.levelH - 1;
    }

    unsigned int index = loc.x * dim.levelH + loc.z;
    return &perm.pointer[index];
}

World::Level::PermVectorFragment World::Level::queryPermissions(LevelID level, WorldHeight height)
{
    if ((unsigned int)level < s_MovementPermissionsCache.size()) 
    { 
        Level::LevelPtrCache ptrCache = s_MovementPermissionsCache[((unsigned int)level)];
        for (int i = 0; i < ptrCache.levels->size(); i++)
        {
            if (height == ptrCache.levels->at(i))
            {
                World::LevelDimensions dim = queryDimensions(level);
                unsigned int size = dim.levelH * dim.levelW;
                unsigned int index = size * i;
                return { &ptrCache.perms->at(index), size };
            }
        }
    } 
    EngineLog("Level permissions not found: ", (unsigned int)level);
    return {&s_MovementPermissionsCache[0].perms->at(0), 1};  //If fails, return first level permis found
}

std::vector<World::MovementPermissions>* World::Level::getPermissions(LevelID level)
{
    if ((unsigned int)level < s_MovementPermissionsCache.size())
    {
        Level::LevelPtrCache ptrCache = s_MovementPermissionsCache[((unsigned int)level)];
        return ptrCache.perms;
    }
    EngineLog("Level permissions not found: ", (unsigned int)level);
    return s_MovementPermissionsCache[0].perms; //If fails return first element
}

void World::SlopeTile(Norm_Tex_Quad* quad, World::Direction direction) {
    //Get index's to slope - 3 is max
    unsigned int verticeIndex[3];
    unsigned int verticesToSlope = 0;

    //To keep slope consistency, all corners will be north/northwest and rotated
    //Due to how quads are formed from tri's
    switch (direction)
    {
    case Direction::NORTH:
        verticeIndex[0] = 0;
        verticeIndex[1] = 1;
        verticesToSlope = 2;
        break;
    case Direction::SOUTH:
        verticeIndex[0] = 0;
        verticeIndex[1] = 1;
        verticesToSlope = 2;
        RotateTileCorner(quad, -180.0f);
        break;
    case Direction::EAST:
        verticeIndex[0] = 0;
        verticeIndex[1] = 1;
        verticesToSlope = 2;
        RotateTileCorner(quad, -90.0f);
        break;
    case Direction::WEST:
        verticeIndex[0] = 0;
        verticeIndex[1] = 1;
        verticesToSlope = 2;
        RotateTileCorner(quad, -270.0f);
        break;
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
    case Direction::NORTHWEST_WRAPPED:
        verticeIndex[0] = 0;
        verticeIndex[1] = 1;
        verticeIndex[2] = 3;
        verticesToSlope = 3;
        break;
    case Direction::NORTHEAST_WRAPPED:
        verticeIndex[0] = 0;
        verticeIndex[1] = 1;
        verticeIndex[2] = 3;
        verticesToSlope = 3;
        //Rotate
        RotateTileCorner(quad, -90.0f);
        break;
    case Direction::SOUTHEAST_WRAPPED:
        verticeIndex[0] = 0;
        verticeIndex[1] = 1;
        verticeIndex[2] = 3;
        verticesToSlope = 3;
        //Rotate
        RotateTileCorner(quad, -180.0f);
        break;
    case Direction::SOUTHWEST_WRAPPED:
        verticeIndex[0] = 0;
        verticeIndex[1] = 1;
        verticeIndex[2] = 3;
        verticesToSlope = 3;
        //Rotate
        RotateTileCorner(quad, -270.0f);
        break;
    default:
        verticesToSlope = 0;
        break;
    }

    for (int i = 0; i < verticesToSlope; i++) {
        TranslateVertex<NormalTextureVertex>((void*)quad, verticeIndex[i], 0.0f, TILE_SIZE / sqrt(2), 0.0f);
    }
}

std::vector<World::Level::LevelPtrCache> World::Level::s_MovementPermissionsCache;
bool World::Level::s_CacheInit = false;

//Level - data is defined back to front with top left being 0,0 not bottom left
bool World::Level::buildLevel(Render::Renderer<NormalTextureVertex>* planeRenderer, TileMap* tileMapPointer)
{
    //Checks if the caches are setup - should happen on first load which shouldn't need to be atomic (not loading through loading zone)
    if (!s_CacheInit)
    {
        s_LevelOriginCache.resize((unsigned int)World::LevelID::LEVEL_NULL);
        s_LevelDimensionCache.resize((unsigned int)World::LevelID::LEVEL_NULL);
        s_MovementPermissionsCache.resize((unsigned int)World::LevelID::LEVEL_NULL);
        s_CacheInit = true;
    }

    LevelData data = ParseLevel(m_ID);
    m_TileMapPointer = tileMapPointer;

    //Allocate permission array
    m_LevelTilesX = data.width; m_LevelTilesY = data.height;
    m_LevelTotalTiles = m_LevelTilesX * m_LevelTilesY;

    //Permissions
    m_Permissions.resize(m_LevelTotalTiles * data.presentWorldLevels.size());
    m_AvailibleLevels = data.presentWorldLevels;

    //Allocate height array
    m_Heights.resize(m_LevelTotalTiles);

    //Create plane
    m_Plane.generatePlaneXZ(data.originX, data.originY, data.width * World::TILE_SIZE, data.height * World::TILE_SIZE, World::TILE_SIZE);
    m_Plane.setRenderer(planeRenderer);

    //Texture all with tex 0,0
    TileUV texData = m_TileMapPointer->uvTile(0, 0);
    m_Plane.texturePlane(texData.u, texData.v, texData.width, texData.height);

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
            for (int i = 0; i < data.presentWorldLevels.size(); i++)
            {
                WorldHeight worldLevel = data.presentWorldLevels[i];
                unsigned int yInd = (i * m_LevelTotalTiles) + yFirstIndex;
                unsigned int xInd = (i * m_LevelTotalTiles) + xFirstIndex;
                if (data.planePermissions[yInd] != World::MovementPermissions::CLEAR) {
                    m_Permissions.at(xInd) = data.planePermissions.at(yInd);
                }
            }

            //Texture tiles
            TileTexture tileTex = data.planeTextures[yFirstIndex];
            if (!(tileTex.textureX == 0 && tileTex.textureY == 0)) {
                texData = m_TileMapPointer->uvTile(tileTex.textureX, tileTex.textureY);
                SetQuadUV((NormalTextureVertex*)m_Plane.accessQuad(x, y), texData.u, texData.v, texData.width, texData.height);
            }
        }
    }
    m_Plane.generatePlaneNormals();

    //Store pointer to permissions - updates every time level loaded
    Level::s_MovementPermissionsCache[(unsigned int)m_ID] = { &m_Permissions, &m_AvailibleLevels };
    return true;
}

void World::Level::purgeLevel()
{
    m_Plane.purgeData();
    m_Permissions.clear();
    m_Heights.clear();
    World::Level::s_MovementPermissionsCache[(unsigned int)m_ID] = {nullptr, nullptr};
    m_TileMapPointer = nullptr;
}

void World::Level::render() {
    m_Plane.render();
}

//Declare unordered maps to cache constant level values
std::vector<Struct2f> World::Level::s_LevelOriginCache;
std::vector<World::LevelDimensions> World::Level::s_LevelDimensionCache;

World::LevelData World::ParseLevel(World::LevelID id) {
    //Setup stream
    const std::string path = "res/level/level";
    const std::string ext = ".json";

    std::ifstream ifs(path + std::to_string((int)id) + ext);
    rapidjson::IStreamWrapper isw(ifs);
    rapidjson::Document doc;
    doc.ParseStream(isw);
    ifs.close();

    //Info
    unsigned int width; unsigned int height;
    float levelOX; float levelOZ;

    //Vectors
    std::vector<WorldHeight> planeHeights;
    std::vector<Direction> planeDirections;
    std::vector<MovementPermissions> planePermissions;
    std::vector<TileTexture> planeTextures;

    //Assertions are to check if is a valid level file
    assert(doc.HasMember("levelID"));
    assert(doc.HasMember("levelWidth"));
    assert(doc.HasMember("levelHeight"));
    assert(doc.HasMember("levelOriginX"));
    assert(doc.HasMember("levelOriginZ"));
    assert(doc.HasMember("worldLevels")); //Tells you what levels have permissions
    assert(doc.HasMember("planeHeightsR0"));
    assert(doc.HasMember("planeDirectionsR0"));
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
    const std::string PERMISSIONS_ROW = "PlanePermissionsR";
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
            planeHeights.push_back((WorldHeight)std::stoi(currentSegment));
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

    //Get which elevations have permissions
    std::vector<WorldHeight> worldLevels; //Levels with permissions
    std::stringstream worldLevelStream(doc["worldLevels"].GetString());
    while (std::getline(worldLevelStream, currentSegment, '|'))
    {
        worldLevels.push_back((WorldHeight)std::stoi(currentSegment));
    }

    //For every world level present store the permissions
    for (int i = 0; i < worldLevels.size(); i++)
    {
        std::string prefix = "l" + std::to_string((int)worldLevels[i]);
        for (int y = height - 1; y >= 0; y--) {
            //Make member name string
            currentRow = prefix + PERMISSIONS_ROW + std::to_string(y);
            //Check member exists
            assert(doc.HasMember(currentRow.c_str()));
            //Reads data
            std::stringstream stream(doc[currentRow.c_str()].GetString());
            while (std::getline(stream, currentSegment, '|'))
            {
                planePermissions.push_back((MovementPermissions)std::stoi(currentSegment));
            }
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
    if ((unsigned int)id < Level::s_LevelOriginCache.size()) {
        Level::s_LevelOriginCache[(unsigned int)id] = { levelOX, -1* levelOZ };
    }

    //Store level size if not loaded before
    if ((unsigned int)id < Level::s_LevelDimensionCache.size()) {
        Level::s_LevelDimensionCache[(unsigned int)id] = { width, height };
    }

    LevelData data = { id, width, height, levelOX, levelOZ, planeHeights, worldLevels, planeDirections, planePermissions, planeTextures };
    return data;
}