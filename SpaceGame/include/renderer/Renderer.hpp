#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include "stdint.h"
#include "renderer/GLClasses.h"
#include "renderer/Mesh.h"
#include "renderer/Camera.h"
#include "renderer/Vertex.h"
#include "utility/SegArray.hpp"

namespace SGRender
{
	typedef int32_t RenderLinkID; //ID so object can reserve unique spot in renderer

	/**
	* Is the base class of derived renderer types (eg batcher, instancer)
	* Has functionality for instancing
	*/
	class RendererBase
	{
	public:
		RendererBase() : m_VA() {};

		/**
		* Sets the vertex layout, pushing to the buffer layout object.
		*
		* @param vertexType The type of vertex, from the Vertex::properties() function
		*/
		void setLayout(VertexType vertexType)
		{
			//All Vertexes have position so add first
			//If this isn't true, there are probably more problems
			m_VBL.push<float>(3, false);

			//Check each bit for property
			for (int i = 0; i < 8; i++)
			{
				int curr_flag = vertexType & (1UL << i);
				int flag_stride = PropertyStride((VProperties)curr_flag);
				if (flag_stride != -1)
				{
					m_VBL.push<float>(flag_stride, false);
				}
			}

			m_VertexType = vertexType;
		}

		/**
		* Sets the drawing mode, defaulting to GLTriangles.
		*/
		void setDrawingMode(GLenum type) { m_PrimitiveType = type; }

		/**
		* Called when collected primitives are to be drawn
		*
		*/
		virtual void drawPrimitives() {}

		/**
		* Gathers vertice data into renderer buffers
		*/
		virtual void bufferVideoData() {}

	protected:

		//Helper functions
		void bindAll() { m_VA.bind();	m_IB.bind(); }

		//type of primitive being drawn
		GLenum m_PrimitiveType = GL_TRIANGLES;

		//type of vertex
		VertexType m_VertexType = V_Vertex;

		//GL Objects for rendering - used once per draw call
		VertexBuffer m_VB;
		IndexBuffer m_IB;
		VertexArray m_VA;
		VertexBufferLayout m_VBL;
		unsigned int m_VertBuffWidth = 0;
	};
}

#endif