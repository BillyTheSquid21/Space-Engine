#include "renderer/Plane.h"

Geometry::Plane::~Plane() {
	if (m_Linked)
	{
		SGRender::System::removeFromBatcher(m_Renderer, this, &m_Quads[0]);
		SGRender::System::unlinkFromBatcher(m_Renderer);
	}
	purgeData();
}

void Geometry::Plane::genQuads(float xPos, float yPos, float width, float height, float tileSize, SGRender::Axis axis, float angle) {
	//If required, purge existing data
	purgeData();
	m_SafeToDraw = false;

	//Define from bottom left so bump up one
	yPos += tileSize;
	
	//Gets how many x and y for tiles
	m_XCount = width / tileSize;
    m_YCount = height / tileSize;

	//Find total number of quads
	unsigned int quadCount = m_XCount * m_YCount;

	m_Quads.resize(quadCount);
	
	for (int y = 0; y < m_YCount; y++) {
		for (int x = 0; x < m_XCount; x++) {
			float tileXPos = ((float)x * tileSize) + xPos;
			float tileYPos = ((float)y * tileSize) + yPos;
			Norm_Tex_Quad quad = CreateNormalTextureQuad(tileXPos, tileYPos, tileSize, tileSize, 0.0f, 0.0f, 0.0f, 0.0f);
			AxialRotate<SGRender::NTVertex>(&quad, { 0.0f, 0.0f, 0.0f }, angle, Shape::QUAD, axis);
			unsigned int quadIndex = (x * m_YCount) + y;
			m_Quads.at(quadIndex) = quad;
		}
	}
	//Buffer indices to minimise counts of data sent to render queue

	//Find total ints needed
	unsigned int quadIntCount = quadCount * Primitive::Q_IND_COUNT;

	//Resize vector to be able to fit and init index
	m_Indices.resize(quadIntCount);
	unsigned int indicesIndex = 0;

	unsigned int lastLargest = -1;
	unsigned int indicesTemp[6]{ 0,1,2,0,2,3 };
	for (unsigned int i = 0; i < quadCount; i++) {
		//Increment all by last largest - set temp to base
		std::copy(&Primitive::Q_IND[0], &Primitive::Q_IND[Primitive::Q_IND_COUNT], &indicesTemp[0]);
		for (int j = 0; j < Primitive::Q_IND_COUNT; j++) {
			indicesTemp[j] += lastLargest + 1;
		}
		//Set last largest
		lastLargest = indicesTemp[Primitive::Q_IND_COUNT - 1];
		//Copy into vector
		std::copy(&indicesTemp[0], &indicesTemp[Primitive::Q_IND_COUNT], m_Indices.begin() + indicesIndex);
		//Increment index
		indicesIndex += Primitive::Q_IND_COUNT;
	}
	m_SafeToDraw = true;
}

void Geometry::Plane::generatePlaneXY(float xPos, float yPos, float width, float height, float tileSize)
{
	genQuads(xPos, yPos, width, height, tileSize, SGRender::Axis::Z, 0.0f);
}

void Geometry::Plane::generatePlaneXZ(float xPos, float zPos, float width, float height, float tileSize)
{
	genQuads(xPos, zPos, width, height, tileSize, SGRender::Axis::X, -90.0f);
}

void Geometry::Plane::generatePlaneYZ(float yPos, float zPos, float width, float height, float tileSize)
{
	genQuads(yPos, zPos, width, height, tileSize, SGRender::Axis::Y, 90.0f);
}

void Geometry::Plane::generatePlaneNormals()
{
	int total = m_XCount * m_YCount;
	//set all normals to 0
	for (int i = 0; i < total; i++)
	{
		m_Quads[i].at(0).normals = glm::vec3(0.0f, 0.0f, 0.0f);
		m_Quads[i].at(1).normals = glm::vec3(0.0f, 0.0f, 0.0f);
		m_Quads[i].at(2).normals = glm::vec3(0.0f, 0.0f, 0.0f);
		m_Quads[i].at(3).normals = glm::vec3(0.0f, 0.0f, 0.0f);
	}
	//calc each triangle normal
	for (int i = 0; i < total; i++)
	{
		//Tri 1 - 0,1,2
		glm::vec3 tri1Edge1 = m_Quads[i].at(1).position - m_Quads[i].at(0).position;
		glm::vec3 tri1Edge2 = m_Quads[i].at(0).position - m_Quads[i].at(2).position;
		glm::vec3 tri1Norm = glm::normalize(glm::cross(tri1Edge1, tri1Edge2));
		//Set norms 1
		m_Quads[i].at(0).normals += tri1Norm;
		m_Quads[i].at(1).normals += tri1Norm;
		m_Quads[i].at(2).normals += tri1Norm;
		
		//Tri 2 - 0,2,3
		glm::vec3 tri2Edge1 = m_Quads[i].at(2).position - m_Quads[i].at(0).position;
		glm::vec3 tri2Edge2 = m_Quads[i].at(0).position - m_Quads[i].at(3).position;
		glm::vec3 tri2Norm = glm::normalize(glm::cross(tri2Edge1, tri2Edge2));
		
		//Set norms 2
		m_Quads[i].at(0).normals += tri2Norm;
		m_Quads[i].at(2).normals += tri2Norm;
		m_Quads[i].at(3).normals += tri2Norm;

		//Normalise all
		m_Quads[i].at(0).normals = glm::normalize(m_Quads[i].at(0).normals);
		m_Quads[i].at(1).normals = glm::normalize(m_Quads[i].at(1).normals);
		m_Quads[i].at(2).normals = glm::normalize(m_Quads[i].at(2).normals);
		m_Quads[i].at(3).normals = glm::normalize(m_Quads[i].at(3).normals);
	}
}

void Geometry::Plane::render()
{	
	if (m_Quads.size() <= 0 || !m_SafeToDraw)
	{
		return;
	}
}

Norm_Tex_Quad* Geometry::Plane::accessQuad(unsigned int x, unsigned int y) {
	int index = x * m_YCount + y;
	if (index < m_Quads.size()) {
		return &m_Quads.at(index);
	}
	return &m_Quads[0];
}

void Geometry::Plane::texturePlane(float u, float v, float width, float height) {
	for (int i = 0; i < m_XCount * m_YCount; i++) {
		SetQuadUV((SGRender::NTVertex*)&m_Quads[i], u, v, width, height);
	}
}

void Geometry::Plane::purgeData() {
	if (m_Quads.size() > 0) {
		m_Quads.clear();
		m_Indices.clear();
		m_SafeToDraw = false;
	}
	m_Renderer = 0;
}