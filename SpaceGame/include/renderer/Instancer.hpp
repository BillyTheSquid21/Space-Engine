#pragma once
#ifndef INSTANCER_HPP
#define INSTANCER_HPP

#include "renderer/Renderer.hpp"
#include "renderer/Model.h"

namespace SGRender
{
	/**
	* Instancer is a type of renderer for instancing one single model
	* 
	* First you generate the instancer, and add instances if you want instanced rendering
	* When you use the instancer, you submit any additional instance variables for drawing the geometry multiple times
	*
	* You then buffer video data and draw primitives
	*/
	class Instancer : public RendererBase
	{
	public:
		/**
		* Generates the parameters for the instancer, setting up the camera, buffers and binding.
		*/
		void generate(float width, float height, Mesh* model, size_t bufferWidth)
		{
			//Init camera and model
			m_Model = model;
			m_VA.create();	m_VB.create(bufferWidth);	m_IB.create(1);
			m_VA.addBuffer(m_VB, m_VBL);
			//Bind
			m_VA.bind(); m_VB.unbind(); m_IB.unbind();
		}

		/**
		* Adds parameters for instancing, and uses the data stride and the start location (aka the layout number)
		* @param T underlying data type (float, double, int etc)
		* @param instanceDataStride the count of the underlying data type per instance (eg 3 for vec3)
		* @param startLocation the location in the shader layout
		*/
		template<typename T>
		void generateInstances(unsigned int instanceDataStride, int startLocation)
		{
			m_VBInstances.create(0);
			m_VBLInstances.push<T>(instanceDataStride, true);
			m_VA.addBuffer(m_VBInstances, m_VBLInstances, startLocation);
			m_VA.bind(); m_VBInstances.unbind();
		}

		/**
		* Adds matrix parameters for instancing, and uses the data stride and the start location (aka the layout number)
		* @param T underlying data type (float, double, int etc)
		* @param bufferWidth the size in bytes of the used matrix
		* @param startLocation the location in the shader layout
		*/
		template<typename T>
		void generateMatInstances(unsigned int bufferWidth, int startLocation)
		{
			m_VBInstances.create(0);

			//Check matrix size (if is square, casting to int shouldn't change)
			float matDimFloat = sqrt(bufferWidth / sizeof(T));
			int matDimInt = (int)matDimFloat;
			assert(matDimInt == matDimFloat && "Matrix is not square!");
			for (int i = 0; i < matDimInt; i++)
			{
				m_VBLInstances.push<T>(matDimInt, true);
			}

			m_VA.addBuffer(m_VBInstances, m_VBLInstances, startLocation);
			m_VA.bind(); m_VBInstances.unbind();
		}

		/**
		* System for commiting vertices. Uses Segmented Array.
		*
		* @param data Pointer to instance data (eg array of position offsets)
		* @param dataSize Amount of floats contained within the array (eg for array of vec3 - sizeof(vec3) * count)
		* @param count Amount of instances to be drawn
		*/
		void commitInstance(void* data, unsigned int dataSize, unsigned int count)
		{
			if (m_InstanceIndex >= m_InstanceInstructions.size())
			{
				m_InstanceInstructions.emplace_back(data, dataSize);
			}
			else
			{
				m_InstanceInstructions[m_InstanceIndex] = InstanceInfo::InstanceInfo(data, dataSize);
			}
			m_InstanceSize += dataSize;
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
			//Draw Elements depending on draw type
			drawInstanced();
		}

		/**
		* Gathers vertice data into renderer buffers
		*/
		void bufferVideoData()
		{
			if (!m_Model)
			{
				return;
			}

			if (m_InstanceCount <= 0 || !m_Model->isLoaded())
			{
				return;
			}
			m_VB.bufferData(m_Model->getVertices(), m_Model->getVertSize());
			m_IB.bufferData(m_Model->getIndices(), m_Model->getIndicesCount());

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

		Mesh* model() { return m_Model; }

		void reset()
		{
			m_Model = nullptr;
			m_InstanceInstructions.clear();
			m_InstanceSize = 0;
			m_InstanceIndex = 0;
			m_InstanceCount = 0;
			m_FirstInstance = true;
		}

	protected:

		void drawInstanced()
		{
			glDrawElementsInstanced(m_PrimitiveType, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr, m_InstanceCount);
			glBindVertexArray(0);
			m_FirstInstance = true;
			m_InstanceCount = 0;
		}

		VertexBuffer m_VBInstances;
		VertexBufferLayout m_VBLInstances;
		int32_t m_InstanceSize = 0;
		int32_t m_InstanceIndex = 0;
		int32_t m_InstanceCount = 0;
		bool m_FirstInstance = true;

		struct InstanceInfo
		{
			InstanceInfo() = default;
			InstanceInfo(void* v, unsigned int vF) : verts(v), vertFloats(vF) {}
			void* verts = nullptr;
			int32_t vertFloats = 0;
		};

		SegArray<InstanceInfo, 96> m_InstanceInstructions;

		//Model to be instanced
		Mesh* m_Model = nullptr;
	};
}

#endif