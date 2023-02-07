#include "core/Game.h"

bool Game::s_Close = false;

//Main constructor
Game::Game(int width, int height) 
{
	m_Width = width;
	m_Height = height;
    m_GlfwTime = 0.0;
}

//Init
bool Game::init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback, MousePos_Callback mpCallback, bool windowed)
{
    bool success = true;

    /* Initialize the library */
    if (!glfwInit()) {
        EngineLogFail("GLFW");
        success = false;
    }
    EngineLogOk("GLFW");

    //Set version of openGl
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //Set non resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);    

    //Set sampling for msaa
    glfwWindowHint(GLFW_SAMPLES, 8);

    //Sets up monitor
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!windowed)
    {
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        m_Width = mode->width;
        m_Height = mode->height;
    }

    //Create a windowed mode window and its OpenGL context
    if (!windowed)
    {
        window = glfwCreateWindow(m_Width, m_Height, name, monitor, nullptr);
    }
    else
    {
        window = glfwCreateWindow(m_Width, m_Height, name, nullptr, nullptr);
    }

    if (!window)
    {
        glfwTerminate();
        EngineLogFail("GLFW Window Create");
        success = false;
    }
    EngineLogOk("GLFW Window Create");

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    //enable vsync
    glfwSwapInterval(0);

    //Callbacks
    glfwSetKeyCallback(window, kCallback);
    glfwSetMouseButtonCallback(window, mCallback);
    glfwSetScrollCallback(window, sCallback);
    glfwSetCursorPosCallback(window, mpCallback);

    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        EngineLogFail("OpenGL");
        success = false;
    }
    EngineLogOk("OpenGL");
    EngineLog("OpenGl ", glGetString(GL_VERSION));
    EngineLog(glGetString(GL_VENDOR), glGetString(GL_RENDERER));

    //depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //gamma - TODO - add control
    //glEnable(GL_FRAMEBUFFER_SRGB);

    //Init static engines
    success = SGRender::System::init(m_Width, m_Height);
    success = SGObject::System::init();
    success = SGSound::System::init();

    //Init thread pool
    const auto process_count = std::thread::hardware_concurrency();
    MtLib::ThreadPool::Init(process_count);
    EngineLogOk("Thread Pool:", process_count);

    //Init imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    EngineLogOk("ImGui");

    //Init fonts
    GUI::FontContainer::init();
    GUI::GameGUI::LoadDefaultFonts();

    if (!success)
    {
        EngineLogFail("Engine Init");
        return false;
    }

    EngineLogOk("Engine Init");
    return success;
}

//Render
void Game::render() 
{
    //Call object render methods
    SGObject::System::render();

    glfwSwapBuffers(window);
    SGRender::System::clearScreen();

    //Buffer rendersys data and draw
    SGRender::System::bufferVideoData();
    SGRender::System::render();

    //Draw Gui
    GUI::GameGUI::PushDefault();
    GUI::GameGUI::GUIStart(m_Width, m_Height);
    GUI::GameGUI::GUIEnd();
    GUI::GameGUI::PopDefault();
}

void Game::handleEvents() 
{
    glfwPollEvents();
}

//Update
void Game::update(double deltaTime) {
    SGSound::System::update();
    SGObject::System::update(deltaTime);
}

//Clean
void Game::clean() 
{
    glfwDestroyWindow(window);
    glfwTerminate();

    //Clean static systems
    SGSound::System::clean();
    SGObject::System::clean();
    SGRender::System::clean();

    //Clear fonts
    GUI::FontContainer::clear();

    //Clean ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    EngineLogOk("System Cleaned!");
}

void Game::setTime(double time) {
    m_GlfwTime = time;
}