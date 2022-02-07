#pragma once

#include "ShapeFactory.h"
#include "GLClasses.h"
#include "RenderQueue.hpp"

//ACTUAL RENDERER CLASSES
class Camera
{
public:
	Camera() = default;
	Camera(float width, float height);
	~Camera() = default;
	void translateCamera(float deltaX, float deltaY);
	void positionCamera(float x, float y);
	void zoomCamera(float deltaZoom);
	void setZoomCamera(float zoom);

	//get data from camera
	void sendCameraUniforms(Shader& shader);

	//get bounds
	glm::mat4 bounds() const { return m_ResultantMatrix; }

private:
	float m_CameraWidth; float m_CameraHeight;
	glm::vec3 m_Scaling = { 1.0f , 1.0f, 1.0f }; //zoom
	glm::mat4 m_Translation = glm::mat4(1.0f);
	glm::mat4 m_Projection;
	Component4f m_CameraBounds = { -1.0f, 1.0f, -1.0f, 1.0f }; //xmin,xmax,ymin,ymax

	//Result to be sent to screen
	glm::mat4 m_ResultantMatrix = glm::mat4(1.0f);
};

class GUI
{
public:
	GUI() = default;
	GUI(float width, float height);
	~GUI() = default;

	//scale gui
	void setScale(float scale);

	//get data from camera
	void sendGUIUniforms(Shader& shader);

private:
	float m_GUIWidth; float m_GUIHeight;
	glm::vec3 m_Scaling = { 1.0f , 1.0f, 1.0f }; //scale
	glm::mat4 m_Projection; //projection for GUI has origin at middle of top of screen

	//Result to be sent to screen
	glm::mat4 m_ResultantMatrix = glm::mat4(1.0f);
};

const unsigned int s_Tri_I[]{
		0, 1, 2
};

template<typename T>
class Renderer
{
public:
	Renderer() :m_VA() {};

	void init(float width, float height) 
	{
		//Init camera
		camera = Camera::Camera(width, height);

		//Create vertex Array
		m_VA.create();

		//Create vertex buffer
		m_VB.create(1);

		//Create indice buffer
		m_IB.create(1);

		//push vertex position floats to layout
		m_VBL.push<float>(3);

		//push color floats to layout
		m_VBL.push<float>(4);

		//add buffer to array
		m_VA.addBuffer(m_VB, m_VBL);

		//Bind
		m_VA.bind();
		m_VB.unbind();
		m_IB.unbind();

		EngineLog("Renderer Initalized");
	}
	static void clearScreen() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

	//Drawing commit system - primitive is direct, shape can be used for primitive or derived
	void commitPrimitive(T* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize) { m_PrimitiveVertices.pushBack(vert, vertSize, ind, indSize); }
	void drawPrimitives(Shader& shader) 
	{
		//Buffer data
		bufferVideoData(m_PrimitiveVertices);
		//Use camera
		camera.sendCameraUniforms(shader);
		//Bind all objects
		bindAll(shader);
		//Draw Elements
		glDrawElements(GL_TRIANGLES, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr);

	}
	
	//static indices - some indices are standard and will not change
	static const unsigned short int IND_TRI = 3;

	//Camera
	Camera camera;

private:
	//Helper functions
	void bindAll(Shader& shader) 
	{
		shader.bind();
		m_VA.bind();
		m_IB.bind();
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
	
	//GL Objects for rendering - used once per draw call
	VertexBuffer m_VB;
	IndexBuffer m_IB;
	VertexArray m_VA;
	VertexBufferLayout m_VBL;

	//data pointers and size
	//general primitives
	RenderQueue<T*> m_PrimitiveVertices;
};


