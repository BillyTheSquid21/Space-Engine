#include "game/states/Overworld.h"

void Overworld::init(int width, int height, Level levelEntry) {
    //Width and height
    m_Width = width; m_Height = height;

    //Renderer setup
    m_Camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));
    m_Renderer.setLayout<float>(3, 4, 2);
    m_Renderer.setDrawingMode(GL_TRIANGLES);
    m_Renderer.generate((float)width, (float)height, &m_Camera);

    //Camera
    m_Camera.moveY(80.0f);
    m_Camera.panYDegrees(45.0f);

    //Splash
    m_Quad = CreateQuad(-0.55f, 0.6f, 1.1f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f);

    EngineLog("Overworld loaded:", (int)m_CurrentLevel);
}

void Overworld::loadRequiredData() {
    //Shader
    m_Shader.create("res/shaders/Default.glsl");

    //Set texture uniform
    m_Shader.setUniform("u_Texture", 1);

    //Texture
    m_Plane.setRenderer(&m_Renderer);
    m_Plane.generatePlaneXZ(-100.0f, -100.0f, 400.0f, 1200.0f, 40.0f);
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
        m_Camera.panX(1.0f * deltaTime);
    }
    if (HELD_D) {
        m_Camera.panX(-1.0f * deltaTime);
    }
    if (HELD_CTRL) {
        m_Camera.panY(-1.0f * deltaTime);
    }
    if (HELD_SHIFT) {
        m_Camera.panY(1.0f * deltaTime);
    }
    if (HELD_W) {
        m_Camera.moveZ(100.0f * deltaTime);
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
    if (key == GLFW_KEY_W) {
        if (action == GLFW_PRESS) {
            HELD_W = true;
        }
        else if (action == GLFW_RELEASE) {
            HELD_W = false;
        }
    }
}