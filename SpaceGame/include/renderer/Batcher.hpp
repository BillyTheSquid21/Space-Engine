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
		* System for commiting meshes - does not immediately batch
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
			m_Batch.emplace_back(src, vert, vertSize, ind, indSize);
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
			Geometry::BatchMeshes(m_MeshBuffer.getMesh(), m_Batch);
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

		Geometry::Mesh m_MeshBuffer; //Mesh of batched geometry
		std::vector<Geometry::MeshMeta> m_Batch; //Contains geometry that has been batched
		bool m_NeedsToBuffer = true;
	};
}

#endif
