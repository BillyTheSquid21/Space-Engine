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
        //Due to how quads are formed from tri's - TODO - rotate UV coords too
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