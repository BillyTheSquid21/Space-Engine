#pragma once
#ifndef TEXTURE_ATLAS_H
#define TEXTURE_ATLAS_H

#include "renderer/Texture.h"
#include "renderer/Geometry.h"
#include "unordered_map"

namespace Tex
{
	struct UVTrans
	{
		glm::vec2 offset = { 0.0f, 0.0f };
		glm::vec2 scale = { 1.0f, 1.0f };
	};

	struct AtlasSegment
	{
		UVTrans trans; //Stores the transform for a texture
		std::vector<Geometry::VertexMeta> vertices; //Stores the vertices attatched
	};

	static void ApplyUVTransformation(glm::vec2& uv, UVTrans& trans);
	static void UndoUVTransformation(glm::vec2& uv, UVTrans& trans);

	class TextureAtlas
	{
	public:
		void setStride(int stride) { m_Stride = stride; }
		void addTexture(std::string texName, std::string texPath);
		void removeTexture(std::string texName);
		void linkModel(std::string texture, float* vertices, int count);
		void unlinkModel(std::string texture, float* vertices);
		void generateTexture(int slot, int bpp, int flags);
		void applyTransforms();
		void undoTransforms();
		Texture* texture() { return &m_Texture; }

	private:
		struct TexData
		{
			std::string name;
			std::string path;
		};

		uint32_t m_Stride = 5; //Stride of vertex
		std::vector<TexData> m_TextureData;
		std::unordered_map<std::string, AtlasSegment> m_Transforms;
		Texture m_Texture;
		bool m_Mapped = false;
	};
}

#endif