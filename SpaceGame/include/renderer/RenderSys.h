#pragma once
#ifndef IND_RENDERER_H
#define IND_RENDERER_H

#include "string"
#include "cstring"
#include "unordered_map"
#include "map"
#include "algorithm"
#include "glm/glm.hpp"
#include "renderer/GLClasses.h"
#include "renderer/Texture.h"
#include "renderer/Lighting.h"
#include "renderer/MatModel.h"
#include "renderer/Renderer.h"
#include "services/RenderService.hpp"
#include "utility/SegArray.hpp"
#include "queue"

//Keeps track of, stores and cycles various renderers for dynamic renderers
namespace SGRender
{
	//LIST OF WHAT TO DO SO I DONT TEAR MY EYES OUT
	//Y means completed, N means otherwise
	//1. Remove the shitty string system - Y
	//2. Rework model system to not use templates - Y
	//3. Remove set parameter - Y
	//4. Rework maps to be ID based not unordered - Y
	//5. Remove referencing, decouple from user - N
	//TODO - list more

	//Each uniform just requires a name and a location of the uniform
	struct Uniform
	{
		std::string name;
		SGRender::UniformType type;
		void* uniform;
	};

	typedef std::map<MatID, std::vector<Mesh>> MaterialMeshMap; //Maps each mesh to a material

	class System
	{
	public:
		bool init(int width, int height, Camera& camera);
		void setCamera(Camera& camera) { s_Camera = camera; }

		void clearScreen() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
		void setClearColor(float r, float g, float b) { glClearColor(r, g, b, 1.0f); }

		void createRenderPass(std::string name, std::vector<Uniform>& uniforms, ShaderID shader, RendererID renderer, int16_t priority, int flag);
		void removeRenderPass(std::string name);
		void generateModelRenderPass(ModelID model, ShaderID shader);

		//Texture Loading
		TexID loadTexture(std::string path, std::string name, int slot, int bpp, int flag);
		void unloadTexture(TexID id);
		bool textureExists(TexID id);
		TexID locateTexture(std::string name);

		//Model Loading
		ModelID loadModel(std::string path, std::string name, VertexType vertexType, int modelFlags);
		ModelID loadMatModel(std::string path, std::string name, VertexType vertexType, int modelFlags);
		void unloadModel(ModelID id);
		bool modelExists(ModelID id);
		ModelID locateModel(std::string name);

		//Shader Loading
		ShaderID loadShader(std::string vertPath, std::string fragPath, std::string name);
		ShaderID loadShader(std::string vertPath, std::string fragPath, std::string geoPath, std::string name);
		void unloadShader(ShaderID id);
		bool shaderExists(ShaderID id);
		ShaderID locateShader(std::string name);

		//Material
		bool materialExists(MatID material);
		MatID locateMaterial(std::string name);

		//Renderer
		RendererID addRenderer(std::string name, VertexType vertexType, GLenum drawMode);
		void removeRenderer(RendererID id);
		bool rendererExists(RendererID id);
		RendererID locateRenderer(std::string name);
		

		void queueInstruction(RenderInstruction instr) { m_Instructions.push(instr); }
		void processInstructions();
		void render();

		void clean();

	private:
		//Internal Checks
		bool modelExistsInternal(ModelID id);
		bool matModelExistsInternal(ModelID id);

		//Instructions
		void drawInstr(RenderCallInstr instr);
		void addLightInstr(AddLightInstr instr);
		void moveCamInstr(MoveCameraInstr instr);

		//Debug Texture
		void generateDebugTex();

	private:

		template<typename T>
		struct Identifier
		{
			std::string id = "";
			T object;
		};

		template<typename T>
		struct IdCount : public Identifier<T>
		{
			int32_t count = 0; //tracks how many render components still point 
		};

		//keeps the uniform as an address not a string
		struct InternalUniform
		{
			InternalUniform(SGRender::Shader* sha, Uniform uni)
			{
				location = sha->getUniformLocation(uni.name);
				type = uni.type;
				uniform = uni.uniform;
			}

			GLint location;
			SGRender::UniformType type;
			void* uniform;
		};

		struct InternalTexture
		{
			std::string name;
			Tex::Texture texture;
		};

		struct InternalModel
		{
			std::string name;
			Model::Model model;
		};

		struct InternalMatModel
		{
			std::string name;
			MaterialMeshMap meshes;
		};

		struct InternalShader
		{
			std::string name;
			Shader shader;
		};

		struct InternalRenderer
		{
			std::string name;
			int32_t type = 0;
			Renderer renderer;
		};

		struct InternalMaterial
		{
			Material::Material material;
		};

		struct RenderPass
		{
			std::string id = "";
			int16_t priority = 0; //Gives relative priority over another pass (higher number is done first)
			int flag = 0;
			ShaderID shader;
			std::vector<InternalUniform> uniforms;
			RendererID renderer;
		};

	private:

		//Extract data from Mat Model and return a pointer
		std::shared_ptr<MaterialMeshMap> extractMatModel(Model::MatModel& model);

		//Render data
		std::vector<RenderPass> s_RenderPasses;

		//Data storage
		std::map<ShaderID, InternalShader> s_Shaders;
		std::map<RendererID, InternalRenderer> s_Renderers;
		std::map<ModelID, InternalModel> s_Models;
		std::map<ModelID, InternalMatModel> s_MatModels;
		std::map<TexID, InternalTexture> s_Textures;
		std::map<MatID, InternalMaterial> m_Materials;

		//Instruction queue
		std::queue<RenderInstruction> m_Instructions;

		//IDs
		TexID m_NextTexID = 0;
		ModelID m_NextModelID = 0;
		ShaderID m_NextShaderID = 0;
		RendererID m_NextRendererID = 0;
		MatID m_NextMaterialID = 0;

		//Dimensions
		int32_t s_Width = 640; int32_t s_Height = 320;

		const int m_DebugID = 0;

		//Lighting
		Lighting s_Lighting;

		//Camera
		Camera s_Camera;
		UniformBuffer s_CameraBuffer;

		const glm::mat4 s_Identity = glm::mat4(1.0f);
	};
}

#endif