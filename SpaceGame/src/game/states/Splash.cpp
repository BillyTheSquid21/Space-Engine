#include "game/states/Splash.h"

void Splash::init(int width, int height) {
    //Width and height
    m_Width = width; m_Height = height;

    //Renderer setup
    m_Camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));
    m_Renderer.setLayout<float>(3, 4, 2);
    m_Renderer.setDrawingMode(GL_TRIANGLES);
    m_Renderer.generate((float)width, (float)height, &m_Camera);

    //Shader
    m_Shader.create("res/shaders/Default_CT_Shader.glsl");

    //Set texture uniform
    m_Shader.setUniform("u_Texture", 0);

    //Texture
    m_SplashTexture.loadTexture("res/splash.png");
    m_SplashTexture.generateTexture(0);
    m_SplashTexture.bind();
    m_SplashTexture.clearBuffer();

    //Splash
    m_Splash = CreateQuad(-0.55f, 0.6f, 1.1f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f);
    m_Splash2 = CreateQuad(-0.55f, 0.6f, 1.1f, 1.0f, 0.5f, 0.0f, 0.5f, 1.0f);
    AxialRotate<ColorTextureVertex>(&m_Splash2, { 0.0f, 0.0f, 0.0f }, 90, Shape::QUAD, Axis::Y);
    Translate<ColorTextureVertex>(&m_Splash2, 0.55f, 0.0f, -0.55f, Shape::QUAD);

    Translate<ColorTextureVertex>(&m_Splash, 0.0f, 0.0f, 0.55f, Shape::QUAD);
    Translate<ColorTextureVertex>(&m_Splash2, 0.0f, 0.0f, 0.55f, Shape::QUAD);

    EngineLog("Splash Screen loaded");
}

void Splash::render() {
    Render::Renderer<ColorTextureVertex>::clearScreen();

    //Bind shader program
    m_Shader.bind();

    //Renders
    m_Renderer.commit((ColorTextureVertex*)&m_Splash, GetFloatCount<ColorTextureVertex>(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT);
    m_Renderer.commit((ColorTextureVertex*)&m_Splash2, GetFloatCount<ColorTextureVertex>(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT);
    m_Shader.setUniform("u_Texture", 0);
    m_Camera.sendCameraUniforms(m_Shader);

    m_Renderer.drawPrimitives(m_Shader);
}

void Splash::update(double deltaTime, double time) {
    //Fade in
    m_CurrentTime += deltaTime;
    float alpha = 1.0f;
    if (m_CurrentTime < m_FadeIn) {
        alpha = (float)m_CurrentTime / (float)m_FadeIn;
    }

    //Rotate to second half of splash
    if (time > 5.0 && angle > -90.0f) {
        //Rotate and update angle
        m_Renderer.m_RendererModelMatrix = glm::rotate(m_Renderer.m_RendererModelMatrix, glm::radians(-40.0f * (float)deltaTime), glm::vec3(0.0f, 1.0f, 0.0f));
        angle += -40.0f * (float)deltaTime;
    }

}