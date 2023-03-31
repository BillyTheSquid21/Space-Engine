#pragma once
#ifndef SG_SIB_RENDER_HPP
#define SG_SIB_RENDER_HPP

#include "services/RenderService.hpp"
#include "renderer/Camera.h"
#include "renderer/Renderer.hpp"
#include "renderer/Batcher.hpp"
#include "renderer/Instancer.hpp"
#include "renderer/GLClasses.h"
#include "renderer/MatModel.hpp"
#include "utility/SegArray.hpp"
#include "mutex"
#include "queue"
#include "map"

namespace SGRender
{
	//Stands for Simple Instance Batching Render
	//Renderers are either instancers or static batchers
	class SIBRender : public SGRender::RenderService
	{
	public:
		bool init(float width, float height);

		//Set camera params, also make able to give camera instructions
		void setCamera(const Camera& camera) { m_Camera = camera; };

		//Shader loading
		ShaderID loadShader(std::string vertPath, std::string fragPath);
		ShaderID loadShader(std::string vertPath, std::string geoPath, std::string fragPath);
		void unloadShader(ShaderID id);
		bool shaderExists(ShaderID id);

		//Model loading
		ModelID loadModel(std::string path);
		ModelID loadMatModel(std::string path);
		void unloadModel(ModelID id);
		bool modelExists(ModelID id);

		//Batcher/Instancer (refer as renderer)
		RendererID createBatchRenderer(VertexType vertexType, GLenum drawingmode);
		RendererID createInstancedRenderer(VertexType vertexType, GLenum drawingmode);
		void setInstancedRendererModel(RendererID renderer, ModelID model);
		void removeRenderer(RendererID id);
		bool batchedRendererExists(RendererID id);
		bool instancedRendererExists(RendererID id);

		//Render passes
		RenderPassID createPass(RendererID renderer, ShaderID shader);
		void removePass(RenderPassID id);
		bool passExists(RenderPassID id);

		//Uniforms
		void linkPassUniform(RenderPassID id, const Uniform& uniform); //Links so sets each frame
		void setPassUniform(RenderPassID id, const Uniform& uniform); //Sets as a once off
		void unlinkPassUniform(RenderPassID id, std::string name);

		//Main new change - instructions queued
		//Means game logic and renderer much less entwined
		void queueInstruction(RenderInstruction instr);

		void update(double deltaTime);
		void render();
		void clean();

	private:

		bool validatePass(RenderPassID id);

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

		std::vector<RenderPass> m_RenderPasses;

		//Store in map for now, optimise if slow as will be on very hot paths
		std::map<RendererID, std::shared_ptr<Batcher>> m_BatchedRenderers;
		std::map<RendererID, std::shared_ptr<Instancer>> m_InstancedRenderers;
		std::map<ShaderID, std::shared_ptr<Shader>> m_Shaders;
		std::map<ModelID, Mesh> m_Models;
		std::map<ModelID, Model::MatModel> m_MatModels;

		//ID's are just incremented assuming 32bits provides enough range for now
		int32_t m_NextRendererID = 0;
		int32_t m_NextShaderID = 0;
		int32_t m_NextRenderPassID = 0;
		int32_t m_NextModelID = 0;

		Camera m_Camera;
		UniformBuffer m_CameraBuffer;

		float m_Width = 640.0f;
		float m_Height = 640.0f;
	};
}

#endif