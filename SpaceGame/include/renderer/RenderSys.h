#pragma once
#ifndef IND_RENDERER_H
#define IND_RENDERER_H

#include "string"
#include "cstring"
#include "unordered_map"
#include "algorithm"
#include "glm/glm.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Batcher.hpp"
#include "renderer/Instancer.hpp"
#include "renderer/GLClasses.h"
#include "renderer/Texture.h"
#include "renderer/TextureAtlas.h"
#include "renderer/Lighting.h"
#include "utility/SegArray.hpp"

#define MAX_NAME_LENGTH 15

//Keeps track of, stores and cycles various renderers for dynamic renderers
namespace SGRender
{

	//Each uniform just requires a name and a location of the uniform
	struct Uniform
	{
		std::string name;
		SGRender::UniformType type;
		void* uniform;
	};

	//Flags for draw behaviour - D prefix
	enum DFlag
	{
		D_DISABLE_DEPTH_TEST = 0b00000001
	};

	//Const char is used to easier convert to static string (char str[16])
	//std::string is used as key to unordered map
	class System
	{
	public:
		static bool init(int width, int height);
		static void set();
		static void setCamera(Camera& camera) { s_Camera = camera; }
		static Camera& camera() { return s_Camera; }

		static void clearScreen() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }
		static void setClearColor(float r, float g, float b) { glClearColor(r, g, b, 1.0f); }

		static void createRenderPass(const char* passName, std::vector<Uniform>& uniforms, const char* shaderName, const char* rendererName, int16_t priority, int flag);
		static void removeRenderPass(const char* name);

		static void loadTexture(std::string path, std::string name, int slot, int bpp, int flag);
		static void loadTexture(std::string atlasName, std::string path, std::string name);
		static void unloadTexture(std::string name);
		static void unloadTexture(std::string atlasName, std::string name);
		static void linkModelToAtlas(std::string atlas, std::string texture, std::string model);
		static void unlinkModelFromAtlas(std::string atlas, std::string texture, std::string model);
		static void generateAndMapAtlas(std::string atlas, int slot, int bpp, int flag);

		template<typename VertexType>
		static void addAtlas(std::string name)
		{
			//Check if exists
			if (s_TexAtlases->find(name) == s_TexAtlases->end())
			{
				EngineLog("Atlas already exists!");
				return;
			}

			s_TexAtlases->insert(name, Tex::TextureAtlas());
		}

		static void removeAtlas(std::string name)
		{
			if (s_TexAtlases->find(name) != s_TexAtlases->end())
			{
				s_TexAtlases->at(name).undoTransforms();
				s_TexAtlases->erase(name);
				return;
			}
			EngineLog("Atlas not found!");
		}

		//TODO - make threaded
		template<typename VertexType>
		static void loadModel(std::string path, std::string name)
		{
			if (!s_Set)
			{
				return;
			}

			if (s_Models->find(name) == s_Models->end())
			{
				(*s_Models)[name] = Geometry::Mesh();
				Model::LoadModel<VertexType>(path.c_str(), s_Models->at(name));
				EngineLog("Model loaded: ", name);
				return;
			}
			EngineLog("Model ", name, " was found");
		}

		static void unloadModel(std::string name);

		static int loadShader(const char* vertPath, const char* fragPath, const char* name);
		static int loadShader(const char* vertPath, const char* fragPath, const char* geoPath, const char* name);
		
		template<typename T, typename... Args>
		static int addBatcher(const char* name, GLenum drawMode, Args... args)
		{
			if (!s_Set || strlen(name) > MAX_NAME_LENGTH)
			{
				EngineLog("Invalid Batcher Name!");
				return -1;
			}

			char id[MAX_NAME_LENGTH + 1] = "\0";
			strcpy_s(id, MAX_NAME_LENGTH + 1, name);
			//Check if exists
			for (int i = 0; i < s_Batchers.size(); i++)
			{
				if (!strcmp(id, s_Batchers[i].id))
				{
					EngineLog("Batcher found!");
					return i;
				}
			}

			//Otherwise create and generate
			SGRender::Batcher batcher;

			//First check for any spaces
			for (int i = 0; i < s_Batchers.size(); i++)
			{
				if (!strcmp(s_Batchers[i].id, ""))
				{
					memset(s_Batchers[i].id, 0, MAX_NAME_LENGTH + 1);
					strcpy_s(s_Batchers[i].id, MAX_NAME_LENGTH + 1, id);
					s_Batchers[i].object = Batcher();
					s_Batchers[i].count = 0;
					s_Batchers[i].object.setLayout<T>(args...);
					s_Batchers[i].object.setDrawingMode(drawMode);
					s_Batchers[i].object.generate(s_Width, s_Height, 0);
					return i;
				}
			}

			//Otherwise put at back
			auto bt = s_Batchers.emplace_back();
			strcpy_s(bt->id, MAX_NAME_LENGTH + 1, id);
			bt->count = 0;
			bt->object.setLayout<T>(args...);
			bt->object.setDrawingMode(drawMode);
			bt->object.generate(s_Width, s_Height, 0);
			return s_Batchers.size() - 1;
		}
		
