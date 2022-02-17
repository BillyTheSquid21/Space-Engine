#include "renderer/Plane.h"

Plane::~Plane() {
	purgeData();
}

void Plane::genQuads(float xPos, float yPos, float width, float height, float tileSize, Axis axis, float angle) {
	purgeData();
	
	//Gets how many x and y for tiles
	m_XCount = width / tileSize;
    m_YCount = height / tileSize;

	m_Quads = new TextureQuad [m_XCount * m_YCount];
	
	for (int y = 0; y < m_YCount; y++) {
		for (int x = 0; x < m_XCount; x++) {
			float tileXPos = (x * tileSize) + xPos;
			float tileYPos = (y * tileSize) + yPos;
			TextureQuad quad = CreateTextureQuad(tileXPos, tileYPos, tileSize, tileSize, 0.0f, 0.0f, 0.5f, 0.5f);
			RotateShape<TextureVertex>(&quad, { 0.0f, 0.0f, 0.0f }, angle, Shape::QUAD, axis);
			m_Quads[x * m_YCount + y] = quad;
		}
	}
	//Buffer indices to minimise counts of data sent to render queue
	//Find total number of quads
	unsigned int quadCount = m_XCount * m_YCount;

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
	m_Renderer->commit((TextureVertex*)&m_Quads[0], GetFloatCount<TextureVertex>(Shape::QUAD) * (m_XCount * m_YCount), (unsigned int*)&m_Indices[0], m_Indices.size());
}

TextureQuad* Plane::accessQuad(unsigned int x, unsigned int y) {
	if (&m_Quads[x * m_YCount + y]) {
		return &m_Quads[x * m_YCount + y];
	}
	return &m_Quads[0];
}

void Plane::purgeData() {
	if (m_Quads != NULL) {
		delete[] m_Quads;
	}
}