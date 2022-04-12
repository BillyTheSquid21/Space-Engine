#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include "ShapeFactory.h"
#include "GLClasses.h"
#include "RenderQueue.hpp"
#include "Camera.h"

namespace Render
{
	/**
	* Renderer for when batching is more efficient than instancing.
	* Useful for: primitives, sprites, small meshes, static geometry
	*/
	template<typename T>
	class Renderer
	{
	public:
		Renderer() :m_VA() {};

		/**
		* Generates the parameters for the renderer, setting up the camera, buffers and binding.
		*/
		void generate(float width, float height, Camera* cam)
		{
			//Init camera
			camera = cam;
			m_VA.create();	m_VB.create(1);	m_IB.create(1);
			m_VA.addBuffer(m_VB, m_VBL);
			//Bind
			m_VA.bind(); m_VB.unbind(); m_IB.unbind();
		}
		static void clearScreen() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

		/**
		* Sets the vertex layout, pushing to the buffer layout object. Floats are recommended.
		* 
		* @param stride Stride (width) of attribute (Eg for position - x, y, z is 3)
		*/
		template<typename Type>
		void setLayout(unsigned char stride) { m_VBL.push<Type>(stride); }
		template<typename Type>
		void setLayout(unsigned char stride1, unsigned char stride2) { m_VBL.push<Type>(stride1); m_VBL.push<Type>(stride2); }
		template<typename Type>
		void setLayout(unsigned char stride1, unsigned char stride2, unsigned char stride3) { m_VBL.push<Type>(stride1); m_VBL.push<Type>(stride2); m_VBL.push<Type>(stride3); }

		/**
		* Sets the drawing mode, defaulting to GLTriangles.
		*/
		void setDrawingMode(GLenum type) { m_PrimitiveType = type; }

		/**
		* System for commiting vertices. Uses Bulk Render Queue.
		* 
		* @param vert Pointer to vertices being drawn
		* @param vertSize Amount of floats contained within all the vertices to be drawn
		* @param ind Pointer to indices for drawing
		* @param indSize Amount of indices being submitted
		*/
		void commit(T* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize) { m_PrimitiveVertices.pushBack(vert, vertSize, ind, indSize); }							//only adds the vertices if no geometry exists
		
		/**
		* Called when collected primitives are to be drawn
		* 
		* @param shader Shader being bound for this draw call
		*/
		void drawPrimitives(Shader& shader) {
			drawCall(&m_RendererModelMatrix, shader, true);
		}

		//static indices - some indices are standard and will not change
		static const unsigned short int IND_TRI = 3;

		//Camera
		Camera* camera = nullptr;

		//Single model matrix for renderer
		glm::mat4 m_RendererModelMatrix = glm::mat4(1.0f);

	protected:
		//Helper functions - TODO - use thread pooling so separate renderers collect data in parallel
		void bindAll(Shader& shader) { shader.bind();	m_VA.bind();	m_IB.bind(); }
		void drawCall(glm::mat4* modelMatrix, Shader& shader, bool first) {
			//Buffer data
			bufferVideoData(m_PrimitiveVertices);
			//Use model matrix
			shader.setUniform("u_Model", modelMatrix);
			//Bind all objects
			bindAll(shader);
			//Draw Elements
			glDrawElements(m_PrimitiveType, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr);
		}

		//Pass collected primitives to buffer for draw
		void bufferVideoData(BulkRenderQueue<T*>& verticesArray)
		{
			//First get amount of data among all vertice floats
			unsigned int totalVertFloats = m_PrimitiveVertices.vertFloatCount();
			//Get amount of data among all indice ints
			unsigned int totalIndFloats = m_PrimitiveVertices.indIntCount();

			//Then create buffer with space for that many floats
			std::vector<float> vertices; std::vector<unsigned int> indices;		//Buffer declaration
			vertices.resize(totalVertFloats); indices.resize(totalIndFloats);	//Buffer resizing
			auto verticesIterator = vertices.begin(); auto indicesIterator = indices.begin();

			//Indexes
			unsigned int vertIndex = 0; unsigned int indIndex = 0;
			int vertexSizeIndex = 0; int indiceSizeIndex = 0;
			unsigned int largestInd = 0;

			while (verticesArray.itemsWaiting()) {
				//Get instructions from render queue
				BulkRenderContainer<T*> instructions = verticesArray.nextInQueue();

				//For each bulk chunk, render all
				for (int i = 0; i < instructions.elementsCount; i++)
				{
					//VERTICES
					const float* dataPointer = (const float*)instructions.verts[i];
					unsigned int dataSize = instructions.vertFloats[i];
					vertexSizeIndex++;

					//Copy vertices into vector		
					std::copy(&dataPointer[0], &dataPointer[dataSize], verticesIterator + vertIndex);
					vertIndex += dataSize;

					//INDICES
					const unsigned int* indDataPointer = instructions.inds[i];
					unsigned int indDataSize = instructions.indCount[i];
					//Add to vector one by one
					unsigned int currentLargest = 0;
					for (int j = 0; j < indDataSize; j++) {
						unsigned int newValue = indDataPointer[j] + largestInd;
						indices[indIndex] = newValue + indiceSizeIndex;
						indIndex++;
						if (newValue > currentLargest) {
							currentLargest = newValue;
						}
					}
					largestInd = currentLargest;

					indiceSizeIndex++;
				}

			}
			m_VB.bufferData(vertices.data(), vertices.size());
			m_IB.bufferData(indices.data(), indices.size());
		}

