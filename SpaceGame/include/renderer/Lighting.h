#pragma once
#ifndef SG_LIGHTING_H
#define SG_LIGHTING_H

#include "GLClasses.h"
#include "Camera.h"
#include "glm/glm.hpp"
#include "thread"
#include "atomic"
#include "mtlib/ThreadPool.h"

#define MAX_CLUSTER_LIGHTS 64

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
		float pointLinear = 0.05f;
		float pointQuad = 0.008f;
		float pointCutoff = 0.08f; //Where the influence stops

		//Directional Light (i.e. the sun)
		DirectionalLight directionalLight;
	};

	//Cluster for clustered shading (just an AABB)
	struct Cluster
	{
		glm::vec4 minPoint;
		glm::vec4 maxPoint;
	};

	//Light Grid element
	struct LGridElement
	{
		uint32_t offset;
		uint32_t size;
	};

	class Lighting
	{
	public:
		Lighting() = default;

		void set(int width, int height, Camera* camera);
		void setAmbient(float amb) { m_BaseData.ambient = amb; updateLightBuffer(); }
		void setAmbientColor(float r, float g, float b) { m_BaseData.ambientColor = { r, g, b }; updateLightBuffer(); }

		int32_t addLight(const glm::vec3& pos, float brightness, const glm::vec3& color, float radius);
		bool removeLight(int32_t id);

		const std::vector<PointLight>& lights() { return m_LightList; }
		
		GLuint lightBindingPoint() const { return m_LightingSSBO.bindingPoint(); }
		
		//Culls the lights
		void cullLights();

		//Update the lighting buffer
		void updateLightBuffer();

		void linkShader(SGRender::Shader& shader);
		void clean();

	private:

		//Keep track of which light is where in the array
		struct LightID
		{
			int32_t lightID;
			int32_t lightIndex;
		};

		//Work out the radius of light plus attenuation
		float lightRadius(const PointLight& light);

		//Test light sphere against cluster AABB
		bool testSphereAABB(int light, int cluster, const glm::mat4& view);

		//Get sqr dist from point and aabb
		float sqDistPointAABB(glm::vec3 point, int cluster);

		//Clusters are in view space
		void buildClusters();

		//Check if light is in frustum
		void checkLightInFrustum(int light, LightID* idArray, std::atomic<int>& index);

		//Check all lights for a given cluster
		void checkClusterLights(int cluster, const glm::mat4& view);

		//Builds the frustum culled list
		void buildCulledList();

		//Builds the light grid and Tile light index list
		void buildSupportLists();

		//Copy data into tile buffer
		void copyTileLightData(void* lights, int offset, int size);

		//SSBO for GPU side lighting
		SSBO m_LightingSSBO;

		//SSBOs for cluster AABBs and Light Grid
		SSBO m_Clusters;
		SSBO m_LightGrid;
		SSBO m_TileLightIndex;

		//Camera pointer for frustum culling
		Camera* m_Camera = nullptr;

		//Unculled light list
		BaseLightData m_BaseData;
		std::vector<PointLight> m_LightList;
		std::vector<LightID> m_LightIDs;

		//Culled light list
		std::vector<PointLight> m_CulledLightList;
		std::vector<LightID> m_CulledLightIDs;

		//Raw cluster list and light grid list
		std::vector<Cluster> m_ClustersList;
		std::vector<LGridElement> m_LightGridList;
		std::vector<int32_t> m_TileLightIndexList;
		bool m_ModifiedLights = false; //Tracks if need to rebuild culling list

		struct GridBuffer
		{
			int32_t buffer[MAX_CLUSTER_LIGHTS];
		};

		std::vector<GridBuffer> m_LightGridBuffer;

		int32_t m_NextLightID = 0;
		int32_t m_MaxLights = 5000; //current hard limit - no reason
		size_t m_LightCount = 0;
		bool m_Set = false;

		int m_Width, m_Height;
	};
}

#endif
