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

class Renderer
{
public:
	Renderer();

	void init(float width, float height);
	static void clearScreen() { glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); }

	//Drawing commit system - primitive is direct, shape can be used for primitive or derived
	void commitPrimitive(void* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize);
	void drawPrimitives(Shader& shader);
	
	//static indices - some indices are standard and will not change
	static const unsigned int s_Tri_I[];
	static const unsigned short int IND_TRI = 3;

	//Camera
	Camera camera;

private:
	//Helper functions
	void bindAll(Shader& shader);

	//Pass collected primitives to buffer for draw
	void bufferVideoData(RenderQueue<void*>& verticesArray);
	
	//GL Objects for rendering - used once per draw call
	VertexBuffer m_VB;
	IndexBuffer m_IB;
	VertexArray m_VA;
	VertexBufferLayout m_VBL;

	//data pointers and size
	//general primitives
	RenderQueue<void*> m_PrimitiveVertices;
};


