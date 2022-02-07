#include "Renderer.h"

//RENDERER

//Camera
Camera::Camera(float width, float height)
	:m_CameraWidth{ width }, m_CameraHeight{ height },
	m_Projection{ glm::ortho(-width / 2, width / 2, -height / 2, height / 2, -1.0f, 2.0f) } //depth buffer layer - above +1 is GUI
{
	m_CameraBounds = { -m_CameraWidth / 2,m_CameraWidth / 2,-m_CameraHeight / 2,m_CameraHeight / 2 };
}

void Camera::positionCamera(float x, float y) {
	m_Translation[3][0] = x;
	m_Translation[3][1] = y;
	m_Translation[3][2] = 1.0f;

	//position bounds
	m_CameraBounds = { x - (m_CameraWidth / 2), x + (m_CameraWidth / 2), y - (m_CameraHeight / 2), y + (m_CameraHeight / 2) };
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

Renderer::Renderer()
	: m_VA{ VertexArray::VertexArray() }
{}

void Renderer::init(float width, float height)
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

void Renderer::commitPrimitive(void* vert, unsigned int vertSize, const unsigned int* ind, unsigned short int indSize) {
	m_PrimitiveVertices.pushBack(vert, vertSize, ind, indSize);
}

void Renderer::bufferVideoData(RenderQueue<void*>& verticesArray) {
	
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
			RenderContainer<void*> instructions = verticesArray.nextInQueue();

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

void Renderer::drawPrimitives(Shader& shader) {
	//Buffer data
	bufferVideoData(m_PrimitiveVertices);
	//Use camera
	camera.sendCameraUniforms(shader);
	//Bind all objects
	bindAll(shader);
	//Draw Elements
	glDrawElements(GL_TRIANGLES, m_IB.GetCount(), GL_UNSIGNED_INT, nullptr);
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