#pragma once
#ifndef SG_LIGHTING_H
#define SG_LIGHTING_H

#include "GLClasses.h"
#include "Camera.h"
#include "glm/glm.hpp"
#include "thread"
#include "atomic"
#include "mtlib/ThreadPool.h"
#include "mutex"

#define MAX_LIGHTS_PER_CLUSTER 64

namespace SGRender
{
	typedef int32_t LightID;

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

	//Buffer for lights
	struct GridBuffer
	{
		int32_t buffer[MAX_LIGHTS_PER_CLUSTER];
	};

	//Create cluster list of x*y*z
	void CreateClusterList(std::vector<Cluster>& clusters, const Camera* camera, const glm::vec2& screenDim, int dim, float tileSizeX, float tileSizeY);
	Cluster CreateCluster(const glm::vec2& screenDim, const glm::mat4& invproj, int x, int y, int z, float tileX, float tileY, float sszNear, float sszFar);

	//Get sqr dist from point and aabb
	static float SqDistPointAABB(glm::vec3 point, Cluster c);
	bool TestSphereAABB(glm::vec3 vs_pos, float radius, Cluster& cluster);

	class ClusterOctree
	{
	public:

		ClusterOctree(const Camera* camera, const glm::vec2& screenDim, int dim, float tileSizeX, float tileSizeY)
		{
			assert((dim > 0 && ((dim & (dim - 1)) == 0)), "Tree dim is not a power of two!");

			//Find how many layers will need
			m_MaxDepth = (log(dim) / log(2)) + 1;
			m_Dim = dim;
			EngineLog("Octree Depth: ", m_MaxDepth, " Octree Dim: ", m_Dim);

			//Reserve that many layers
			m_ClusterLayers.resize(m_MaxDepth);

			//Create underlying data for each depth
			for (int i = 0; i < m_ClusterLayers.size(); i++)
			{
				//Get depth
				int curr_depth = i + 1;

				//Get dim of layer
				int layer_dim = pow(2, i);

				//Get tilesize
				float tileX = screenDim.x / (float)layer_dim;
				float tileY = screenDim.y / (float)layer_dim;

				//Create data for that layer
				CreateClusterList(m_ClusterLayers[i], camera, screenDim, layer_dim, tileX, tileY);
			}

			m_Nodes.resize(1);

			//Generate tree recursively
			generateChildren(0, 1);
		}

		//Check light against light grid
		void markLightGrid(int32_t light, glm::vec3 ws_pos, float radius, const glm::mat4& view, std::vector<LGridElement>& lightGrid, std::vector<GridBuffer>& buffer, std::vector<std::shared_mutex>& mutex)
		{
			//Get centre in viewspace
			glm::vec3 position = glm::vec3(view * glm::vec4(ws_pos, 1.0));

			//Recursively check down tree to bottom to mark clusters
			//As frustum culled, assume all lights are in root node
			for (int i = 0; i < 8; i++)
			{
				int32_t node = m_Nodes[0].children[i];
				checkCluster(node, 2, light, position, radius, lightGrid, buffer, mutex);
			}
		}

		//Get the cluster the shader sees
		const std::vector<Cluster>& getBottomCluster() const { return m_ClusterLayers.back(); }

	private:

		struct Node
		{
			int32_t parent = -1;
			int32_t children[8] = { -1 };

			glm::ivec3 extent = glm::ivec3(1); //One more than indice

			Cluster* data;
		};

		void checkCluster(int32_t n, int32_t depth, int32_t light, glm::vec3 position, float radius, std::vector<LGridElement>& lightGrid, std::vector<GridBuffer>& buffer, std::vector<std::shared_mutex>& mutex)
		{
			//Get index
			if (depth == m_MaxDepth)
			{
				//Check final, then mark
				Cluster* cluster = m_Nodes[n].data;

				if (TestSphereAABB(position, radius, *cluster))
				{
					//Get index
					int cluster_index = (m_Nodes[n].extent.x - 1) +
						m_Dim * (m_Nodes[n].extent.y - 1) +
						(m_Dim * m_Dim) * (m_Nodes[n].extent.z - 1);

					//Lock grid buffer
					std::lock_guard<std::shared_mutex> lock(mutex[cluster_index]);

					if (lightGrid[cluster_index].size >= MAX_LIGHTS_PER_CLUSTER)
					{
						return;
					}

					//Add to buffer and update size
					buffer[cluster_index].buffer[lightGrid[cluster_index].size] = light;

					lightGrid[cluster_index].size++;
				}
				return;
			}

			//If in node, check each child, if not, go back
			Node& node = m_Nodes[n];
			Cluster* cluster = node.data;
			if (TestSphereAABB(position, radius, *cluster))
			{
				for (int i = 0; i < 8; i++)
				{
					checkCluster(m_Nodes[n].children[i], depth + 1, light, position, radius, lightGrid, buffer, mutex);
				}
			}
			else
			{
				return;
			}
		}

