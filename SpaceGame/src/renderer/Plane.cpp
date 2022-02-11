#include "renderer/Plane.h"

Plane::~Plane() {
	if (m_Quads != NULL) {
		delete[] m_Quads;
	}
}

void Plane::genQuads(float xPos, float yPos, float width, float height, float tileSize, Axis axis, float angle) {
	if (m_Quads != NULL) {
		delete[] m_Quads;
	}
	
	//Gets how many x and y for tiles
	m_XCount = width / tileSize;
    m_YCount = height / tileSize;

	m_Quads = new Quad [m_XCount * m_YCount];
	
	for (int y = 0; y < m_YCount; y++) {
		for (int x = 0; x < m_XCount; x++) {
			float tileXPos = (x * tileSize) + xPos;
			float tileYPos = (y * tileSize) + yPos;
			Quad quad = CreateQuad(tileXPos, tileYPos, tileSize, tileSize, 0.0f, 0.0f, 0.5f, 0.5f);
			RotateShape(&quad, { 0.0f, 0.0f, 0.0f }, angle, Shape::QUAD, axis);
			m_Quads[x * m_YCount + y] = quad;
		}
	}
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

void Plane::render() 
{	
	for (int i = 0; i < m_XCount * m_YCount; i++) {
		m_Renderer->commit((Vertex*)&m_Quads[i], GetFloatCount(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT);
	}
}

Quad* Plane::accessQuad(unsigned int x, unsigned int y) {
	if (&m_Quads[x * m_YCount + y]) {
		return &m_Quads[x * m_YCount + y];
	}
	return &m_Quads[0];
}