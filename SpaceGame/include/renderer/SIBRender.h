#pragma once
#ifndef SG_SIB_RENDER_HPP
#define SG_SIB_RENDER_HPP

#include "services/RenderService.hpp"
#include "renderer/Camera.h"
#include "renderer/Renderer.hpp"
#include "renderer/Batcher.hpp"
#include "renderer/Instancer.hpp"
#include "renderer/GLClasses.h"
#include "renderer/Lighting.h"
#include "renderer/MatModel.h"
#include "utility/SegArray.hpp"
#include "mutex"
#include "queue"
#include "map"

namespace SGRender
{
	//Decoupled Mat model that doesn't store materials directly
	//Can later improve the implementation of the actual mat model
	//Store here for now, need to test system
	struct DecoMatModel
	{
		std::map<MatID, SGRender::Mesh> meshes;
		VertexType vertexType;
	};

	//Stands for Simple Instance Batching Render
	//Renderers are either instancers or static batchers
	//Will plop this in game state (very bad and wrong) to test before finalising render service methods
	class SIBRender : public SGRender::RenderService
	{
	public:
		bool init(float width, float height);

		//Set camera params, also make able to give camera instructions
		void setCamera(const Camera& camera) { m_Camera = camera; m_Lighting.set(m_Width, m_Height, &m_Camera); };

		//Shader loading
		ShaderID loadShader(std::string vertPath, std::string fragPath, std::string name);
		ShaderID loadShader(std::string vertPath, std::string geoPath, std::string fragPath, std::string name);
		void unloadShader(ShaderID id);
		bool shaderExists(ShaderID id);
		ShaderID locateShader(std::string name);

		//Model loading
		ModelID loadModel(std::string path, std::string name, VertexType vertexType);
		ModelID loadMatModel(std::string path, std::string name, VertexType vertexType);
		void unloadModel(ModelID id);
		bool modelExists(ModelID id);
		ModelID locateModel(std::string name);

		//TESTING WILL BE REMOVED
		//Will allow to do stuff as before, then can make good
		DecoMatModel& getMatModel_bad(ModelID id) { return *m_MatModels[id].model; }
		Material::Material getMat_bad(MatID id) { return m_Materials[id]; }

		//Manual Texture loading - into CPU memory
		TexID loadTexture(std::string path, std::string name);
		//Manual Texture generation - into GPU memory
		void generateTexture(TexID id, int slot, Tex::TFlag flags);
		void unloadTexture(TexID id);
		bool textureExists(TexID id);
		TexID locateTexture(std::string name);

		//Batcher/Instancer (refer as renderer)
		RendererID createBatchRenderer(std::string name, VertexType vertexType, GLenum drawingmode);
		RendererID createInstancedRenderer(std::string name, VertexType vertexType, GLenum drawingmode);
		void setInstancedRendererModel(RendererID renderer, ModelID model);
		void removeRenderer(RendererID id);
		bool rendererExists(RendererID id);
		bool batchedRendererExists(RendererID id);
		bool instancedRendererExists(RendererID id);
		RenderLinkID linkToBatcher(RendererID renderer, ModelID model);

		//Render passes
		RenderPassID createPass(std::string name, RendererID renderer, ShaderID shader);
		void removePass(RenderPassID id);
		bool passExists(RenderPassID id);

		//Uniforms
		void linkPassUniform(RenderPassID id, const Uniform& uniform); //Links so sets each frame
		void setPassUniform(RenderPassID id, const Uniform& uniform); //Sets as a once off
		void unlinkPassUniform(RenderPassID id, std::string name);

		//Main new change - instructions queued
		//Means game logic and renderer much less entwined
		void queueInstruction(RenderInstruction instr);
		void processInstructions();

		void update(double deltaTime);
		void render();
		void clean();

	private:

		bool validatePass(RenderPassID id);
		bool modelExistsInternal(ModelID id);
		bool matModelExistsInternal(ModelID id);

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

		struct RenderPass
		{
			RenderPassID passID;
			RendererID renderer;
			ShaderID shader;
			std::vector<InternalUniform> uniforms;
		};

		DecoMatModel processMatModel(const Model::MatModel& model);

		//Storage
		struct InternalRenderer
		{
			std::string name;
			int32_t type = 0; //0 is batcher, 1 is renderer
			std::shared_ptr<RendererBase> renderer;
		};

		struct InternalShader
		{
			std::string name;
			std::shared_ptr<Shader> shader;
		};

		struct InternalModel
		{
			std::string name;
			std::shared_ptr<Model::Model> model;
		};

		struct InternalMatModel
		{
			std::string name;
			std::shared_ptr<DecoMatModel> model;
		};

		struct InternalTexture
		{
			std::string name;
			std::shared_ptr<Tex::Texture> texture;
		};

		std::vector<RenderPass> m_RenderPasses;
		std::queue<RenderInstruction> m_Instructions;

		//Store in map for now, optimise if slow as will be on very hot paths
		std::map<RendererID, InternalRenderer> m_Renderers;
		std::map<ShaderID, InternalShader> m_Shaders;
		std::map<ModelID, InternalModel> m_Models;
		std::map<ModelID, InternalMatModel> m_MatModels;
		std::map<TexID, InternalTexture> m_Textures;

		//Store materials and their properties for easy lookup
		std::map<MatID, Material::Material> m_Materials;
		std::map<RendererID, MatID> m_RendererMaterialMap; //Maps which renderers correspond to which mat

		//ID's are just incremented assuming 32bits provides enough range for now
		int32_t m_NextRendererID = 0;
		int32_t m_NextShaderID = 0;
		int32_t m_NextRenderPassID = 0;
		int32_t m_NextModelID = 0;
		int32_t m_NextTexID = 0;
		int32_t m_NextMatID = 0;

		Camera m_Camera;
		UniformBuffer m_CameraBuffer;

		Lighting m_Lighting;

		float m_Width = 640.0f;
		float m_Height = 640.0f;
	};
}

#endif