		//type of primitive being drawn
		GLenum m_PrimitiveType = GL_TRIANGLES;

		//GL Objects for rendering - used once per draw call
		VertexBuffer m_VB;
		IndexBuffer m_IB;
		VertexArray m_VA;
		VertexBufferLayout m_VBL;

		//Queue for rendering
		Render::BulkRenderQueue<T*> m_PrimitiveVertices;
	};

	/**
	 * Renderer type for meshes where instancing is more efficient than batching.
	 * Useful for: Large meshes, dynamic objects
	 */
	template <typename T>
	class InstanceRenderer : public Renderer<T>
	{
	public:
		using Renderer<T>::m_PrimitiveVertices; using Renderer<T>::m_IB; using Renderer<T>::m_VB; using Renderer<T>::m_PrimitiveType; using Renderer<T>::bindAll;
		
		/**
		* Sets the model to be drawn for this renderer
		* 
		* @param vert Pointer to model vertices
		* @param vertSize Amount of floats contained within all the vertices to be drawn
		* @param ind Pointer to indices for drawing
		* @param indSize Amount of indices being submitted
		*/
		void commit(T* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize) { m_Model = { vert, vertSize, ind, indSize }; }
		
		/**
		 * Commits a model matrix for the model, to be added to the list for drawing multiple copies
		 */
		void commitModelMat(glm::mat4 matrix) { m_ModelMatrixes.pushBack(matrix); }
		
		/**
		* Called when collected primitives are to be drawn
		*
		* @param shader Shader being bound for this draw call
		*/
		void drawPrimitives(Shader& shader) {
			drawCall(shader, true);
		}

	private:
		void drawCall(Shader& shader, bool first) {
			//Buffer data
			bufferVideoData();
			while (m_ModelMatrixes.itemsWaiting())
			{
				BulkContainer<glm::mat4> cont = m_ModelMatrixes.nextInQueue();
				for (int i = 0; i < cont.elementsCount; i++)
				{
					//Use model matrix
					shader.setUniform("u_Model", &cont.verts[i]);
					//Bind all objects
					bindAll(shader);
					//Draw Elements
					glDrawElements(m_PrimitiveType, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr);
				}
			}
		}

		//Pass collected primitives to buffer for draw
		void bufferVideoData()
		{
			//First get amount of data among all vertice floats
			unsigned int totalVertFloats = m_Model.vertFloats;
			//Get amount of data among all indice ints
			unsigned int totalIndFloats = m_Model.indCount;

			//Then create buffer with space for that many floats
			std::vector<float> vertices; std::vector<unsigned int> indices;		//Buffer declaration
			vertices.resize(totalVertFloats); indices.resize(totalIndFloats);	//Buffer resizing
			auto verticesIterator = vertices.begin(); auto indicesIterator = indices.begin();

			//Indexes
			unsigned int vertIndex = 0; unsigned int indIndex = 0;
			int vertexSizeIndex = 0; int indiceSizeIndex = 0;
			unsigned int largestInd = 0;

			//Get instructions from render queue
			RenderContainer<T*> instructions = m_Model;

			//For each bulk chunk, render all
			//VERTICES
			const float* dataPointer = (const float*)instructions.verts;
			unsigned int dataSize = instructions.vertFloats;
			vertexSizeIndex++;

			//Copy vertices into vector		
			std::copy(&dataPointer[0], &dataPointer[dataSize], verticesIterator);

			//INDICES
			const unsigned int* indDataPointer = instructions.inds;
			unsigned int indDataSize = instructions.indCount;
			//Add to vector one by one
			unsigned int currentLargest = 0;
			for (int j = 0; j < indDataSize; j++) {
				unsigned int newValue = indDataPointer[j] + largestInd;
				indices[indIndex] = newValue + indiceSizeIndex;
				indIndex++;
				if (newValue > currentLargest) {
					currentLargest = newValue;
				}
			}
			largestInd = currentLargest;

			indiceSizeIndex++;

			m_VB.bufferData(vertices.data(), vertices.size());
			m_IB.bufferData(indices.data(), indices.size());
		}

		using Renderer<T>::drawCall;
		Render::BulkQueue<glm::mat4> m_ModelMatrixes;
		Render::RenderContainer<T*> m_Model;
	};
}

#endif