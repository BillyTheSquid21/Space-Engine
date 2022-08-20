#pragma once
#ifndef TRANSITION_H
#define TRANSITION_H

#include <string>

#include "renderer/GLClasses.h"
#include "renderer/ShapeFactory.h"

//All shaders used must have a "u_Time" float uniform
class Transition
{
public:
	void init(float width, float height)
	{
		m_Width = width; m_Height = height;

		m_Renderer.setLayout<float>(3);
		m_Renderer.setDrawingMode(GL_TRIANGLES);
		m_Renderer.generate(width, height, &m_Camera);

		m_Quad = CreateQuad(0, 0, width, height);
		m_Camera.setProjection(glm::ortho(0.0f, width, -height, 0.0f, -1.0f, 1.0f));
	}

	void update(double deltaTime, Shader& shader)
	{
		if (!m_Started)
		{
			return;
		}
		m_Timer += deltaTime;
	}

	void setCap(double cap)
	{
		m_Cap = cap;
	}

	void start()
	{
		m_Started = true;
	}

	bool isEnded()
	{
		if (m_Timer >= m_Cap)
		{
			m_Timer = 0.0;
			m_Started = false;
			return true;
		}
		return false;
	}

	bool isStarted() const { return m_Started; }

	//Uniform links
	void linkUniform(std::string name, float* value)
	{
		m_Uniforms.emplace_back(name, (void*)value, UniformType::TYPE_FLOAT);
	}
	void linkUniform(std::string name, glm::mat4* value)
	{
		m_Uniforms.emplace_back(name, (void*)value, UniformType::TYPE_MAT4);
	}

	void render(Shader& shader)
	{
		if (!m_Started)
		{
			return;
		}

		m_Renderer.commit(&m_Quad[0], GetFloatCount<Vertex>(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT);
		m_Renderer.bufferVideoData();

		shader.bind();

		//Set uniforms
		m_Camera.sendCameraUniforms(shader);
		shader.setUniform("u_Time", (float)m_Timer);
		//shader.setUniform("u_Height", 1080.0f);
		for (int i = 0; i < m_Uniforms.size(); i++)
		{
			Uniform uniform = m_Uniforms[i];
			switch (uniform.type)
			{
			case UniformType::TYPE_FLOAT:
			{
				float* floatPtr = (float*)uniform.value;
				shader.setUniform(uniform.name, *floatPtr);
				break;
			}
			case UniformType::TYPE_MAT4:
			{
				glm::mat4* mat4Ptr = (glm::mat4*)uniform.value;
				shader.setUniform(uniform.name, mat4Ptr);
				break;
			}
			}
		}

		m_Renderer.drawPrimitives();
	}

	float m_Width; float m_Height;

private:
	//Render
	Quad m_Quad;
	Camera m_Camera;
	Render::Renderer m_Renderer;

	//Update
	double m_Timer = 0.0;
	double m_Cap = 1.0;
	bool m_Started = false;

	//Uniforms
	enum class UniformType
	{
		TYPE_FLOAT, TYPE_MAT4
	};

	struct Uniform
	{
		std::string name;
		void* value;
		UniformType type;
	};

	std::vector<Uniform> m_Uniforms;
};

#endif