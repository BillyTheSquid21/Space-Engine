#pragma once
#ifndef SG_CLUSTER_OCTREE
#define SG_CLUSTER_OCTREE

#include "renderer/Lighting.h"

namespace SGRender
{
	class ClusterOctree
	{
	public:

		ClusterOctree(const Camera* camera, const glm::vec2& screenDim, int dim, float tileSizeX, float tileSizeY)
		{
			assert(dim % 2 == 0, "Dimensions must be even!");

			//Find how many layers will need
			m_MaxDepth = dim / 2;
			m_Dim = dim;

			//Reserve that many layers
			m_ClusterLayers.resize(m_MaxDepth);

			//Create root data - cluster that encompasses whole frustum
			m_RootCluster = CreateCluster(
				screenDim, 
				glm::inverse(camera->getProj()), 
				0, 0, 0, 
				camera->width(), 
				camera->height(), 
				camera->nearPlane(), 
				camera->farPlane()
			);

			//Create underlying data for each depth
			for (int i = 0; i < m_ClusterLayers.size(); i++)
			{
				//Get depth
				int curr_depth = i + 1;

				//Get dim of layer
				int layer_dim = curr_depth * 2;

				//Get tilesize
				float tileX = screenDim.x / (float)layer_dim;
				float tileY = screenDim.y / (float)layer_dim;

				//Create data for that layer
				CreateClusterList(m_ClusterLayers[i], camera, screenDim, layer_dim, tileX, tileY);
			}

			m_RootNode.data = &m_RootCluster;

			//Generate tree recursively
			generateTree(&m_RootNode, 1);
		}

		const std::vector<Cluster>& getBottomCluster() const { return m_ClusterLayers.back(); }

	private:

		struct Node
		{
			Node* parent;
			Node* children[8];

			glm::ivec3 extent = glm::ivec3(1); //One more than indice

			Cluster* data;
		};

		void generateTree(Node* node, int depth)
		{
			if (depth > m_MaxDepth)
			{
				return;
			}

			//Get new extent
			glm::ivec3 extent_range = node->extent * 2;

			//Subdivide into 8
			Node children[8];
			for (int i = 0; i < 8; i++)
			{
				children[i] = generateChild(node, extent_range, i, depth);
			}

			//Add to node vector
			m_Nodes.insert(m_Nodes.begin(), &children[0], &children[7]);

			//Link children to parent
			for (int i = 0; i < 8; i++)
			{
				node->children[i] = &m_Nodes[m_Nodes.size() - i - 1];
			}

			//Subdivide children
			for (int i = 0; i < 8; i++)
			{
				generateTree(node->children[i], depth + 1);
			}
		}

		Node generateChild(Node* parent, const glm::ivec3& extent, int index, int depth)
		{
			Node child;
			child.parent = parent;
			child.extent = extentChild(child, index, extent);

			//Attach data
			int vector_index = depth - 1;
			int dim = depth * 2;

			int cluster_index = (child.extent.x-1) + dim * (child.extent.y-1) +
				(dim*dim) * (child.extent.z-1);

			child.data = &m_ClusterLayers[vector_index].at(cluster_index);
		}

		//Gives the extent of the child
		glm::ivec3 extentChild(Node& child, int index, const glm::ivec3& ext_r)
		{
			switch (index)
			{
			case 0:
				child.extent = ext_r - glm::ivec3(1);
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

		Node m_RootNode = {};
		std::vector<Node> m_Nodes;

		Cluster m_RootCluster;
		std::vector<std::vector<Cluster>> m_ClusterLayers;

		int32_t m_Dim = 0;
		int32_t m_MaxDepth = 0;
	};
}

#endif