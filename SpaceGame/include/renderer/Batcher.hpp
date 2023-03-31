#pragma once
#ifndef BATCHER_HPP
#define BATCHER_HPP

#include "renderer/Renderer.hpp"

namespace SGRender
{
	/**
	* Batcher is a type of renderer for collecting geometry and potentially instancing it
	*
	* First you generate the batcher, and add instances if you want instanced rendering
	* When you use the batcher, you submit:
	* 1. The vertices and indices you wish to batch together
	* 2. Then any additional instance variables for drawing the batched geometry multiple times
	*
	* You then buffer video data and draw primitives
	* This version is depreciated
	*/
	class Dep_Batcher : public RendererBase
	{
	public:

		/**
		* Generates the parameters for the renderer, setting up the camera, buffers and binding.
		*/
		void generate(float width, float height, size_t bufferWidth)
		{
			//Init camera
			m_VA.create();	m_VB.create(bufferWidth);	m_IB.create(1);
			m_VA.addBuffer(m_VB, m_VBL);
			//Bind
			m_VA.bind(); m_VB.unbind(); m_IB.unbind();
		}

		/**
		* System for commiting meshes and signalling active
		*
		* @param vert Pointer to vertices being drawn
		* @param vertSize Amount of floats contained within all the vertices to be drawn
		* @param ind Pointer to indices for drawing
		* @param indSize Amount of indices being submitted
		*/
		void commit(void* src, float* vert, unsigned int vertSize, const unsigned int* ind, unsigned int indSize)
		{
			//Avoid duplicate
			if (findBatch(src, vert) != -1)
			{
				return;
			}
			m_Batch.emplace_back(src, vert, vertSize, m_VertexType, ind, indSize);
		}

		/**
		* System for commiting meshes - batches after
		*
		* @param vert Pointer to vertices being drawn
		* @param vertSize Amount of floats contained within all the vertices to be drawn
		* @param ind Pointer to indices for drawing
		* @param indSize Amount of indices being submitted
		*/
		void commitAndBatch(void* src, float* vert, unsigned int vertSize, const unsigned int* ind, unsigned int indSize)
		{
			commit(src, vert, vertSize, ind, indSize);
			batch();
		}

		void remove(void* src, void* vert)
		{
			int index = findBatch(src, vert);
			if (index == -1)
			{
				EngineLog("Batch not found!");
				return;
			}
			m_Batch.erase(m_Batch.begin() + index);
		}

		void batch()
		{
			m_MeshBuffer.setLoaded(false);
			SGRender::BatchMeshes(m_MeshBuffer.getMesh(), m_Batch);
			m_MeshBuffer.setLoaded(true);
			m_NeedsToBuffer = true;
		}

		/**
		* Called when collected primitives are to be drawn
		*
		*/
		void drawPrimitives() {
			if (m_Batch.size() <= 0)
			{
				return;
			}

			//Bind all objects
			bindAll();
			drawBatched();
		}

		/**
		* Gathers vertice data into renderer buffers
		*/
		void bufferVideoData()
		{
			if (m_Batch.size() <= 0 || !m_NeedsToBuffer)
			{
				return;
			}

			m_VB.bufferData(m_MeshBuffer.getVertices(), m_MeshBuffer.getVertSize());
			m_IB.bufferData(m_MeshBuffer.getIndices(), m_MeshBuffer.getIndicesCount());
			
			//Clears out mesh buffer as is no longer needed
			m_MeshBuffer.clear();
			m_MeshBuffer.setLoaded(false);

			m_NeedsToBuffer = false;
		}

	protected:

		//Finds a batch instruction using the src object and the src verticies within
		int findBatch(void* src, void* vert)
		{
			for (int i = 0; i < m_Batch.size(); i++)
			{
				if (m_Batch[i].vertexMeta.verts == vert && m_Batch[i].source == src)
				{
					return i;
				}
			}
			return -1;
		}

		//Draw
		void drawBatched()
		{
			if (m_Batch.size() <= 0)
			{
				return;
			}

			glDrawElements(m_PrimitiveType, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		}

		Mesh m_MeshBuffer; //Mesh of batched geometry
		std::vector<MeshMeta> m_Batch; //Contains geometry that has been batched
		bool m_NeedsToBuffer = true;
	};

	/**
	* Batcher is a type of renderer for collecting geometry and potentially instancing it
	*
	* First you generate the batcher, and add instances if you want instanced rendering
	* When you use the batcher, you submit:
	* 1. The vertices and indices you wish to batch together
	* 2. Then any additional instance variables for drawing the batched geometry multiple times
	*
	* You then buffer video data and draw primitives
	*/
	class Batcher : public RendererBase
	{
	public:

		/**
		* Generates the parameters for the renderer, setting up the camera, buffers and binding.
		*/
		void generate(float width, float height, size_t bufferWidth)
		{
			//Init camera
			m_VA.create();	m_VB.create(bufferWidth);	m_IB.create(1);
			m_VA.addBuffer(m_VB, m_VBL);
			//Bind
			m_VA.bind(); m_VB.unbind(); m_IB.unbind();
		}

