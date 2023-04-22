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
	renderer1.loadMatModel("res/s/Sponza.gltf", "sponza", SGRender::V_UNTVertex, Model::M_FILL_MISSING);

	//2.2 Load the default shader
	SGRender::ShaderID shader = renderer1.loadShader("res/default.vert", "res/default.frag", "default");

	//2.3 Setup a renderer and pass per material
	SGRender::MaterialMeshMap* model;
	renderer1.accessMatModel("sponza", &model);
	for (auto& m : *model)
	{
		//2.4.a Load diffuse texture
		Material::Material& material = renderer1.accessMaterial(m.first);
		std::string diffPath = "res/s/" + material.diffuseTexture;
		std::string diffName = "diff_" + std::to_string(m.first);
		renderer1.loadTexture(diffPath, diffName, 0, 3, Tex::T_FILTER_LINEAR);

		//2.4.b Load normal texture
		std::string normPath = "res/s/" + material.normalTexture;
		std::string normName = "norm_" + std::to_string(m.first);
		renderer1.loadTexture(normPath, normName, 1, 3, Tex::T_FILTER_LINEAR);

		//2.5 Add batcher for material
		SGRender::RendererID renderer = renderer1.addBatcher(std::to_string(m.first).c_str(), SGRender::V_UNTVertex, GL_TRIANGLES);

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
				&material.ambient
			},

			{
				"u_Diffuse",
				SGRender::UniformType::VEC3,
				&material.diffuse
			},

			{
				"u_Specular",
				SGRender::UniformType::VEC3,
				&material.specular
			},

			{
				"u_Shininess",
				SGRender::UniformType::FLOAT,
				&material.shininess
			},

			{
				"u_ShowLights",
				SGRender::UniformType::INT,
				&m_ShowLights
			},
		};
		renderer1.createRenderPass(std::to_string(m.first).c_str(), u, shader, renderer, 0, 0);

		//2.7 Link to batcher
		for (auto& mesh : m.second)
		{
			renderer1.commitToBatcher(renderer, &mesh, mesh.getVertices(), mesh.getVertSize(), mesh.getIndices(), mesh.getIndicesCount());
		}
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
		renderer1.lighting().addLight(pos1, 1.0f, col1, 10.0f);
	}

	EngineLogOk("Scene loaded");
}

void ExampleScene::initRenderer2()
{
	//1. Setup camera and renderer
	SGRender::Camera camtmp = SGRender::Camera(m_Width, m_Height, { 0.0f, 0.0f, 0.0f });
	camtmp.setProjection(glm::perspective(glm::radians(45.0f), (float)m_Width / (float)m_Height, 0.2f, 5000.0f));
	camtmp.setNearAndFarPlane(0.2f, 5000.0f);
	camtmp.setFOV(45.0f);
	camtmp.moveUp(50.0f);
	camtmp.panYDegrees(20.0f);
	renderer2.init(m_Width, m_Height);
	renderer2.setCamera(camtmp);

	//2.2 Setup all of the scene
	//2.1 Load sponza atrium model
	model = renderer2.loadMatModel("res/s/Sponza.gltf", "sponza", SGRender::V_UNTVertex, Model::M_FILL_MISSING);

	//2.2 Load the default shader
	shader = renderer2.loadShader("res/default.vert", "res/default.frag", "default");

	//2.3 Setup a renderer and pass per material
	Model::MatModel& mm = renderer2.getMatModel_bad(model);
	int ind = 0;
	for (auto& m : mm.meshes)
	{
		//2.4.a Load diffuse texture
		std::string diffName = "diff_" + std::to_string(ind);
		if (m.mat.diffuseTexture != "")
		{
			std::string diffPath = "res/s/" + m.mat.diffuseTexture;
			SGRender::TexID diff = renderer2.loadTexture(diffPath, diffName);
			renderer2.generateTexture(diff, 0, Tex::T_FILTER_LINEAR);
		}

		//2.4.b Load normal texture
		std::string normName = "norm_" + std::to_string(ind);
		if (m.mat.normalTexture != "")
		{
			std::string normPath = "res/s/" + m.mat.normalTexture;
			SGRender::TexID norm = renderer2.loadTexture(normPath, normName);
			renderer2.generateTexture(norm, 1, Tex::T_FILTER_LINEAR);
		}

		//2.5 Add batcher for material
		SGRender::RendererID batcher = renderer2.createBatchRenderer(std::to_string(ind), SGRender::V_UNTVertex, GL_TRIANGLES);

		//2.6 Set tex as uniform
		std::vector<SGRender::Uni> u =
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
				&m.mat.ambient
			},

			{
				"u_Diffuse",
				SGRender::UniformType::VEC3,
				&m.mat.diffuse
			},

			{
				"u_Specular",
				SGRender::UniformType::VEC3,
				&m.mat.specular
			},

			{
				"u_Shininess",
				SGRender::UniformType::FLOAT,
				&m.mat.shininess
			},

			{
				"u_ShowLights",
				SGRender::UniformType::INT,
				&m_ShowLights
			},
		};
		SGRender::RenderPassID pass = renderer2.createPass(std::to_string(ind), batcher, shader);
		
		//2.7 Link to batcher
		renderer2.batcher(batcher)->commitAndBatch(&m.mesh, m.mesh.getVertices(), m.mesh.getVertSize(), m.mesh.getIndices(), m.mesh.getIndicesCount());
		renderer2.batcher(batcher)->bufferVideoData();
		ind++;
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
		renderer2.lighting().addLight(pos1, 1.0f, col1, 10.0f);
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
	renderer1.bufferVideoData();
	renderer1.render();
}

void ExampleScene::renderRenderer2()
{
	//for (auto& i : instr)
	//{
	//	renderer2.queueInstruction(i);
	//}
	//renderer2.processInstructions();

	glfwSwapBuffers(window);
	renderer1.clearScreen();
	renderer2.render();
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

void ExampleScene::cleanRenderer2()
{
	renderer2.clean();
}

void ExampleScene::handleInput(int key, int scancode, int action, int mods)
{
	Game::handleInput(key, scancode, action, mods);

	SGRender::Camera& cam = renderer1.camera();
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