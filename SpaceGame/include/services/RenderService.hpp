#pragma once
#ifndef SG_RENDER_SERVICE_HPP
#define SG_RENDER_SERVICE_HPP

#include "string"
#include "renderer/Renderer.hpp"
#include "renderer/GLClasses.h"

namespace SGRender
{
	typedef int32_t RendererID;
	typedef int32_t RenderPassID;
	typedef int32_t ShaderID;
	typedef int32_t TexID;
	typedef int32_t ModelID;
	typedef int32_t MatID;

	enum class InstrType
	{
		//Rendering
		DRAW,

		//Camera

		//Lighting

	};

	struct RenderCall
	{
		RendererID renderer;
		RenderLinkID link;
		glm::mat4* transform;
	};

	struct RenderInstruction
	{
		InstrType instr;
		union
		{
			RenderCall renderCall;
			bool clear;
		};
	};

	//Each uniform just requires a name and a location of the uniform
	struct Uniform
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
