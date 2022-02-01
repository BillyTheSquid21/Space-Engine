#include "Renderer.h"

//RENDERER

//Camera
Camera::Camera(float width, float height)
	:m_CameraWidth{width}, m_CameraHeight{height}, 
	m_Projection{glm::ortho(-width/2, width/2, -height/2, height/2, -1.0f, 2.0f)} //depth buffer layer - above +1 is GUI
{
	m_CameraBounds = { -m_CameraWidth / 2,m_CameraWidth / 2,-m_CameraHeight / 2,m_CameraHeight / 2 };
}

void Camera::positionCamera(float x, float y) {
	m_Translation[3][0] = x;
	m_Translation[3][1] = y;
	m_Translation[3][2] = 1.0f;

	//position bounds
	m_CameraBounds = {x - (m_CameraWidth/2), x + (m_CameraWidth/2), y - (m_CameraHeight/2), y + (m_CameraHeight/2)};
}

void Camera::translateCamera(float deltaX, float deltaY) {
	m_Translation[3][0] += deltaX;
	m_Translation[3][1] += deltaY;
	m_Translation[3][2] = 1.0f;

	//translate bounds
	m_CameraBounds.a += deltaX;
	m_CameraBounds.b += deltaX;
	m_CameraBounds.c += deltaY;
	m_CameraBounds.d += deltaY;
}

void Camera::zoomCamera(float deltaZoom) {
	setZoomCamera(m_Scaling.x + deltaZoom);
}

void Camera::setZoomCamera(float zoom) {
	//correct back to original scale
	m_CameraBounds.a *= m_Scaling.x; m_CameraBounds.b *= m_Scaling.x;
	m_CameraBounds.c *= m_Scaling.y; m_CameraBounds.d *= m_Scaling.y;

	//scale
	m_Scaling.x = zoom;
	m_Scaling.y = zoom;

	//scale accordingly
	m_CameraBounds.a /= m_Scaling.x; m_CameraBounds.b /= m_Scaling.x;
	m_CameraBounds.c /= m_Scaling.y; m_CameraBounds.d /= m_Scaling.y;
}

void Camera::sendCameraUniforms(Shader& shader) {
	//multiply
	m_ResultantMatrix = glm::scale(m_Projection * m_Translation, m_Scaling);
	shader.setUniform("u_Projection", &m_ResultantMatrix);
}

//GUI
GUI::GUI(float width, float height) 
	:m_GUIWidth{ width }, m_GUIHeight{ height },
	m_Projection{ glm::ortho(-width / 2, width / 2, -height, 0.0f, -1.0f, 2.0f) }
{}

void GUI::setScale(float scale) {
	//scale
	m_Scaling.x = scale;
	m_Scaling.y = scale;
}

void GUI::sendGUIUniforms(Shader& shader) {
	//multiply
	m_ResultantMatrix = glm::scale(m_Projection, m_Scaling);
	shader.setUniform("u_Projection", &m_ResultantMatrix);
}

//declare vectors
RenderQueue<void*> Renderer::s_PrimitiveVertices;
std::vector<unsigned int> Renderer::s_PrimitiveVerticesSize;
RenderQueue<void*> Renderer::s_PrimitiveIndices;
std::vector<unsigned int> Renderer::s_PrimitiveIndicesSize;

RenderQueue<void*> Renderer::s_GUIVertices;
std::vector<unsigned int> Renderer::s_GUIVerticesSize;
RenderQueue<void*> Renderer::s_GUIIndices;
std::vector<unsigned int> Renderer::s_GUIIndicesSize;

Renderer::Renderer() 
	: m_VA{VertexArray::VertexArray()}
{}

