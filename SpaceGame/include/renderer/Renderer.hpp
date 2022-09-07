#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include "ShapeFactory.h"
#include "GLClasses.h"
#include "utility/SegArray.hpp"
#include "Camera.h"

namespace Render
{
	/**
	* Renderer for when batching is more efficient than instancing.
	* Useful for: primitives, sprites, small meshes, static geometry
	*/
	class Renderer
	{
	public:
		Renderer() : m_VA() {};

		/**
		* Generates the parameters for the renderer, setting up the camera, buffers and binding.
		*/
		void generate(float width, float height, Camera* cam, size_t bufferWidth)
		{
			//Init camera
			camera = cam;
			m_VA.create();	m_VB.create(bufferWidth);	m_IB.create(1);
			m_VA.addBuffer(m_VB, m_VBL);
			//Bind
			m_VA.bind(); m_VB.unbind(); m_IB.unbind();
			m_VertBuffWidth = bufferWidth;
		}
		template<typename T>
		void addInstances(unsigned int bufferWidth, unsigned int instanceDataStride, int startLocation)
		{
			m_VBInstances.create(bufferWidth);
			m_VBLInstances.push<T>(instanceDataStride, true);
			m_VA.addBuffer(m_VBInstances, m_VBLInstances, startLocation);
			m_VA.bind(); m_VBInstances.unbind();
			m_InstBuffWidth = bufferWidth;
		}
		static void clearScreen() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

		/**
		* Sets the vertex layout, pushing to the buffer layout object. Floats are recommended.
		* 
		* @param stride Stride (width) of attribute (Eg for position - x, y, z is 3)
		*/
		template<typename Type>
		void setLayout(unsigned char stride) { m_VBL.push<Type>(stride, false); }
		template<typename Type>
		void setLayout(unsigned char stride1, unsigned char stride2) { m_VBL.push<Type>(stride1, false); m_VBL.push<Type>(stride2, false); }
		template<typename Type>
		void setLayout(unsigned char stride1, unsigned char stride2, unsigned char stride3) { m_VBL.push<Type>(stride1, false); m_VBL.push<Type>(stride2, false); m_VBL.push<Type>(stride3, false); }

		/**
		* Sets the drawing mode, defaulting to GLTriangles.
		*/
		void setDrawingMode(GLenum type) { m_PrimitiveType = type; }

		/**
		* System for commiting vertices. Uses Segmented Array.
		* 
		* @param vert Pointer to vertices being drawn
		* @param vertSize Amount of floats contained within all the vertices to be drawn
		* @param ind Pointer to indices for drawing
		* @param indSize Amount of indices being submitted
		*/
		void commit(void* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize) 
		{ 
			if (m_VerticesIndex >= m_RenderInstructions.size())
			{
				m_RenderInstructions.emplaceBack(vert, vertSize, ind, indSize);
			}
			else
			{
				m_RenderInstructions[m_VerticesIndex] = RenderInfo::RenderInfo(vert, vertSize, ind, indSize);
			}
			m_VertSize += vertSize; m_IndSize += indSize;
			m_VerticesIndex++;
		}	

		void commitInstance(void* vert, unsigned int vertSize, unsigned int count)
		{
			if (m_InstanceIndex >= m_InstanceInstructions.size())
			{
				m_InstanceInstructions.emplaceBack(vert, vertSize);
			}
			else
			{
				m_InstanceInstructions[m_InstanceIndex] = InstanceInfo::InstanceInfo(vert, vertSize);
			}
			m_InstanceSize += vertSize;
			m_InstanceIndex++;
			if (m_FirstInstance)
			{
				m_InstanceCount = 0;
				m_FirstInstance = false;
			}
			m_InstanceCount += count;
		}
		
		/**
		* Called when collected primitives are to be drawn
		* 
		*/
		void drawPrimitives() {
			//Bind all objects
			bindAll();
			//Draw Elements
			if (m_InstanceCount == 0)
			{
				m_InstanceCount = 1;
			}
			glDrawElementsInstanced(m_PrimitiveType, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr, m_InstanceCount);
			glBindVertexArray(0);
			m_FirstInstance = true;
		}

		//Camera
		Camera* camera = nullptr;

		//Single model matrix for renderer
		glm::mat4 m_RendererModelMatrix = glm::mat4(1.0f);

