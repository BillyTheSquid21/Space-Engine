#include "game/states/Overworld.h"

void Overworld::init(int width, int height, World::LevelID levelEntry) {
    //Width and height
    m_Width = width; m_Height = height;

    //Renderer setup
    m_Camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));
    m_Renderer.setLayout<float>(3, 4, 2);
    m_Renderer.setDrawingMode(GL_TRIANGLES);
    m_Renderer.generate((float)width, (float)height, &m_Camera);

    //Camera
    m_Camera.moveUp(World::TILE_SIZE * 5);
    m_Camera.panYDegrees(45.0f);

    EngineLog("Overworld loaded: ", (int)m_CurrentLevel);
}

void Overworld::loadRequiredData() {
    //Shader
    m_Shader.create("res/shaders/Default.glsl");

    //Set texture uniform
    m_Shader.setUniform("u_Texture", 1);

    //Texture
    m_Plane.setRenderer(&m_Renderer);
    m_Plane.generatePlaneXZ(0.0f, 0.0f, World::TILE_SIZE * 32, World::TILE_SIZE * 32, World::TILE_SIZE);
    
    m_PlaneTexture.loadTexture("res/textures/default.png");
    m_PlaneTexture.generateTexture(1);
    m_PlaneTexture.bind();
    m_PlaneTexture.clearBuffer();
    m_DataLoaded = true;
}

void Overworld::purgeRequiredData() {
    m_PlaneTexture.unbind();
    m_Plane.purgeData();
    m_DataLoaded = false;
}

void Overworld::update(double deltaTime, double time) {
    //Update Camera
    if (HELD_A) {
        m_Camera.moveX(100.0f * deltaTime);
    }
    if (HELD_D) {
        m_Camera.moveX(-100.0f * deltaTime);
    }
    if (HELD_CTRL) {
        m_Camera.panUp(-1.0f * deltaTime);
    }
    if (HELD_SHIFT) {
        m_Camera.panUp(1.0f * deltaTime);
    }
    if (HELD_Q) {
        m_Camera.moveForwards(100.0f * deltaTime);
    }
    if (HELD_E) {
        m_Camera.moveForwards(-100.0f * deltaTime);
    }
    if (HELD_W) {
        m_Camera.moveZ(100.0f * deltaTime);
    }
    if (HELD_S) {
        m_Camera.moveZ(-100.0f * deltaTime);
    }
}

void Overworld::render() {
    Renderer<Vertex>::clearScreen();

    //Bind shader program
    m_Shader.bind();

    //Renders
    m_Plane.render();
    //m_Renderer.commit((Vertex*)&m_Quad, GetFloatCount(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT);
    m_Shader.setUniform("u_Texture", 1);
    m_Camera.sendCameraUniforms(m_Shader);

    m_Renderer.drawPrimitives(m_Shader);
}

void Overworld::handleInput(int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_A) {
        if (action == GLFW_PRESS) {
            HELD_A = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_A = false;
        }
    }
    if (key == GLFW_KEY_D) {
        if (action == GLFW_PRESS) {
            HELD_D = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_D = false;
        }
    }
    if (key == GLFW_KEY_LEFT_CONTROL) {
        if (action == GLFW_PRESS) {
            HELD_CTRL = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_CTRL = false;
        }
    }
    if (key == GLFW_KEY_LEFT_SHIFT) {
        if (action == GLFW_PRESS) {
            HELD_SHIFT = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_SHIFT = false;
        }
    }
    if (key == GLFW_KEY_Q) {
        if (action == GLFW_PRESS) {
            HELD_Q = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_Q = false;
        }
    }
    if (key == GLFW_KEY_E) {
        if (action == GLFW_PRESS) {
            HELD_E = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_E = false;
        }
    }
    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS) {
            HELD_W = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_W = false;
        }
    }
    if (key == GLFW_KEY_S) {
        if (action == GLFW_PRESS) {
            HELD_S = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_S = false;
        }
    }
}

//World functions
void RotateTileCorner(Quad* quad, float angle) {
    Vertex v0 = quad->at(0);
    float x = v0.position.a + (World::TILE_SIZE / 2);
    float z = v0.position.c + (World::TILE_SIZE / 2);
    RotateShape(quad, { x, 0.0f, z }, angle, Shape::QUAD, Axis::Y);
}

void World::tileLevel(Quad* quad, WorldLevel level) {
    TranslateShape((void*)quad, 0.0f, ((float)level / sqrt(2)) * World::TILE_SIZE, 0.0f, Shape::QUAD);
}

void World::SlopeTile(Quad* quad, World::Direction direction) {
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
        TranslateShapeVertex((void*)quad, verticeIndex[i], 0.0f, TILE_SIZE / sqrt(2), 0.0f);
    }
}