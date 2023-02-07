#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include "stdint.h"
#include "renderer/Geometry.h"
#include "renderer/GLClasses.h"
#include "renderer/Camera.h"
#include "utility/SegArray.hpp"

namespace SGRender
{
	/**
	* Is the base class of derived renderer types (eg batcher, instancer)
	* Has functionality for instancing
	*/
	class RendererBase
	{
	public:
		RendererBase() : m_VA() {};

		/**
		* Sets the vertex layout, pushing to the buffer layout object. Floats are recommended.
		*
		* @param stride Stride (width) of attribute (Eg for position - x, y, z is 3)
		*/
		template<typename Type>
		void setLayout(unsigned char stride) { m_VBL.push<Type>(stride, false); }
		template<typename Type>
		void setLayout(unsigned char stride1, unsigned char stride2) { m_VBL.push<Type>(stride1, false); m_VBL.push<Type>(stride2, false); }
		template<typename Type>
		void setLayout(unsigned char stride1, unsigned char stride2, unsigned char stride3) { m_VBL.push<Type>(stride1, false); m_VBL.push<Type>(stride2, false); m_VBL.push<Type>(stride3, false); }
		template<typename Type>
		void setLayout(unsigned char stride1, unsigned char stride2, unsigned char stride3, unsigned char stride4) { m_VBL.push<Type>(stride1, false); m_VBL.push<Type>(stride2, false); m_VBL.push<Type>(stride3, false); m_VBL.push<Type>(stride4, false);}

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

		//GL Objects for rendering - used once per draw call
		VertexBuffer m_VB;
		IndexBuffer m_IB;
		VertexArray m_VA;
		VertexBufferLayout m_VBL;
		unsigned int m_VertBuffWidth = 0;
	};
}

#endif