		/**
		* Gathers vertice data into renderer buffers
		*/
		void bufferVideoData()
		{
			//Then create buffer with space for that many floats
			std::vector<float> vertices; std::vector<unsigned int> indices;		//Buffer declaration
			vertices.resize(m_VertSize); indices.resize(m_IndSize);				//Buffer resizing
			auto verticesIterator = vertices.begin(); auto indicesIterator = indices.begin();

			//Indexes
			unsigned int vertIndex = 0; unsigned int indIndex = 0;
			int indiceSizeIndex = 0;
			unsigned int largestInd = 0;

			for (int i = 0; i < m_VerticesIndex; i++) 
			{
				//Get instructions from render queue
				RenderInfo instructions = m_RenderInstructions[i];

				//VERTICES
				const float* dataPointer = (const float*)instructions.verts;
				unsigned int dataSize = instructions.vertFloats;

				//Copy vertices into vector		
				std::copy(&dataPointer[0], &dataPointer[dataSize], verticesIterator + vertIndex);
				vertIndex += dataSize;

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

			}
			m_VB.bufferData(vertices.data(), vertices.size());
			m_IB.bufferData(indices.data(), indices.size());

			//Keep array to correct size
			if (m_VerticesIndex < m_RenderInstructions.size())
			{
				m_RenderInstructions.shrinkTo(m_VerticesIndex);
			}
			m_VerticesIndex = 0;
			m_VertSize = 0; m_IndSize = 0;
			//Return if is non instanced
			if (m_InstanceIndex <= 0)
			{
				m_InstanceIndex = 0;
				m_InstanceSize = 0;
				return;
			}

			//Buffer any instance data
			std::vector<float> instances;
			instances.resize(m_InstanceSize);
			unsigned int instanceIndex = 0;
			auto instanceIterator = instances.begin();
			for (int i = 0; i < m_InstanceIndex; i++)
			{
				InstanceInfo instructions = m_InstanceInstructions[i];

				//INSTANCES
				const float* dataPointer = (const float*)instructions.verts;
				unsigned int dataSize = instructions.vertFloats;
				//Copy data into vector
				std::copy(&dataPointer[0], &dataPointer[dataSize], instanceIterator + instanceIndex);
				instanceIndex += dataSize;
			}
			m_VBInstances.bufferData(instances.data(), instances.size());
			//Keep array to correct size
			if (m_InstanceIndex < m_InstanceInstructions.size())
			{
				m_InstanceInstructions.shrinkTo(m_InstanceIndex);
			}
			m_InstanceIndex = 0;
			m_InstanceSize = 0;
		}

	protected:
		//Helper functions - TODO - use thread pooling so separate renderers collect data in parallel
		void bindAll() { m_VA.bind();	m_IB.bind(); }

		//type of primitive being drawn
		GLenum m_PrimitiveType = GL_TRIANGLES;

		//GL Objects for rendering - used once per draw call
		VertexBuffer m_VB;
		VertexBuffer m_VBInstances;
		IndexBuffer m_IB;
		VertexArray m_VA;
		VertexBufferLayout m_VBL;
		VertexBufferLayout m_VBLInstances;
		unsigned int m_VertBuffWidth = 0;
		unsigned int m_InstBuffWidth = 0;

		//Queue for rendering
		struct RenderInfo
		{
			RenderInfo() = default;
			RenderInfo(void* v, unsigned int vF, const unsigned int* i, unsigned short int iC) : verts(v), vertFloats(vF), inds(i), indCount(iC) {}
			void* verts = nullptr;
			unsigned int vertFloats = 0;
			const unsigned int* inds = nullptr;
			unsigned int indCount = 0;
		};

		struct InstanceInfo
		{
			InstanceInfo() = default;
			InstanceInfo(void* v, unsigned int vF) : verts(v), vertFloats(vF) {}
			void* verts = nullptr;
			unsigned int vertFloats = 0;
		};

		SegArray<RenderInfo, 96> m_RenderInstructions;
		SegArray<InstanceInfo, 96> m_InstanceInstructions;
		unsigned int m_VertSize = 0;
		unsigned int m_IndSize = 0;
		unsigned int m_InstanceSize = 0;
		unsigned int m_VerticesIndex = 0;
		unsigned int m_InstanceIndex = 0;
		unsigned int m_InstanceCount = 0;
		bool m_FirstInstance = true;
	};
}

#endif