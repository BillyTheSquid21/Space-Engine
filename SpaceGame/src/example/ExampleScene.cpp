#include "example/ExampleScene.h"

bool ExampleScene::init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback, MousePos_Callback mpCallback, bool windowed)
{
	bool succ = Game::init(name, kCallback, mCallback, sCallback, mpCallback, windowed);
	SGRender::System::set();
	//Setup all of the scene
	//1. Load sponza atrium model
	SGRender::System::loadMatModel<SGRender::UNVertex>("res/s/Sponza.gltf", "sponza", Model::M_FILL_MISSING);
	
	//2. Create a render pass for each material mesh
	//2.1 Load the default shader
	SGRender::System::loadShader("res/default.vert", "res/default.frag", "default");

	//2.2 Create and set the camera
	SGRender::Camera camtmp = SGRender::Camera(m_Width, m_Height, { 0.0f, 0.0f, 0.0f });
	camtmp.setProjection(glm::perspective(glm::radians(45.0f), (float)m_Width / (float)m_Height, 0.01f, 5000.0f));
	camtmp.setFOV(45.0f);
	camtmp.moveUp(50.0f);
	camtmp.panYDegrees(20.0f);
	SGRender::System::setCamera(camtmp);

	//2.3 Setup a renderer and pass per material
	Model::MatModel* model;
	SGRender::System::accessMatModel("sponza", &model);
	for (int i = 0; i < model->meshes.size(); i++)
	{
		//2.4 Load diffuse texture
		std::string diffPath = "res/s/" + model->diffuseTextures[model->meshes[i].matName];
		std::string diffName = "diff_" + std::to_string(i);
		SGRender::System::loadTexture(diffPath, diffName, 0, 3, Tex::T_FILTER_LINEAR);
		
		//2.5 Add batcher for material
		SGRender::System::addBatcher<float>(std::to_string(i).c_str(), GL_TRIANGLES, 3, 2, 3);
		
		//2.6 Set tex as uniform
		std::vector<SGRender::Uniform> u = 
		{
			{
				"u_Texture",
				SGRender::UniformType::TEXTURE,
				&diffName
			}
		};
		SGRender::System::createRenderPass(std::to_string(i).c_str(), u, "default", std::to_string(i).c_str(), 0, 0);
		
		//2.7 Link to batcher
		uint32_t index; Geometry::Mesh& mesh = model->meshes[i].mesh;
		SGRender::System::linkToBatcher(std::to_string(i).c_str(), index);
		SGRender::System::commitToBatcher(index, &mesh, mesh.getVertices(), mesh.getVertSize(), mesh.getIndices(), mesh.getIndicesCount());
	}

	EngineLogOk("Scene loaded");
	return succ;
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
		if (action == GLFW_PRESS)
		{
			cam.moveInCurrentDirection(speed);
		}
	}
	if (key == GLFW_KEY_S)
	{
		if (action == GLFW_PRESS)
		{
			cam.moveInCurrentDirection(-speed);
		}
	}
	if (key == GLFW_KEY_UP)
	{
		if (action == GLFW_PRESS)
		{
			cam.moveForwards(speed);
		}
	}
	if (key == GLFW_KEY_DOWN)
	{
		if (action == GLFW_PRESS)
		{
			cam.moveForwards(-speed);
		}
	}
	if (key == GLFW_KEY_A)
	{
		if (action == GLFW_PRESS)
		{
			cam.moveSideways(speed);
		}
	}
	if (key == GLFW_KEY_D)
	{
		if (action == GLFW_PRESS)
		{
			cam.moveSideways(-speed);
		}
	}
	if (key == GLFW_KEY_SPACE)
	{
		if (action == GLFW_PRESS)
		{
			cam.moveUp(speed);
		}
	}
	if (key == GLFW_KEY_LEFT_CONTROL)
	{
		if (action == GLFW_PRESS)
		{
			cam.moveUp(-speed);
		}
	}
	if (key == GLFW_KEY_Q)
	{
		if (action == GLFW_PRESS)
		{
			cam.panSideways(speed/20.0f);
		}
	}
	if (key == GLFW_KEY_E)
	{
		if (action == GLFW_PRESS)
		{
			cam.panSideways(-speed/20.0f);
		}
	}
}