		void generateChildren(int n, int depth)
		{
			if (depth >= m_MaxDepth)
			{
				return;
			}

			//Get extent
			glm::ivec3 extent = m_Nodes[n].extent * 2;

			//Get dim of layer below this depth (hence no -1)
			int layer_dim = pow(2, depth);
			
			//Subdivide node
			Node children[8];

			for (int i = 0; i < 8; i++)
			{
				//Set parent and get extent
				children[i].parent = n;
				extentChild(children[i], i, extent);

				//Attach to underlying data
				int data_index = (children[i].extent.x-1) +
					layer_dim * (children[i].extent.y - 1) +
					(layer_dim * layer_dim) * (children[i].extent.z - 1);
				children[i].data = &(m_ClusterLayers[depth])[data_index];
				
				//Append node and generate children
				m_Nodes[n].children[i] = m_Nodes.size();
				m_Nodes.push_back(children[i]);
				generateChildren(m_Nodes.size() - 1, depth + 1);
			}
		}

		//Gives the extent of the child
		void extentChild(Node& child, int index, const glm::ivec3& ext_r)
		{
			switch (index)
			{
			case 0:
				child.extent = { ext_r.x - 1, ext_r.y - 1, ext_r.z - 1 };
				break;
			case 1:
				child.extent = { ext_r.x, ext_r.y - 1, ext_r.z - 1 };
				break;
			case 2:
				child.extent = { ext_r.x - 1, ext_r.y, ext_r.z - 1 };
				break;
			case 3:
				child.extent = { ext_r.x, ext_r.y, ext_r.z - 1 };
				break;
			case 4:
				child.extent = { ext_r.x - 1, ext_r.y - 1, ext_r.z };
				break;
			case 5:
				child.extent = { ext_r.x, ext_r.y - 1, ext_r.z };
				break;
			case 6:
				child.extent = { ext_r.x - 1, ext_r.y, ext_r.z };
				break;
			case 7:
				child.extent = ext_r;
				break;
			default:
				break;
			}
		}

		//Nodes within tree that index points to
		//Note that the root node has no data attached
		//this is due to the assumption all culled lights fit in the root of the camera frustum
		std::vector<Node> m_Nodes;

		//Cluster arrays ordered by depth
		std::vector<std::vector<Cluster>> m_ClusterLayers;

		int32_t m_Dim = 0;
		int32_t m_MaxDepth = 0;
	};

	class Lighting
	{
	public:
		Lighting() = default;

		void set(int width, int height, Camera* camera);
		void setAmbient(float amb) { m_BaseData.ambient = amb; updateLightBuffer(); }
		void setAmbientColor(float r, float g, float b) { m_BaseData.ambientColor = { r, g, b }; updateLightBuffer(); }

		LightID addLight(const glm::vec3& pos, float brightness, const glm::vec3& color, float radius);
		bool removeLight(LightID id);

		const std::vector<PointLight>& lights() { return m_LightList; }
		
		GLuint lightBindingPoint() const { return m_PointLightSSBO.bindingPoint(); }
		GLuint clusterBindingPoint() const { return m_ClusterInfo.bindingPoint(); }

		//Culls the lights
		void cullLights();

		//Update the lighting buffer
		void updateLightBuffer();

		size_t lightCount() const { return m_LightList.size(); }

		void linkShader(SGRender::Shader& shader);
		void clean();

	private:

		//Keep track of which light is where in the array
		struct LightTrackID
		{
			LightID id;
			int32_t index;
		};

		//Work out the radius of light plus attenuation
		float lightRadius(const PointLight& light);
		
		//Clusters are in view space
		void buildClusters();

		//Check if light is in frustum
		void checkLightInFrustum(int light, LightTrackID* idArray, std::atomic<int>& index);

		//Check all clusters for a given light (optimised)
		void checkClusterLightsOP(int light, const glm::mat4& view);

		//Builds the frustum culled list
		void buildCulledList();

		//Builds the light grid and Tile light index list
		void buildSupportLists();

		//Copy data into tile buffer
		void copyTileLightData(void* lights, int offset, int size);

		//SSBO for GPU side lighting
		SSBO m_PointLightSSBO;

		//SSBOs for cluster AABBs and Light Grid
		UniformBuffer m_ClusterInfo;
		SSBO m_LightGrid;
		SSBO m_TileLightIndex;

		//Camera pointer for frustum culling
		Camera* m_Camera = nullptr;

		//Unculled light list
		BaseLightData m_BaseData;
		std::vector<PointLight> m_LightList;
		std::vector<LightTrackID> m_LightIDs;

		//Culled light list
		std::vector<PointLight> m_CulledLightList;
		std::vector<LightTrackID> m_CulledLightIDs;

		//Cluster Octree
		std::shared_ptr<ClusterOctree> m_Octree;

		//Raw cluster list and light grid list
		std::vector<LGridElement> m_LightGridList;
		std::vector<int32_t> m_TileLightIndexList;
		bool m_ModifiedLights = false; //Tracks if need to rebuild culling list

		//Buffers lights during the cluster checking phase
		std::vector<GridBuffer> m_LightGridBuffer;
		std::vector<std::shared_mutex> m_LightGridMutex;

		int32_t m_NextLightID = 0;
		size_t m_LightCount = 0;
		bool m_Set = false;

		int m_Width, m_Height;
	};
}

#endif
