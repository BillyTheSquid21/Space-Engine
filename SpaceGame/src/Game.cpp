#include "Game.h"

//Main constructor
Game::Game(int width, int height) 
{
	m_Width = width;
	m_Height = height;
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

    //Hide cursor
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    if (glewInit() != GLEW_OK) {
        glfwTerminate();
        EngineLog("Failed to initialize openGL");
        success = false;
    }
    EngineLog("OpenGl ", glGetString(GL_VERSION));
    EngineLog(glGetString(GL_VENDOR), glGetString(GL_RENDERER));

    m_Renderer.init((float)m_Width, (float)m_Height);
    
    //shaders
    m_ShaderProgram.create("res/shaders/Default.glsl");
    m_ShaderProgram.bind();
    m_ShaderProgram.unbind();

    //depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    return success;
}

//Render
void Game::render() 
{
    //Clears
    Renderer::clearScreen();

    m_Renderer.commitPrimitive(&tri, GetElementCount(Shape::TRI), Renderer::s_Tri_I, Renderer::IND_TRI);

    //Bind shader program
    m_ShaderProgram.bind();

    //Renders the buffered primitives
    m_Renderer.drawPrimitives(m_ShaderProgram);

    /* Swap front and back buffers */
    glfwSwapBuffers(window);
}

void Game::handleEvents() 
{
    /* Poll for and process events */
    glfwPollEvents();
}

void Game::handleInput(int key, int scancode, int action, int mods) {

}

void Game::handleScrolling(double xOffset, double yOffset) {

}

void Game::handleMouse(int button, int action, int mods) {

}

//Update
void Game::update(double deltaTime) 
{

}

//Clean
void Game::clean() 
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Game::setTime(double time) {
    m_GlfwTime = time;
}