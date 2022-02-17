#include "game/states/Overworld.h"

void Overworld::init(int width, int height, World::LevelID levelEntry) {
    //Width and height
    m_Width = width; m_Height = height;

    //Renderer setup
    m_Camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 0.0f));
    m_Renderer.setLayout<float>(3, 2);
    m_Renderer.setDrawingMode(GL_TRIANGLES);
    m_Renderer.generate((float)width, (float)height, &m_Camera);

    //Camera
    m_Camera.moveUp(World::TILE_SIZE * 5);
    m_Camera.panYDegrees(45.0f);

    //test object
    Player* sprite = new Player();
    sprite->generate(0.0f, 0.0f, HELD_W, HELD_S, HELD_A, HELD_D, levelEntry, 2);
    sprite->setRenderer(&m_Renderer);
    GameObject* spriteObject = (GameObject*)sprite;
    m_Manager.loadObject(spriteObject);

    EngineLog("Overworld loaded: ", (int)m_CurrentLevel);
}

void Overworld::loadRequiredData() {
    //Shader
    m_Shader.create("res/shaders/DefaultTexture.glsl");

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

    //update objects
    m_Manager.update(deltaTime, time);
}

void Overworld::render() {
    Renderer<ColorTextureVertex>::clearScreen();

    //Bind shader program
    m_Shader.bind();

    //Renders
    m_Plane.render();
    m_Manager.render();

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