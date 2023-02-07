#include "renderer/TextureAtlas.h"

void Tex::ApplyUVTransformation(glm::vec2& uv, UVTrans& trans)
{
	uv *= trans.scale;
	uv += trans.offset;
}

void Tex::UndoUVTransformation(glm::vec2& uv, UVTrans& trans)
{
	uv -= trans.offset;
	uv /= trans.scale;
}

void Tex::TextureAtlas::addTexture(std::string texName, std::string texPath)
{
	if (m_Mapped)
	{
		EngineLog("Cannot add texture when models are mapped!");
		return;
	}

	for (int i = 0; i < m_TextureData.size(); i++)
	{
		if (m_TextureData[i].name == texName)
		{
			EngineLog("Texture already in atlas!");
			return;
		}
	}

	m_TextureData.emplace_back(texName, texPath);
}

void Tex::TextureAtlas::removeTexture(std::string texName)
{
	if (m_Mapped)
	{
		EngineLog("Cannot remove texture when models are mapped!");
		return;
	}

	for (int i = 0; i < m_TextureData.size(); i++)
	{
		if (m_TextureData[i].name == texName)
		{
			m_TextureData.erase(m_TextureData.begin() + i);
			return;
		}
	}
	EngineLog("Texture wasn't found in atlas!");
}

void Tex::TextureAtlas::generateTexture(int slot, int bpp, int flags)
{
	//Load textures from texData
	std::vector<Texture> textureBuffers;
	textureBuffers.resize(m_TextureData.size());
	for (int i = 0; i < m_TextureData.size(); i++)
	{
		textureBuffers[i].loadTexture(m_TextureData[i].path);
	}

	//Get max dim
	int height = 0; int maxWidth = 0;
	for (auto& t : textureBuffers)
	{
		height += t.height();
		int width = t.width();
		if (width > maxWidth)
		{
			maxWidth = width;
		}
	}

	//Allocate buffer
	int bufferSize = height * maxWidth * bpp;
	std::vector<uint8_t> buffer;
	buffer.resize(bufferSize);

	//Copy across data
	int lastHeight = 0;
	for (int i = 0; i < textureBuffers.size(); i++)
	{
		int w = textureBuffers[i].width();
		int h = textureBuffers[i].height();
		uint8_t* b = textureBuffers[i].buffer();
		for (int y = lastHeight; y < lastHeight + h; y++)
		{
			//Copy across each row
			int bufferRowStart = maxWidth * bpp * y;
			int textureRowWidth = w * bpp;
			int textureRowStart = textureRowWidth * y;
			
			memcpy(&buffer[0] + bufferRowStart, b + textureRowStart, textureRowWidth);
		}
		textureBuffers[i].clearBuffer();

		//Update Transformation
		m_Transforms[m_TextureData[i].name].trans.scale.y = (float)h/(float)height;
		m_Transforms[m_TextureData[i].name].trans.scale.x = (float)w/(float)maxWidth;
		m_Transforms[m_TextureData[i].name].trans.offset.y = (float)lastHeight/(float)height;
	}

	//Generate Texture
	m_Texture.setWidth(maxWidth); m_Texture.setHeight(height); m_Texture.setBPP(bpp);
	m_Texture.generateTexture(slot, &buffer[0], flags);
}

void Tex::TextureAtlas::applyTransforms()
{
	if (m_Mapped)
	{
		return;
	}

	for (std::pair<std::string, AtlasSegment> element : m_Transforms)
	{
		AtlasSegment segment = element.second;
		for (auto& v : segment.vertices)
		{
			for (int i = 3; i < v.count; i += m_Stride)
			{
				//Get uv and map
				glm::vec2* uv = (glm::vec2*)&v.verts[i];
				ApplyUVTransformation(*uv, segment.trans);
			}
		}
	}
	m_Mapped = true;
}

void Tex::TextureAtlas::undoTransforms()
{
	if (!m_Mapped)
	{
		return;
	}

	for (std::pair<std::string, AtlasSegment> element : m_Transforms)
	{
		AtlasSegment segment = element.second;
		for (auto& v : segment.vertices)
		{
			for (int i = 3; i < v.count; i += m_Stride)
			{
				//Get uv and unmap
				glm::vec2* uv = (glm::vec2*)&v.verts[i];
				UndoUVTransformation(*uv, segment.trans);
			}
		}
	}
	m_Mapped = false;
}

void Tex::TextureAtlas::linkModel(std::string texture, float* vertices, int count)
{
	if (m_Mapped)
	{
		EngineLog("Cannot link while vertices are mapped!");
		return;
	}

	if (m_Transforms.find(texture) == m_Transforms.end())
	{
		EngineLog("Texture is not in atlas!");
		return;
	}

	AtlasSegment& segment = m_Transforms[texture];

	//Check if already exists
	for (auto& v : segment.vertices)
	{
		if (v.verts == vertices)
		{
			EngineLog("Model already exists in atlas!");
			return;
		}
	}

	segment.vertices.emplace_back(vertices, count);
}

void Tex::TextureAtlas::unlinkModel(std::string texture, float* vertices)
{
	if (!m_Mapped)
	{
		EngineLog("Cannot unlink while vertices aren't mapped!");
		return;
	}

	if (m_Transforms.find(texture) == m_Transforms.end())
	{
		EngineLog("Texture is not in atlas!");
		return;
	}

	AtlasSegment& segment = m_Transforms[texture];
	
	//Check if already exists
	for (int i = 0; i < segment.vertices.size(); i++)
	{
		if (segment.vertices[i].verts == vertices)
		{
			segment.vertices.erase(segment.vertices.begin() + i);
			return;
		}
	}
	EngineLog("Model wasn't found in atlas!");
}