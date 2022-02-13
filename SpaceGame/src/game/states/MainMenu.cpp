#include "game/states/MainMenu.h"

void MainMenu::init(int width, int height, GLFWwindow* window) {
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

    //ImGui
    GameGUI::SetColors(16, 16, 16, ImGuiCol_WindowBg);
    GameGUI::SetColors(24, 24, 24, ImGuiCol_ChildBg);

    EngineLog("Main Menu loaded");
}

void MainMenu::update(double deltaTime, double time) {

}

void MainMenu::render() {
    //ImGUI
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //Bind shader program
    m_Shader.bind();

    //Renders
    m_Shader.setUniform("u_Texture", 0);
    m_Camera.sendCameraUniforms(m_Shader);
    m_Renderer.drawPrimitives(m_Shader);

    //IMGui - All of this is shit and additionally testing right now
    GameGUI::SetNextWindowSize(300.0f, m_Height);
    GameGUI::SetNextWindowPos(0.0f, 0.0f);
    ImGui::Begin("Menu", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    //Side Menu
    ImGui::BeginChild("##LeftSide", ImVec2(300, ImGui::GetContentRegionAvail().y), false);
    
    //Top Text box
    ImFont* font = ImGui::GetFont();
    font->Scale = 0.5f;
    ImFont fontA = *font;
    fontA.Scale = 1.0f;
    
    ImGui::PushFont(&fontA);
    ImGui::SetCursorPosX(8.0f);
    ImGui::Text("Demo Game");
    ImGui::PopFont();
    

    //Buttons
    if (ImGui::Button("Game", ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 50))) {
        
    }
    if (ImGui::Button("Exit", ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, 50))) {
        Game::s_Close = true;
    }

    ImGui::EndChild();

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}