#pragma once

#include "ShapeFactory.h"
#include "GLClasses.h"

//ACTUAL RENDERER CLASSES
struct primitiveLocation
{	
	//used to return where a particular primitive is stored - needs to provide tri count
	unsigned int indexVert;
	unsigned int indexSizeVert;
	unsigned int indexInd;
	unsigned int indexSizeInd;
};

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
	void clearScreen() const;

	//Drawing commit system - primitive is direct, shape can be used for primitive or derived
	static void commitShape(const void* vert, Shape derivedType, Shape baseType, bool isGUI);
	static void commitPrimitive(void* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize);
	static void commitGUIPrimitive(void* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize);
	void drawPrimitives(Shader& shader);
	void draw(const VertexArray &va, const IndexBuffer &ib, const Shader &shader) const;
	
	//static indices - some indices are standard and will not change
	static const unsigned int s_Tri_I[];
	static const unsigned int s_Quad_I[];
	static const unsigned int s_Line_I[];
	static const unsigned int s_Circle_I[];

	//static indices sizes
	static const unsigned short int IND_TRI = 3;
	static const unsigned short int IND_QUAD = 6;
	static const unsigned short int IND_LINE = 6;
	static const unsigned short int IND_CIRCLE = 189;	

	//Camera
	Camera camera;

	//GUI
	GUI gui;

private:
	//Helper functions
	void bindAll(Shader& shader);

	//Commit internal
	template<typename T>
	static void commitInternal(bool isGUI, unsigned short int baseTypeCount, Shape baseType, T verticeArray, const unsigned int* baseIndices, unsigned short int baseIndiceCount)
	{
		if (isGUI) {
			for (int i = 0; i < baseTypeCount; i++) {
				Renderer::commitGUIPrimitive(&verticeArray[i], GetElementCount(baseType), baseIndices, baseIndiceCount);
			}
			return;
		}
		for (int i = 0; i < baseTypeCount; i++) {
			Renderer::commitPrimitive(&verticeArray[i], GetElementCount(baseType), baseIndices, baseIndiceCount);
		}
		return;
	}

	//Pass collected primitives to buffer for draw
	void bufferVideoData(RenderQueue<void*>& verticesArray, std::vector<unsigned int>& verticesSizeArray,
		RenderQueue<void*>& indicesArray, std::vector<unsigned int>& indicesSizeArray);
	void clearPrimitiveSizes(std::vector<unsigned int>& verticesSizeArray, std::vector<unsigned int>& indicesSizeArray);
	
	//GL Objects for rendering - used once per draw call
	VertexBuffer m_VB;
	IndexBuffer m_IB;
	VertexArray m_VA;
	VertexBufferLayout m_VBL;

	//data pointers and size
	//general primitives
	static RenderQueue<void*> s_PrimitiveVertices;
	static std::vector<unsigned int> s_PrimitiveVerticesSize;
	static RenderQueue<void*> s_PrimitiveIndices;
	static std::vector<unsigned int> s_PrimitiveIndicesSize;

	//GUI primitives
	static RenderQueue<void*> s_GUIVertices;
	static std::vector<unsigned int> s_GUIVerticesSize;
	static RenderQueue<void*> s_GUIIndices;
	static std::vector<unsigned int> s_GUIIndicesSize;
};


