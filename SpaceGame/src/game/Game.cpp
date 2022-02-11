#include "game/Game.h"

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
    camera = Camera::Camera(m_Width, m_Height, glm::vec3(0.0f, 0.0f, 2.0f));
    m_Renderer.setLayout<float>(3, 4, 2);
    m_Renderer.setDrawingMode(GL_TRIANGLES);
    m_Renderer.generate((float)m_Width, (float)m_Height, &camera);
    
    //shaders
    m_ShaderProgram.create("res/shaders/Default.glsl");

    //depth buffer
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    //alpha blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    //test 
    t1.loadTexture("res/default.png");
    t1.generateTexture(0);
    t1.clearBuffer();
    t1.bind();
    plane.generatePlaneXZ(-10.0f, -10.0f, 32.00f, 32.00f, 3.20f);
    plane.setRenderer(&m_Renderer);
    ColorShape(plane.accessQuad(0, 0), 1.0f, 0.0f, 0.0f, Shape::QUAD);
    ColorShape(plane.accessQuad(4, 7), 1.0f, 0.0f, 0.0f, Shape::QUAD);
    ColorShape(plane.accessQuad(6, 2), 1.0f, 0.0f, 0.0f, Shape::QUAD);
    m_ShaderProgram.setUniform("u_Texture", 0);

    return success;
}

//Render
void Game::render() 
{
    //Clears
    m_Renderer.clearScreen();

    m_Renderer.commit((Vertex*)(void*)&vert, 45, indices, 18);
    plane.render();

    //Bind shader program
    m_ShaderProgram.bind();

    //Renders the buffered primitives
    m_ShaderProgram.setUniform("u_Texture", 0);
    camera.sendCameraUniforms(m_ShaderProgram);
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
        camera.moveZ(camera.m_Speed);
    }
    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        camera.moveZ(-camera.m_Speed);
    }
    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        camera.moveX(camera.m_Speed);
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        camera.moveX(-camera.m_Speed);
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        camera.moveY(camera.m_Speed);
    }
    if (key == GLFW_KEY_LEFT_CONTROL && action == GLFW_PRESS) {
        camera.moveY(-camera.m_Speed);
    }
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
        camera.panX(camera.m_Speed);
    }
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
        camera.panX(-camera.m_Speed);
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