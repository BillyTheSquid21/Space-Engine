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
bool Game::init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback)
{
    bool success = true;

    /* Initialize the library */
    if (!glfwInit()) {
        EngineLog("Failed to initialize GLFW");
        success = false;
    }

    //Sets up monitor
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    m_Width = mode->width;
    m_Height = mode->height;

    //Set version of openGl
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //set non resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);    

    //set sampling for msaa
    glfwWindowHint(GLFW_SAMPLES, 8);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(m_Width, m_Height, name, monitor, nullptr);
    if (!window)
    {
        glfwTerminate();
        EngineLog("Window failed to create");
        success = false;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    //enable vsync
    glfwSwapInterval(1);

    //Callbacks
    glfwSetKeyCallback(window, kCallback);
    glfwSetMouseButtonCallback(window, mCallback);
    glfwSetScrollCallback(window, sCallback);

    //Hide cursor - currently disabled
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        EngineLog("Failed to initialize openGL");
        success = false;
    }
    EngineLog("OpenGl ", glGetString(GL_VERSION));
    EngineLog(glGetString(GL_VENDOR), glGetString(GL_RENDERER));

    //depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Init sound engine
    success = sound.init();
    if (!success)
    {
        return false;
    }

    return success;
}

//Render
void Game::render() 
{
    /* Swap front and back buffers */
    glfwSwapBuffers(window);
}

void Game::handleEvents() 
{
    glfwPollEvents();
}

//Update
void Game::update(double deltaTime) {
    sound.update();
}

//Clean
void Game::clean() 
{
    glfwDestroyWindow(window);
    glfwTerminate();
    sound.clean();
}

void Game::setTime(double time) {
    m_GlfwTime = time;
}