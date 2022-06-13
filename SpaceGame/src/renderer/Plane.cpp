#include "renderer/Plane.h"

Plane::~Plane() {
	purgeData();
}

void Plane::genQuads(float xPos, float yPos, float width, float height, float tileSize, Axis axis, float angle) {
	//If required, purge existing data
	purgeData();

	//Define from bottom left so bump up one
	yPos += tileSize;
	
	//Gets how many x and y for tiles
	m_XCount = width / tileSize;
    m_YCount = height / tileSize;

	//Find total number of quads
	unsigned int quadCount = m_XCount * m_YCount;

	m_Quads = new Norm_Tex_Quad[quadCount];
	
	for (int y = 0; y < m_YCount; y++) {
		for (int x = 0; x < m_XCount; x++) {
			float tileXPos = ((float)x * tileSize) + xPos;
			float tileYPos = ((float)y * tileSize) + yPos;
			Norm_Tex_Quad quad = CreateNormalTextureQuad(tileXPos, tileYPos, tileSize, tileSize, 0.0f, 0.0f, 0.0f, 0.0f);
			RotateShape<NormalTextureVertex>(&quad, { 0.0f, 0.0f, 0.0f }, angle, Shape::QUAD, axis);
			unsigned int quadIndex = (x * m_YCount) + y;
			m_Quads[quadIndex] = quad;
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

void Plane::generatePlaneXY(float xPos, float yPos, float width, float height, float tileSize) 
{
	genQuads(xPos, yPos, width, height, tileSize, Axis::Z, 0.0f);
}

void Plane::generatePlaneXZ(float xPos, float zPos, float width, float height, float tileSize)
{
	genQuads(xPos, zPos, width, height, tileSize, Axis::X, -90.0f);
}

void Plane::generatePlaneYZ(float yPos, float zPos, float width, float height, float tileSize)
{
	genQuads(yPos, zPos, width, height, tileSize, Axis::Y, 90.0f);
}

void Plane::generatePlaneNormals()
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

void Plane::render() 
{	
	if (!m_Quads || !m_SafeToDraw)
	{
		return;
	}
	m_Renderer->commit((NormalTextureVertex*)&m_Quads[0], GetFloatCount<NormalTextureVertex>(Shape::QUAD) * (m_XCount * m_YCount), (unsigned int*)&m_Indices[0], m_Indices.size());
}

Norm_Tex_Quad* Plane::accessQuad(unsigned int x, unsigned int y) {
	if (&m_Quads[x * m_YCount + y]) {
		return &m_Quads[x * m_YCount + y];
	}
	return &m_Quads[0];
}

void Plane::texturePlane(float u, float v, float width, float height) {
	for (int i = 0; i < m_XCount * m_YCount; i++) {
		SetQuadUV((NormalTextureVertex*)&m_Quads[i], u, v, width, height);
	}
}

void Plane::purgeData() {
	if (m_Quads != NULL) {
		delete[] m_Quads;
		m_Quads = nullptr;
		m_SafeToDraw = false;
	}
}