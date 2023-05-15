#include "example/ExampleScene.h"

#include "cmath"

bool ExampleScene::init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback, MousePos_Callback mpCallback, bool windowed)
{
	bool succ = Game::init(name, kCallback, mCallback, sCallback, mpCallback, windowed);
	initRenderer1();
	return succ;
}

void ExampleScene::initRenderer1()
{
	//1. Setup camera and renderer
	SGRender::Camera camtmp = SGRender::Camera(m_Width, m_Height, { 0.0f, 0.0f, 0.0f });
	camtmp.setProjection(glm::perspective(glm::radians(45.0f), (float)m_Width / (float)m_Height, 0.2f, 5000.0f));
	camtmp.setNearAndFarPlane(0.2f, 5000.0f);
	camtmp.setFOV(45.0f);
	camtmp.moveUp(50.0f);
	camtmp.panYDegrees(20.0f);
	renderer1.init(m_Width, m_Height, camtmp);

	//2.2 Setup all of the scene
	//2.1 Load sponza atrium model
	SGRender::ModelID modelID = renderer1.loadMatModel("res/s/Sponza.gltf", "sponza", SGRender::V_UNTVertex, Model::M_FILL_MISSING);

	//2.2 Load the default shader
	SGRender::ShaderID shader = renderer1.loadShader("res/default.vert", "res/default.frag", "default");

	//2.3 Generate Render passes
	renderer1.generateModelRenderPass(model, shader);

	//3. Add point lights
	constexpr int LIGHT_COUNT = 50;
	constexpr double RADIUS = 500.0f;
	constexpr double SEG_ANGLE = (double)LIGHT_COUNT / (2.0 * 3.14);
	for (int i = 0; i < LIGHT_COUNT; i++)
	{
		double sin_x = sin((double)i / SEG_ANGLE);
		double cos_z = cos((double)i / SEG_ANGLE);
		glm::vec3 pos1 = { RADIUS * sin_x, 20.0f, RADIUS * cos_z };
		glm::vec3 col1 = { sin_x, 0.2f, cos_z };
		SGRender::AddLightInstr lightAdd = { pos1, col1, 10.0f, 1.0f };
		SGRender::RenderInstruction instr = { SGRender::InstrType::ADD_LIGHT };
		instr.lightAdd = lightAdd;
		renderer1.queueInstruction(instr);
	}

	EngineLogOk("Scene loaded");
}

void ExampleScene::update(double deltaTime)
{
	Game::update(deltaTime);

	m_ElapsedTime += deltaTime;
}

void ExampleScene::render()
{
	renderRenderer1();
	Game::render();
}

void ExampleScene::renderRenderer1()
{
	glfwSwapBuffers(window);
	renderer1.clearScreen();

	//Buffer rendersys data and draw
	renderer1.processInstructions();
	renderer1.render();
}

void ExampleScene::clean()
{
	Game::clean();
	cleanRenderer1();
}

void ExampleScene::cleanRenderer1()
{
	renderer1.clean();
}

void ExampleScene::handleInput(int key, int scancode, int action, int mods)
{
	using namespace SGRender;
	Game::handleInput(key, scancode, action, mods);

	const float speed = 10.0f;
	if (key == GLFW_KEY_W)
	{
		MoveCameraInstr cammove = { CamMotion::Move_CurrentDir, speed };
		RenderInstruction instr = { InstrType::MOVE_CAMERA }; 
		instr.camMove = cammove;
		renderer1.queueInstruction(instr);
	}
	if (key == GLFW_KEY_S)
	{
		MoveCameraInstr cammove = { CamMotion::Move_CurrentDir, -speed };
		RenderInstruction instr = { InstrType::MOVE_CAMERA };
		instr.camMove = cammove;
		renderer1.queueInstruction(instr);
	}
	if (key == GLFW_KEY_UP)
	{
		MoveCameraInstr cammove = { CamMotion::Move_Forward, speed };
		RenderInstruction instr = { InstrType::MOVE_CAMERA };
		instr.camMove = cammove;
		renderer1.queueInstruction(instr);
	}
	if (key == GLFW_KEY_DOWN)
	{
		MoveCameraInstr cammove = { CamMotion::Move_Forward, -speed };
		RenderInstruction instr = { InstrType::MOVE_CAMERA };
		instr.camMove = cammove;
		renderer1.queueInstruction(instr);
	}
	if (key == GLFW_KEY_A)
	{
		MoveCameraInstr cammove = { CamMotion::Move_Sideways, speed };
		RenderInstruction instr = { InstrType::MOVE_CAMERA };
		instr.camMove = cammove;
		renderer1.queueInstruction(instr);
	}
	if (key == GLFW_KEY_D)
	{
		MoveCameraInstr cammove = { CamMotion::Move_Sideways, -speed };
		RenderInstruction instr = { InstrType::MOVE_CAMERA };
		instr.camMove = cammove;
		renderer1.queueInstruction(instr);
	}
	if (key == GLFW_KEY_SPACE)
	{
		MoveCameraInstr cammove = { CamMotion::Move_Up, speed };
		RenderInstruction instr = { InstrType::MOVE_CAMERA };
		instr.camMove = cammove;
		renderer1.queueInstruction(instr);
	}
	if (key == GLFW_KEY_LEFT_CONTROL)
	{
		MoveCameraInstr cammove = { CamMotion::Move_Up, -speed };
		RenderInstruction instr = { InstrType::MOVE_CAMERA };
		instr.camMove = cammove;
		renderer1.queueInstruction(instr);
	}
	if (key == GLFW_KEY_Q)
	{
		MoveCameraInstr cammove = { CamMotion::Pan_Sideways, speed/20.0f };
		RenderInstruction instr = { InstrType::MOVE_CAMERA };
		instr.camMove = cammove;
		renderer1.queueInstruction(instr);
	}
	if (key == GLFW_KEY_E)
	{
		MoveCameraInstr cammove = { CamMotion::Pan_Sideways, -speed/20.0f };
		RenderInstruction instr = { InstrType::MOVE_CAMERA };
		instr.camMove = cammove;
		renderer1.queueInstruction(instr);
	}
	if (key == GLFW_KEY_L)
	{
		if (action == GLFW_PRESS)
		{
			m_ShowLights = !m_ShowLights;
		}
	}
}