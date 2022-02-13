#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include "ShapeFactory.h"
#include "GLClasses.h"
#include "RenderQueue.hpp"
#include "Camera.h"

//ACTUAL RENDERER CLASSES
template<typename T>
class Renderer
{
public:
	Renderer() :m_VA() {};

	void generate(float width, float height, Camera* cam) 
	{
		//Init camera
		camera = cam;
		m_VA.create();	m_VB.create(1);	m_IB.create(1);
		m_VA.addBuffer(m_VB, m_VBL);
		//Bind
		m_VA.bind(); m_VB.unbind(); m_IB.unbind();
		EngineLog("Renderer Initalized");
	}
	static void clearScreen() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

	//Creating layouts - with overloads - currently types should be 8 byte types otherwise padding will copy
	template<typename Type>
	void setLayout(unsigned char stride) { m_VBL.push<Type>(stride); }
	template<typename Type>
	void setLayout(unsigned char stride1, unsigned char stride2) { m_VBL.push<Type>(stride1); m_VBL.push<Type>(stride2);}
	template<typename Type>
	void setLayout(unsigned char stride1, unsigned char stride2, unsigned char stride3) {	m_VBL.push<Type>(stride1); m_VBL.push<Type>(stride2); m_VBL.push<Type>(stride3);	}

	//Set primitive draw type - defaults to triangle
	void setDrawingMode(GLenum type) { m_PrimitiveType = type; }
	
	//Drawing commit system - can only have either one model matrix, or one per object
	void commit(T* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize) { m_PrimitiveVertices.pushBack(vert, vertSize, ind, indSize); }							//only adds the vertices if no geometry exists
	void commit(T* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize, glm::mat4* modelMatrix) { if (m_PrimitiveVertices.size() < 1) { m_PrimitiveVertices.pushBack(vert, vertSize, ind, indSize); } m_ModelMatrices.pushBack(modelMatrix); }
	void drawPrimitives(Shader& shader) {
		//Check if only one model matrix, if not, applies different to each instance
		if (m_ModelMatrices.size() > 0) { bool first = true; while (m_ModelMatrices.itemsWaiting()) { drawCall(m_ModelMatrices.nextInQueue(), shader, first); first = false; } }
		else { drawCall(&m_RendererModelMatrix, shader, true); }
	}
	
	//static indices - some indices are standard and will not change
	static const unsigned short int IND_TRI = 3;

	//Camera
	Camera* camera = nullptr;
	
	//Single model matrix for renderer
	glm::mat4 m_RendererModelMatrix = glm::mat4(1.0f);
	
private:
	//Helper functions
	void bindAll(Shader& shader) {	shader.bind();	m_VA.bind();	m_IB.bind();	}
	void drawCall(glm::mat4* modelMatrix, Shader& shader, bool first) {
		//Buffer data
		if (first) {
			bufferVideoData(m_PrimitiveVertices);
		}
		//Use model matrix
		shader.setUniform("u_Model", modelMatrix);
		//Bind all objects
		bindAll(shader);
		//Draw Elements
		glDrawElements(m_PrimitiveType, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr);
	}
	
	//Pass collected primitives to buffer for draw
	void bufferVideoData(RenderQueue<T*>& verticesArray) 
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
			RenderContainer<Vertex*> instructions = verticesArray.nextInQueue();

			//VERTICES
			const float* dataPointer = (const float*)instructions.object;
			unsigned int dataSize = instructions.vertFloats;
			vertexSizeIndex++;

			//Copy vertices into vector
			std::copy(&dataPointer[0], &dataPointer[dataSize], verticesIterator + vertIndex);
			vertIndex += dataSize;

			//INDICES
			const unsigned int* indDataPointer = instructions.indices;
			unsigned int indDataSize = instructions.indicesCount;
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
	}

	//type of primitive being drawn
	GLenum m_PrimitiveType = GL_TRIANGLES;
	
	//GL Objects for rendering - used once per draw call
	VertexBuffer m_VB;
	IndexBuffer m_IB;
	VertexArray m_VA;
	VertexBufferLayout m_VBL;

	//Queue for rendering
	RenderQueue<T*> m_PrimitiveVertices;
	SimpleQueue<glm::mat4*> m_ModelMatrices;
};

#endif


