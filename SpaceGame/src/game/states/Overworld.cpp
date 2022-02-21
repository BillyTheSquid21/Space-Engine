#include "game/states/Overworld.h"

void Overworld::init(int width, int height, World::LevelID levelEntry) {
    //Width and height
    m_Width = width; m_Height = height;

    //Renderer setup
    m_Camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));

    //World Renderer
    m_WorldRenderer.setLayout<float>(3, 2);
    m_WorldRenderer.setDrawingMode(GL_TRIANGLES);
    m_WorldRenderer.generate((float)width, (float)height, &m_Camera);

    //Sprite Renderer
    m_SpriteRenderer.setLayout<float>(3, 2);
    m_SpriteRenderer.setDrawingMode(GL_TRIANGLES);
    m_SpriteRenderer.generate((float)width, (float)height, &m_Camera);

    //Camera
    m_Camera.moveUp(World::TILE_SIZE * 5);
    m_Camera.panYDegrees(45.0f);

    //test level
    level.buildLevel(10, 10, &m_WorldRenderer, &m_OverworldTileMap);

    //test object system

    EngineLog("Overworld loaded: ", (int)m_CurrentLevel);
}

void Overworld::loadRequiredData() {
    //Shader
    m_Shader.create("res/shaders/DefaultTexture.glsl");

    //Set texture uniform
    m_Shader.setUniform("u_Texture", 1);

    //Plane
    m_Plane.setRenderer(&m_WorldRenderer);
    m_Plane.generatePlaneXZ(0.0f, 0.0f, World::TILE_SIZE * 32, World::TILE_SIZE * 32, World::TILE_SIZE);
    UVData data = m_OverworldTileMap.uvTile(0, 0);
    m_Plane.texturePlane(data.uvX, data.uvY, data.uvWidth, data.uvHeight);
    
    //Load world texture
    m_PlaneTexture.loadTexture("res/textures/OW.png");
    m_PlaneTexture.generateTexture(0);
    m_PlaneTexture.bind();
    m_PlaneTexture.clearBuffer();

    //Load sprite textures
    m_OWSprites.loadTexture("res/textures/Sprite.png");
    m_OWSprites.generateTexture(1);
    m_OWSprites.bind();
    m_OWSprites.clearBuffer();

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

    //update objects
    m_ObjManager.update(deltaTime);
}

void Overworld::render() {
    Renderer<ColorTextureVertex>::clearScreen();

    //Bind shader program
    m_Shader.bind();

    //Renders
    level.render();
    m_ObjManager.render();
    
    m_Camera.sendCameraUniforms(m_Shader);

    m_Shader.setUniform("u_Texture", 0);
    m_WorldRenderer.drawPrimitives(m_Shader);
    m_Shader.setUniform("u_Texture", 1);
    m_SpriteRenderer.drawPrimitives(m_Shader);
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