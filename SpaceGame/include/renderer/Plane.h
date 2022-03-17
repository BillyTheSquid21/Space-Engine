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

	//Load plane vertices from file - TODO: implement
	void loadPlane();
	
	//Render
	void render();
	void setRenderer(Render::Renderer<TextureVertex>* ren) { m_Renderer = ren; }

	//Access specific tile
	TextureQuad* accessQuad(unsigned int x, unsigned int y);
	void texturePlane(float u, float v, float width, float height);

	//Purges data not needed without destruction of class
	void purgeData();

private:
	//Helper
	void genQuads(float xPos, float yPos, float width, float height, float tileSize, Axis axis, float angle);

	Render::Renderer<TextureVertex>* m_Renderer = nullptr;
	TextureQuad* m_Quads;
	unsigned int m_XCount = 0; unsigned int m_YCount = 0;

	//Indices buffer for plane
	std::vector<unsigned int> m_Indices;
};

#endif