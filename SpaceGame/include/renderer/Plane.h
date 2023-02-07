#pragma once
#ifndef PLANE_H
#define PLANE_H

#include "Vertex.hpp"
#include "vector"
#include "string"
#include "stdint.h"
#include "RenderSys.h"

namespace Geometry
{
	class Plane
	{
	public:
		~Plane();

		//Generate planes
		void generatePlaneXY(float xPos, float yPos, float width, float height, float tileSize);
		void generatePlaneXZ(float xPos, float zPos, float width, float height, float tileSize);
		void generatePlaneYZ(float yPos, float zPos, float width, float height, float tileSize);
		void generatePlaneNormals();

		//Load plane vertices from file - TODO: implement
		virtual void loadPlane() {};

		//Render
		void render();
		void setRenderer(const char* ren) { m_Linked = SGRender::System::linkToBatcher(ren, m_Renderer); if (m_Linked) { SGRender::System::commitToBatcher(m_Renderer, this, (float*)&m_Quads[0], GetFloatCount<SGRender::NTVertex>(Shape::QUAD) * m_Quads.size(), (unsigned int*)&m_Indices[0], m_Indices.size());} }

		//Access specific tile
		Norm_Tex_Quad* accessQuad(unsigned int x, unsigned int y);
		void texturePlane(float u, float v, float width, float height);

		//Purges data not needed without destruction of class
		void purgeData();

	protected:
		//Helper
		void genQuads(float xPos, float yPos, float width, float height, float tileSize, SGRender::Axis axis, float angle);

		uint32_t m_Renderer = 0;
		bool m_Linked = false;
		std::vector<Norm_Tex_Quad> m_Quads;
		uint32_t m_XCount = 0; uint32_t m_YCount = 0;

		//Indices buffer for plane
		bool m_SafeToDraw = false;
		std::vector<unsigned int> m_Indices;
	};
}

#endif