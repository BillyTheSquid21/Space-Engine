#pragma once
#ifndef RENDERER_H
#define RENDERER_H

#include "GLClasses.h"
#include "Vertex.h"
#include "map"
#include "renderer/Mesh.h"
#include "services/RenderService.hpp"

namespace SGRender
{
	class Renderer
	{
	public:
		Renderer() : m_VA() {};

		void generate();
		void setLayout(VertexType type);
		void setDrawingMode(GLenum type);

		//Commit either vertices to batch, or instances
		//More instance types could be added if so desired

		//Commit geometry - either commit with no transform or with for whole mesh, otherwise it will break currently
		void commit(Mesh& mesh, ModelID modelID, MeshID meshID);
		void commit(Mesh& mesh, ModelID modelID, MeshID meshID, glm::mat4& transform);

		//Instance commit - TODO implement
		void commit(glm::mat4& instance);

		//Remove geometry
		void remove(ModelID modelID, MeshID meshID);
		void remove(ModelID modelID, MeshID meshID, glm::mat4& transform);

		//Issues a draw call
		void drawPrimitives();

	private:
		typedef std::pair<ModelID, MeshID> InternalKey;

	private:
		void batchGeometry();
		bool modelExists(ModelID model, MeshID mesh);
		int32_t findTransform(ModelID modelID, MeshID meshID, glm::mat4& transform); //-1 if not found
	private:
		//Type of primitive being drawn
		GLenum m_PrimitiveType = GL_TRIANGLES;

		//Type of vertex
		VertexType m_VertexType = V_Vertex;

		//Geometry and Instance Buffers
		std::map<InternalKey, Mesh> m_Geometry;								//Keeps copy of geometry TODO make not need to store a copy of all geometry
		std::map<InternalKey, std::vector<glm::mat4>> m_BatchedTransforms;	//For storing additional transforms to be applied one after the other on batching - can be empty
		bool m_Unbatched = true;											//Whether needs to buffer

		//GL Objects for rendering - used once per draw call
		VertexBuffer m_VB;
		IndexBuffer m_IB;
		VertexArray m_VA;
		VertexBufferLayout m_VBL;

		size_t m_VertBuffWidth = 0;
		size_t m_IndBuffWidth = 0;
	};
}

#endif