		static SGRender::Batcher& batcher(int index) { return s_Batchers[index].object; }
		static SGRender::Instancer& instancer(int index) { return s_Instancers[index].object; }

		template<typename T, typename... Args>
		static int addInstancer(const char* name, const char* modelName, GLenum drawMode, Args... args)
		{
			if (!s_Set || strlen(name) > MAX_NAME_LENGTH)
			{
				EngineLog("Invalid Instancer name!");
				return -1;
			}

			char id[MAX_NAME_LENGTH + 1] = "\0";
			strcpy_s(id, MAX_NAME_LENGTH + 1, name);
			//Check if exists
			for (int i = 0; i < s_Instancers.size(); i++)
			{
				if (!strcmp(id, s_Instancers[i].id))
				{
					EngineLog("Instancer found!");
					return i;
				}
			}

			//Otherwise create and generate
			//Check if model is loaded, if not, don't allow
			std::string stdname = std::string(modelName);
			for (auto& mod : *s_Models)
			{
				if (mod.first == stdname)
				{
					SGRender::Instancer instancer;
					//Check if any spare places
					for (int i = 0; i < s_Instancers.size(); i++)
					{
						if (!strcmp(s_Instancers[i].id, ""))
						{
							memset(s_Instancers[i].id, 0, MAX_NAME_LENGTH + 1);
							strcpy_s(s_Instancers[i].id, MAX_NAME_LENGTH + 1, id);
							s_Instancers[i].object = Instancer();
							s_Instancers[i].count = 0;
							s_Instancers[i].object.setLayout<T>(args...);
							s_Instancers[i].object.setDrawingMode(drawMode);
							s_Instancers[i].object.generate(s_Width, s_Height, &s_Models->at(stdname), 0);
							return i;
						}
					}

					auto in = s_Instancers.emplace_back();
					strcpy_s(in->id, MAX_NAME_LENGTH + 1, id);
					in->count = 0;
					in->object.setLayout<T>(args...);
					in->object.setDrawingMode(drawMode);
					in->object.generate(s_Width, s_Height, &s_Models->at(stdname), 0);

					return s_Instancers.size() - 1;
				}
			}
			EngineLog("Model not loaded!");
			return -1;
		}
		
		static void unloadShader(const char* name);
		static bool linkToBatcher(const char* name, uint32_t& index);
		static bool linkToInstancer(const char* name, uint32_t& index);
		static void unlinkFromBatcher(uint32_t index);
		static void unlinkFromInstancer(uint32_t index);
		
		static void commitToBatcher(int index, void* src, float* vert, unsigned int vertSize, const unsigned int* ind, unsigned int indSize);
		static void commitToInstancer(int index, void* data, unsigned int dataSize, unsigned int count);
		static void removeFromBatcher(int index, void* src, void* vert);
		static void batch(int index);

		static void render();
		static void bufferVideoData();

		static void setDim(int width, int height) { s_Width = width; s_Height = height; }
		static void clean();

		static Tex::Texture* findTexture(std::string name);
		static bool getShader(const char* shader, SGRender::Shader** shaderPtr);
		static const glm::mat4* identity() { return &s_Identity; }

		static bool accessModel(std::string name, Geometry::Mesh** model);
		static bool doesPassExist(const char* passName);

	private:
		static void removeRedundantPasses(void* addr); //Removes passes containing newly cleared renderers

		template<typename T>
		struct Identifier
		{
			char id[MAX_NAME_LENGTH + 1] = "\0";
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

		struct RenderPass
		{
			char id[MAX_NAME_LENGTH + 1] = "\0";
			int16_t priority = 0; //Gives relative priority over another pass (higher number is done first)
			int flag = 0;
			SGRender::Shader* shader;
			std::vector<InternalUniform> uniforms;
			SGRender::RendererBase* renderer;
		};

		static bool sortByPriority(SGRender::System::RenderPass& p1, SGRender::System::RenderPass& p2);

		//Render data
		static std::vector<RenderPass> s_RenderPasses;

		//Data storage - use segarray to keep pointers to consistent and allocations fast
		//TODO - Optimise for a specific size, using 16 for all right now
		static SegArray<Identifier<SGRender::Shader>, 16> s_Shaders;
		static SegArray<IdCount<SGRender::Batcher>, 16> s_Batchers;
		static SegArray<IdCount<SGRender::Instancer>, 16> s_Instancers;
		
		//Data storage for non frequently accessed components
		static std::unique_ptr<std::unordered_map<std::string, Geometry::Mesh>> s_Models;
		static std::unique_ptr<std::unordered_map<std::string, Tex::Texture>> s_Textures;
		static std::unique_ptr<std::unordered_map<std::string, Tex::TextureAtlas>> s_TexAtlases; //TODO - implement

		//Dimensions
		static int32_t s_Width, s_Height;

		static bool s_Set;

		//Debug Texture
		static void generateDebugTex();
		static const char* s_DebugName;

		//Flags
		static void flagStart(int flag);
		static void flagEnd(int flag);

		//Lighting
		static Lighting s_Lighting;

		//Camera
		static Camera s_Camera;
		static UniformBuffer s_CameraBuffer;

		static constexpr glm::mat4 s_Identity = glm::mat4(1.0f);
	};
}

#endif