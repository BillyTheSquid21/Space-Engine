#include "example/ExampleScene.h"

#include "cmath"

bool ExampleScene::init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback, MousePos_Callback mpCallback, bool windowed)
{
	bool succ = Game::init(name, kCallback, mCallback, sCallback, mpCallback, windowed);

	//1. Setup camera and renderer
	SGRender::Camera camtmp = SGRender::Camera(m_Width, m_Height, { 0.0f, 0.0f, 0.0f });
	camtmp.setProjection(glm::perspective(glm::radians(45.0f), (float)m_Width / (float)m_Height, 0.2f, 5000.0f));
	camtmp.setNearAndFarPlane(0.2f, 5000.0f);
	camtmp.setFOV(45.0f);
	camtmp.moveUp(50.0f);
	camtmp.panYDegrees(20.0f);
	SGRender::System::setCamera(camtmp);
	SGRender::System::set();

	//2.2 Setup all of the scene
	//2.1 Load sponza atrium model
	SGRender::System::loadMatModel("res/s/Sponza.gltf", "sponza", SGRender::V_UNTVertex, Model::M_FILL_MISSING);

	//2.2 Load the default shader
	SGRender::System::loadShader("res/default.vert", "res/default.frag", "default");

	//2.3 Setup a renderer and pass per material
	Model::MatModel* model;
	SGRender::System::accessMatModel("sponza", &model);
	for (int i = 0; i < model->meshes.size(); i++)
	{
		//2.4.a Load diffuse texture
		std::string diffPath = "res/s/" + model->meshes[i].mat.diffuseTexture;
		std::string diffName = "diff_" + std::to_string(i);
		SGRender::System::loadTexture(diffPath, diffName, 0, 3, Tex::T_FILTER_LINEAR);

		//2.4.b Load normal texture
		std::string normPath = "res/s/" + model->meshes[i].mat.normalTexture;
		std::string normName = "norm_" + std::to_string(i);
		SGRender::System::loadTexture(normPath, normName, 1, 3, Tex::T_FILTER_LINEAR);

		//2.5 Add batcher for material
		SGRender::System::addBatcher(std::to_string(i).c_str(), SGRender::V_UNTVertex, GL_TRIANGLES, 3, 2, 3, 3);

		//2.6 Set tex as uniform
		std::vector<SGRender::Uniform> u =
		{
			{
				"u_Texture",
				SGRender::UniformType::TEXTURE,
				&diffName
			},

			{
				"u_NormalMap",
				SGRender::UniformType::TEXTURE,
				&normName
			},

			{
				"u_Ambient",
				SGRender::UniformType::VEC3,
				&model->meshes[i].mat.ambient
			},

			{
				"u_Diffuse",
				SGRender::UniformType::VEC3,
				&model->meshes[i].mat.diffuse
			},

			{
				"u_Specular",
				SGRender::UniformType::VEC3,
				&model->meshes[i].mat.specular
			},

			{
				"u_Shininess",
				SGRender::UniformType::FLOAT,
				&model->meshes[i].mat.shininess
			},

			{
				"u_ShowLights",
				SGRender::UniformType::INT,
				&m_ShowLights
			},
		};
		SGRender::System::createRenderPass(std::to_string(i).c_str(), u, "default", std::to_string(i).c_str(), 0, 0);

		//2.7 Link to batcher
		uint32_t index; SGRender::Mesh& mesh = model->meshes[i].mesh;
		SGRender::System::linkToBatcher(std::to_string(i).c_str(), index);
		SGRender::System::commitToBatcher(index, &mesh, mesh.getVertices(), mesh.getVertSize(), mesh.getIndices(), mesh.getIndicesCount());
	}

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
		SGRender::System::lighting().addLight(pos1, 1.0f, col1, 10.0f);
	}

	EngineLogOk("Scene loaded");
	return succ;
}

void ExampleScene::update(double deltaTime)
{
	Game::update(deltaTime);

	m_ElapsedTime += deltaTime;
}

void ExampleScene::clean()
{
	Game::clean();
	Model::MatModel* model;
	SGRender::System::accessMatModel("sponza", &model);
	for (int i = 0; i < model->meshes.size(); i++)
	{
		SGRender::System::unlinkFromBatcher(i);
	}
}

void ExampleScene::handleInput(int key, int scancode, int action, int mods)
{
	Game::handleInput(key, scancode, action, mods);

	SGRender::Camera& cam = SGRender::System::camera();
	const float speed = 10.0f;
	if (key == GLFW_KEY_W)
	{
		cam.moveInCurrentDirection(speed);
	}
	if (key == GLFW_KEY_S)
	{
		cam.moveInCurrentDirection(-speed);
	}
	if (key == GLFW_KEY_UP)
	{
		cam.moveForwards(speed);
	}
	if (key == GLFW_KEY_DOWN)
	{
		cam.moveForwards(-speed);
	}
	if (key == GLFW_KEY_A)
	{
		cam.moveSideways(speed);
	}
	if (key == GLFW_KEY_D)
	{
		cam.moveSideways(-speed);
	}
	if (key == GLFW_KEY_SPACE)
	{
		cam.moveUp(speed);
	}
	if (key == GLFW_KEY_LEFT_CONTROL)
	{
		cam.moveUp(-speed);
	}
	if (key == GLFW_KEY_Q)
	{
		cam.panSideways(speed/20.0f);
	}
	if (key == GLFW_KEY_E)
	{
		cam.panSideways(-speed/20.0f);
	}
	if (key == GLFW_KEY_L)
	{
		if (action == GLFW_PRESS)
		{
			m_ShowLights = !m_ShowLights;
		}
	}
}