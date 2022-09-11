#include "game/states/MainMenu.h"

void MainMenu::init(int width, int height, GLFWwindow* window, std::shared_ptr<Overworld> overworldPtr, FontContainer* fonts, SGSound::System* system) {
    //Width and height
    m_Width = width; m_Height = height;

    //Sound
    m_System = system;

    m_OverworldPtr = overworldPtr;

    //Fonts loading
    m_Fonts = fonts;
    m_Fonts->loadFont("res\\fonts\\Newsgeek\\Newsgeek.ttf", "default", 50);
    m_Fonts->loadFont("res\\fonts\\Newsgeek\\Newsgeek.ttf", "default", 25);

    //Renderer setup
    m_Camera = SGRender::Camera::Camera(width, height, glm::vec3(0.0f, 0.0f, 2.0f));
    m_Renderer.setLayout<float>(3, 2, 4);
    m_Renderer.setDrawingMode(GL_TRIANGLES);
    m_Renderer.generate((float)width, (float)height, &m_Camera, sizeof(SGRender::TVertex));

    //Shader
    m_Shader.create(
        "res/shaders/frag/Default_CT_Shader.vert",
        "res/shaders/vert/Default_CT_Shader.frag"
    );

    //Set texture uniform - is blank
    m_Shader.setUniform("u_Texture", 0);

    //ImGui
    GameGUI::SetColors(16, 16, 16, ImGuiCol_WindowBg);
    GameGUI::SetColors(24, 24, 24, ImGuiCol_ChildBg);

    //GUI
    std::shared_ptr<GameGUI::HUD> gui(new GameGUI::HUD(width, height, 0, 0));
    m_GUI.setBase(gui);

    //Main menu
    std::shared_ptr<MainMenuGUI> menu(new MainMenuGUI());
    menu->m_FillX = false; menu->m_FillY = true;
    menu->m_Width = m_Width / 6.0f;
    menu->setNest(0);
    menu->setFontContainer(m_Fonts);
    menu->setOverworldPtr(m_OverworldPtr);
    menu->linkActiveFunc(std::bind(&Overworld::setActive, this, std::placeholders::_1));
    menu->linkShowOptions(&m_ShowOptions);
    m_GUI.addElement(menu);

    //Options
    std::shared_ptr<OptionsMenu> options(new OptionsMenu());
    options->m_Width = (4.8f * m_Width) / 6.0f;
    options->m_Height = m_Height - 20.0f;
    options->setNest(1);
    options->m_XPos = menu->m_Width + 15.0f;
    options->m_YPos = 10.0f;
    options->setFontContainer(m_Fonts);
    m_GUI.addElement(options);

    EngineLog("Main Menu loaded");
}

void MainMenu::update(double deltaTime, double time) 
{

}

void MainMenu::loadRequiredData()
{
    m_DataLoaded = true;
}

void MainMenu::purgeRequiredData()
{
    m_DataLoaded = false;
}

void MainMenu::render() 
{
    //ImGUI
    GameGUI::StartFrame();
    m_GUI.showNest(0, m_ShowMenu);
    m_GUI.showNest(1, m_ShowOptions);
    m_GUI.render();
    GameGUI::EndFrame();
}