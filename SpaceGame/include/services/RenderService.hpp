#pragma once
#ifndef SG_RENDER_SERVICE_HPP
#define SG_RENDER_SERVICE_HPP

#include "string"
#include "renderer/GLClasses.h"
#include "renderer/Camera.h"

namespace SGRender
{
	typedef int32_t RendererID;		//ID for a given renderer
	typedef int32_t RenderPassID;	//ID for a given render pass
	typedef int32_t ShaderID;		//ID for a given shader
	typedef int32_t TexID;			//ID for a given texture
	typedef int32_t ModelID;		//ID for a given model, made up of meshes
	typedef int32_t MeshID;			//ID for a given mesh inside a material of a model
	typedef int32_t MatID;			//ID for a given material

	enum class InstrType
	{
		//Rendering
		DRAW,

		//Camera
		MOVE_CAMERA,

		//Lighting
		ADD_LIGHT, REMOVE_LIGHT, SET_LIGHT_POS, MOVE_LIGHT
	};

	//Instruction structs

	struct RenderCallInstr
	{
		RendererID renderer;
		ModelID model;
		MatID material = -1;
		glm::mat4* transform = nullptr; //If nullptr, use identity
	};

	struct AddLightInstr
	{
		glm::vec3 pos;
		glm::vec3 col;
		float radius;
		float brightness;
	};

	struct MoveCameraInstr
	{
		CamMotion motionType;
		float speed;
	};

	//Union of all instructions
	struct RenderInstruction
	{
		InstrType instr;
		union
		{
			RenderCallInstr renderCall;
			AddLightInstr lightAdd;
			MoveCameraInstr camMove;
			bool clear;
		};
	};

	//Each uniform just requires a name and a location of the uniform
	struct Uni
	{
		std::string name;
		SGRender::UniformType type;
		void* uniform;
	};

	//Rough outline - not complete
	//Will make an actual implementation for now
	//Then will update this to reflect what I needed
	//Will replace current system as current system is very entangled
	//A lot of the implementation will be left up to the inheritor to make changes very convinient
	class RenderService
	{
	public:

	};
}

#endif
