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

    //Renderer setup
    m_Renderer.setLayout<float>(3, 4, 2);
    m_Renderer.setDrawingMode(GL_TRIANGLES);
    m_Renderer.generate((float)m_Width, (float)m_Height);
    
    //shaders
    m_ShaderProgram.create("res/shaders/Default.glsl");

    //depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    //test
    m_RendererModelMatrix2 = glm::translate(m_RendererModelMatrix2, glm::vec3(-1.5f, 0.0f, 0.0f));
    m_RendererModelMatrix3 = glm::translate(m_RendererModelMatrix3, glm::vec3(1.5f, 0.0f, 0.0f));
    
    t1.loadTexture("res/default.png");
    t1.generateTexture(0);
    t1.clearBuffer();
    t1.bind();
    m_ShaderProgram.setUniform("u_Texture", 0);

    return success;
}

//Render
void Game::render() 
{
    //Clears
    m_RendererModelMatrix = glm::rotate(m_RendererModelMatrix, glm::radians(1.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_RendererModelMatrix2 = glm::rotate(m_RendererModelMatrix2, glm::radians(1.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    m_RendererModelMatrix3 = glm::rotate(m_RendererModelMatrix3, glm::radians(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_Renderer.clearScreen();
    m_Renderer.commit((Vertex*)(void*)vert, 45, indices, 18, &m_RendererModelMatrix);
    m_Renderer.commit((Vertex*)(void*)vert, 45, indices, 18, &m_RendererModelMatrix2);
    m_Renderer.commit((Vertex*)(void*)vert, 45, indices, 18, &m_RendererModelMatrix3);
    
    //Bind shader program
    m_ShaderProgram.bind();

    //Renders the buffered primitives
    m_ShaderProgram.setUniform("u_Texture", 0);
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
    if (key == GLFW_KEY_W && action == GLFW_PRESS) {
        m_Renderer.camera.moveZ(m_Renderer.camera.m_Speed);
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        m_Renderer.camera.moveZ(-m_Renderer.camera.m_Speed);
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        m_Renderer.camera.moveX(m_Renderer.camera.m_Speed);
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        m_Renderer.camera.moveX(-m_Renderer.camera.m_Speed);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        m_Renderer.camera.moveY(m_Renderer.camera.m_Speed);
    }
    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
        m_Renderer.camera.moveY(-m_Renderer.camera.m_Speed);
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        m_Renderer.camera.panX(m_Renderer.camera.m_Speed);
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        m_Renderer.camera.panX(-m_Renderer.camera.m_Speed);
    }
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