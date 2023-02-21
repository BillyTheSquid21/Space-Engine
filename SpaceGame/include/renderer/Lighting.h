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
		float brightness = 0.0f;
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
		//Ambient
		glm::vec3 ambientColor = glm::vec3(1.0f, 1.0f, 1.0f);
		float ambient = 0.16f;

		//Point light base properties
		float pointConstant = 1.0f;
		float pointLinear = 0.025f;
		float pointQuad = 0.005f;
		float pointCutoff = 0.01f; //Where the influence stops

		//Directional Light (i.e. the sun)
		DirectionalLight directionalLight;
	};

	class Lighting
	{
	public:
		Lighting() = default;

		void set(Camera* camera);
		void setAmbient(float amb) { m_BaseData.ambient = amb; updateLightBuffer(); }
		void setAmbientColor(float r, float g, float b) { m_BaseData.ambientColor = { r, g, b }; updateLightBuffer(); }

		int32_t addLight(glm::vec3& pos, float brightness, glm::vec3& color, float radius);
		bool removeLight(int32_t id);
		
		GLuint lightBindingPoint() const { return m_LightingSSBO.bindingPoint(); }
		
		void cullLights();

		void linkShader(SGRender::Shader& shader);
		void clean();

	private:

		void updateLightBuffer();
		float lightRadius(PointLight& light);

		//SSBO for GPU side lighting
		SSBO m_LightingSSBO;

		//SSBO that keeps track of clusters

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

		//Culled light list
		std::vector<PointLight> m_CulledLightList;
		std::vector<LightID> m_CulledLightIDs;

		int32_t m_NextLightID = 0;
		int32_t m_MaxLights = 5000; //current hard limit - no reason
		size_t m_LightCount = 0;
		bool m_Set = false;
	};
}

#endif
