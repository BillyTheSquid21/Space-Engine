#pragma once
#ifndef PLANE_H
#define PLANE_H

#include "Vertex.hpp"
#include <vector>
#include "Renderer.hpp"

class Plane
{
public:
	~Plane();

	//Generate planes
	void generatePlaneXY(float xPos, float yPos, float width, float height, float tileSize);
	void generatePlaneXZ(float xPos, float zPos, float width, float height, float tileSize);
	void generatePlaneYZ(float yPos, float zPos, float width, float height, float tileSize);

	//Render
	void render();
	void setRenderer(Renderer<Vertex>* ren) { m_Renderer = ren; }

	//Access specific tile
	Quad* accessQuad(unsigned int x, unsigned int y);

private:
	//Helper
	void genQuads(float xPos, float yPos, float width, float height, float tileSize, Axis axis, float angle);

	Renderer<Vertex>* m_Renderer = nullptr;
	Quad* m_Quads;
	unsigned int m_XCount = 0; unsigned int m_YCount = 0;
};

#endif