		/**
		* Links to batcher
		*/
		RenderLinkID linkToBatcher(SGRender::Mesh& model)
		{
			//Assign ID
			RenderLinkID id = m_NextRenderLinkID;
			m_NextRenderLinkID++;

			//Check if src mesh already exists
			MeshMeta meta = model.getMeta();
			for (size_t i = 0; i < m_Batch.size(); i++)
			{
				if (m_Batch[i].source == meta.source)
				{
					m_LinkTable[id] = { i, glm::identity<glm::mat4>() };
					return id;
				}
			}

			m_LinkTable[id] = { m_Batch.size(), glm::identity<glm::mat4>() };
			m_Batch.push_back(meta);

			return id;
		}

		/**
		* Unlink from batcher
		*/
		void unlinkFromBatcher(RenderLinkID& id)
		{
			m_LinkTable.erase(id);
			id = -1; //invalidate id
		}

		/**
		* System for commiting meshes and signalling active
		*
		* @param id ID given to object for rendering
		* @param matrix Matrix to apply to mesh
		*/
		void commit(RenderLinkID id, const glm::mat4& matrix)
		{
			if (id == -1)
			{
				EngineLog("Trying to commit to batcher after unlinking!");
				return;
			}

			Link link = m_LinkTable[id];
			bool isDirty = false;

			//Check conditions 1. and 3.
			if (!link.wasRendering)
			{
				isDirty = true;
			}
			else if (link.matrix == matrix)
			{
				isDirty = true;
			}

			if (isDirty)
			{
				link.matrix = matrix;
				link.isRendering = true;
			}

			link.isDirty = isDirty;
		}

		//This will be slow, hence the need to avoid frequent batching
		//TODO - make mesh mapped somehow to avoid rebatching whole thing
		void batch()
		{
			//Create temporary new meshes here for matrix transforms
			std::vector<Mesh> tmpMeshes;
			std::vector<MeshMeta> tmpMeta;
			
			//Count up how many total meshes will be needed
			size_t meshcount = 0;
			for (const auto& l : m_LinkTable)
			{
				if (l.second.isRendering)
				{
					meshcount++;
				}
			}
			tmpMeshes.resize(meshcount);
			tmpMeta.resize(meshcount);

			//Copy each mesh in from link table, transforming if matrix != identity
			size_t meshindex = 0;
			glm::mat4 identity = glm::identity<glm::mat4>();
			for (const auto& l : m_LinkTable)
			{
				if (l.second.isRendering)
				{
					//Get mesh data
					//Source will be a ptr to the mesh
					Mesh* mesh = (Mesh*)m_Batch[l.second.meshIndex].source;
					tmpMeshes[meshindex].copyInto(mesh->getMesh());
					MeshMeta meta = tmpMeshes[meshindex].getMeta();
					tmpMeta[meshindex] = meta;

					//Check matrix
					if (l.second.matrix != identity)
					{
						//Apply transform
						ApplyTransform(meta.vertexMeta, l.second.matrix, meta.vertexMeta.type);
					}
					meshindex++;
				}
			}

			//Batch the new meshes
			m_MeshBuffer.clear();
			m_MeshBuffer.setLoaded(false);
			BatchMeshes(m_MeshBuffer.getMesh(), tmpMeta);
			m_MeshBuffer.setLoaded(true);

			//Buffer data
			bufferVideoData();
		}

		/**
		* Called when collected primitives are to be drawn
		*
		*/
		void drawPrimitives() {
			if (m_Batch.size() <= 0)
			{
				return;
			}

			//Iterate elements to check for dirty and set render status
			bool tableDirty = false;
			for (auto& l : m_LinkTable)
			{
				if (l.second.isRendering && !l.second.wasRendering)
				{
					l.second.wasRendering = true;
				}
				else if (!l.second.isRendering && l.second.wasRendering)
				{
					l.second.wasRendering = false;
				}

				if (l.second.isDirty)
				{
					tableDirty = true;
				}
			}

			//If needs to rebatch
			if (tableDirty)
			{
				batch();
			}

			bindAll();
			drawBatched();

			//Mark all as not rendering anymore
			for (auto l : m_LinkTable)
			{
				l.second.isRendering = false;
			}
		}

	protected:

		//Buffers vertice data, internal so only called once mesh batched
		void bufferVideoData()
		{
			if (m_Batch.size() <= 0)
			{
				return;
			}

			m_VB.bufferData(m_MeshBuffer.getVertices(), m_MeshBuffer.getVertSize());
			m_IB.bufferData(m_MeshBuffer.getIndices(), m_MeshBuffer.getIndicesCount());

			//Clears out mesh buffer as is no longer needed
			m_MeshBuffer.clear();
			m_MeshBuffer.setLoaded(false);
		}

		//Draw
		void drawBatched()
		{
			glDrawElements(m_PrimitiveType, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		}

		struct Link
		{
			size_t meshIndex; //Index of mesh to be batched
			glm::mat4 matrix;
			bool isRendering = false;
			bool wasRendering = false;
			bool isDirty = false; 
			//Is dirty if: 1. Wasn't rendering and now needs to be
			//			   2. Was rendering and now doesn't need to be
			//			   3. The matrix describing has changed
		};

		SGRender::Mesh m_MeshBuffer; //Mesh of batched geometry
		std::map<RenderLinkID, Link> m_LinkTable; //Table for links - tells what mesh an object calling is linked to
		std::vector<SGRender::MeshMeta> m_Batch; //Contains geometry that has been batched
		int32_t m_NextRenderLinkID = 0;
	};
}

#endif