void Renderer::init(float width, float height)
{
	//Init camera
	camera = Camera::Camera(width, height);
	gui = GUI::GUI(width, height);

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

void Renderer::clearScreen() const {
	/* Render here */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::commitShape(const void* vert, Shape derivedType, Shape baseType, bool isGUI) 
{
	//If is a simple primitive, will just go through
	unsigned short int baseTypeCount = GetBasePrimitiveCount(derivedType);
	const unsigned int* baseIndices = NULL;
	unsigned short int baseIndiceCount = 0;
	switch (baseType) 
	{
		{
	case Shape::LINE:
		baseIndices = Renderer::s_Line_I;
		baseIndiceCount = Renderer::IND_LINE;
		Line* lineArray = (Line*)vert;
		commitInternal(isGUI, baseTypeCount, baseType, lineArray, baseIndices, baseIndiceCount);
		break;
		}
		{
	case Shape::TRI:
		baseIndices = Renderer::s_Tri_I;
		baseIndiceCount = Renderer::IND_TRI;
		Tri* triArray = (Tri*)vert;
		commitInternal(isGUI, baseTypeCount, baseType, triArray, baseIndices, baseIndiceCount);
		break;
		}
		{
	case Shape::QUAD:
		baseIndices = Renderer::s_Quad_I;
		baseIndiceCount = Renderer::IND_QUAD;
		Quad* quadArray = (Quad*)vert;
		commitInternal(isGUI, baseTypeCount, baseType, quadArray, baseIndices, baseIndiceCount);
		break;
		}
		{
	case Shape::CIRCLE:
		baseIndices = Renderer::s_Circle_I;
		baseIndiceCount = Renderer::IND_CIRCLE;
		Circle* circleArray = (Circle*)vert;
		commitInternal(isGUI, baseTypeCount, baseType, circleArray, baseIndices, baseIndiceCount);
		break;
		}
		{
	default:
		baseIndices = Renderer::s_Tri_I;
		baseIndiceCount = Renderer::IND_TRI;
		Tri* failedArray = (Tri*)vert;
		commitInternal(isGUI, baseTypeCount, baseType, failedArray, baseIndices, baseIndiceCount);
		EngineLog("Incorrect shape ID passed to renderer");
		break;
		}
	}

}

void Renderer::commitPrimitive(void* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize) {
	s_PrimitiveVertices.pushBack(vert);
	s_PrimitiveVerticesSize.push_back(vertSize);
	s_PrimitiveIndices.pushBack((void*)ind);
	s_PrimitiveIndicesSize.push_back(indSize);
}

void Renderer::commitGUIPrimitive(void* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize) {
	s_GUIVertices.pushBack(vert);
	s_GUIVerticesSize.push_back(vertSize);
	s_GUIIndices.pushBack((void*)ind);
	s_GUIIndicesSize.push_back(indSize);
}

void Renderer::clearPrimitiveSizes(std::vector<unsigned int>& verticesSizeArray,
	std::vector<unsigned int>& indicesSizeArray) {
	indicesSizeArray.clear();
	verticesSizeArray.clear();
}

void Renderer::bufferVideoData(RenderQueue<void*>&verticesArray, std::vector<unsigned int> &verticesSizeArray,
	RenderQueue<void*>& indicesArray, std::vector<unsigned int>& indicesSizeArray) {
	{
		//First get amount of data among all vertice floats
		unsigned int totalVertFloats = 0;
		for (unsigned int i = 0; i < verticesSizeArray.size(); i++) {
			totalVertFloats += verticesSizeArray[i];
		}

		//Then create buffer with space for that many floats
		//Using array for vertices
		std::vector<float> vertices;
		vertices.resize(totalVertFloats);
		auto verticesIterator = vertices.begin();
		unsigned int index = 0;

		//Add all the vertice data
		int sizeArrayIndex = 0;
		while (verticesArray.itemsWaiting()) {
			const float* dataPointer = (const float*)verticesArray.nextInQueue();
			unsigned int dataSize = verticesSizeArray[sizeArrayIndex];
			sizeArrayIndex++;

			//Copy into vector
			std::copy(&dataPointer[0], &dataPointer[dataSize], verticesIterator + index);
			index += dataSize;
		}
		m_VB.bufferData(vertices.data(), vertices.size());
	}

	//next get amount of data among all indice ints
	unsigned int totalIndFloats = 0;
	for (unsigned int i = 0; i < indicesArray.size(); i++) {
		totalIndFloats += indicesSizeArray[i];
	}

	//Then create buffer with space for that many ints
	std::vector<unsigned int> indices;
	indices.resize(totalIndFloats);
	auto indicesIterator = indices.begin();
	unsigned int index = 0;

	//Store last biggest indice
	unsigned int largestInd = 0;

	//Add all the indice data
	int indicesSizeIndex = 0;
	while (indicesArray.itemsWaiting()) {
		const unsigned int* dataPointer = (const unsigned int*)indicesArray.nextInQueue();
		unsigned int dataSize = indicesSizeArray[indicesSizeIndex];

		//Add to vector one by one
		unsigned int currentLargest = 0;
		for (int j = 0; j < dataSize; j++) {
			unsigned int newValue = dataPointer[j] + largestInd;
			indices[index] = newValue + indicesSizeIndex;
			index++;
			if (newValue > currentLargest) {
				currentLargest = newValue;
			}
		}
		largestInd = currentLargest;

		indicesSizeIndex++;
	}
	m_IB.bufferData(indices.data(), indices.size());

	//clear list
	clearPrimitiveSizes(verticesSizeArray, indicesSizeArray);
}

void Renderer::drawPrimitives(Shader& shader) {
	//OBJECTS DRAW CALL
	//buffer data
	bufferVideoData(s_PrimitiveVertices, s_PrimitiveVerticesSize, s_PrimitiveIndices, s_PrimitiveIndicesSize);
	//Use camera
	camera.sendCameraUniforms(shader);
	bindAll(shader);
	glDrawElements(GL_TRIANGLES, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr);

	//GUI DRAW CALL
	//buffer data
	bufferVideoData(s_GUIVertices, s_GUIVerticesSize, s_GUIIndices, s_GUIIndicesSize);
	//Use camera
	gui.sendGUIUniforms(shader);
	bindAll(shader);
	glDrawElements(GL_TRIANGLES, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader) const {
	shader.bind();
	va.bind();
	ib.bind();
	glDrawElements(GL_TRIANGLES, ib.GetCount(), GL_UNSIGNED_INT, nullptr);
}

void Renderer::bindAll(Shader& shader) {
	shader.bind();
	m_VA.bind();
	m_IB.bind();
}

//constant indices for a given supported shape
const unsigned int Renderer::s_Tri_I[] =
{
	0, 1, 2
};

const unsigned int Renderer::s_Quad_I[] =
{
	0, 1, 2,
	2, 3, 0
};

const unsigned int Renderer::s_Line_I[] =
{
	0, 1, 2,
	2, 3, 0
};

const unsigned int Renderer::s_Circle_I[] =
{
	0, 1, 2,
	0, 2, 3,
	0, 3, 4,
	0, 4, 5,
	0, 5, 6,
	0, 6, 7,
	0, 7, 8,
	0, 8, 9,
	0, 9, 10,
	0, 10, 11,
	0, 11, 12,
	0, 12, 13,
	0, 13, 14,
	0, 14, 15,
	0, 15, 16,
	0, 16, 17,
	0, 17, 18,
	0, 18, 19, 
	0, 19, 20,
	0, 20, 21,
	0, 21, 22,
	0, 22, 23,
	0, 23, 24,
	0, 24, 25,
	0, 25, 26,
	0, 26, 27,
	0, 27, 28,
	0, 28, 29,
	0, 29, 30,
	0, 30, 31,
	0, 31, 32,
	0, 32, 33,
	0, 33, 34,
	0, 34, 35,
	0, 35, 36,
	0, 36, 37,
	0, 37, 38,
	0, 38, 39,
	0, 39, 40,
	0, 40, 41,
	0, 41, 42,
	0, 42, 43,
	0, 43, 44,
	0, 44, 45,
	0, 45, 46,
	0, 46, 47,
	0, 47, 48,
	0, 48, 49,
	0, 49, 50, 
	0, 50, 51,
	0, 51, 52,
	0, 52, 53,
	0, 53, 54,
	0, 54, 55,
	0, 55, 56,
	0, 56, 57,
	0, 57, 58,
	0, 58, 59,
	0, 59, 60,
	0, 60, 61,
	0, 61, 62,
	0, 62, 63,
	0, 63, 64
};
 

