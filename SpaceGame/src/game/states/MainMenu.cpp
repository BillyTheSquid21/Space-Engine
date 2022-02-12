#include "game/states/MainMenu.h"

void MainMenu::init(int width, int height) {
    //Width and height
    m_Width = width; m_Height = height;

    //Renderer setup
    m_Camera = Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));
    m_Renderer.setLayout<float>(3, 4, 2);
    m_Renderer.setDrawingMode(GL_TRIANGLES);
    m_Renderer.generate((float)width, (float)height, &m_Camera);

    //Shader
    m_Shader.create("res/shaders/Default.glsl");

    //Set texture uniform - is blank
    m_Shader.setUniform("u_Texture", 0);

    EngineLog("Main Menu loaded");
}

void MainMenu::update(double deltaTime, double time) {

}

void MainMenu::render() {
    //ImGUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //GUI HERE

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}