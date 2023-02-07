#pragma once
#ifndef SG_LIGHTING_H
#define SG_LIGHTING_H

#include "GLClasses.h"
#include "Camera.h"
#include "glm/glm.hpp"

namespace SGRender
{
	//Directional lighting
	struct DirectionalLight
	{
		glm::vec3 direction = glm::vec3(0.0f, 0.5f, 0.5f);
		float brightness = 1.0f;
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
		float buffer;
	};

	//Point lighting
	struct PointLight
	{
		glm::vec3 position;
		float brightness = 1.0f;
		glm::vec3 color = glm::vec3(1.0f, 1.0f, 1.0f);
		float radius = 10.0f;
	};

	//Global lighting struct - should be aligned to GL Standard
	struct BaseLightData
	{
		glm::vec3 ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
		float ambient = 0.5f;

		//Directional Light (i.e. the sun)
		DirectionalLight directionalLight;
	};

	class Lighting
	{
	public:
		Lighting() = default;

		void set(Camera* camera);
		int32_t addLight(glm::vec3& pos, float brightness, glm::vec3& color, float radius);
		bool removeLight(int32_t id);
		GLuint lightBindingPoint() const { return m_LightingSSBO.bindingPoint(); }
		void linkShader(SGRender::Shader& shader);
		void clean();

	private:

		void updateLightBuffer();

		//SSBO for GPU side lighting
		SSBO m_LightingSSBO;

		//Keep track of which light is where in the array
		struct LightID
		{
			int32_t lightID;
			int32_t lightIndex;
		};

		//Camera pointer for frustum culling
		Camera* m_Camera = nullptr;

		//Unculled light list
		BaseLightData m_BaseData;
		std::vector<PointLight> m_LightList;
		std::vector<LightID> m_LightIDs;

		int32_t m_NextLightID = 0;
		int32_t m_MaxLights = 5000; //current hard limit - no reason
		size_t m_LightCount = 0;
		bool m_Set = false;
	};
}

#endif
