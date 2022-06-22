#pragma once
#ifndef MODEL_OBJECT_H
#define MODEL_OBJECT_H

#include "renderer/Model.hpp"
#include "core/GameObject.hpp"

static const std::string ModelPathBegin = "res/model/";
static const std::string TexturePathBegin = "res/textures/";

class ModelObject : public GameObject
{
public:
	ModelObject(std::string texture, std::string modelPath, Tex::TextureAtlasRGBA& atlas) 
	{ 
		m_Texture = texture; atlas.loadTexture(TexturePathBegin+texture, texture);
		m_Model.load((ModelPathBegin + modelPath).c_str());
	}
	void setRen(Render::Renderer<NormalTextureVertex>* ren) { m_Model.setRen(ren); }

	std::string m_Texture = "";
	Model::Model<NormalTextureVertex> m_Model;
};

class ModelAtlasUpdate : public RenderComponent
{
public:
	ModelAtlasUpdate() = default;
	ModelAtlasUpdate(std::string texture, Model::Model<NormalTextureVertex>* model, Tex::TextureAtlasRGBA* atlas) {m_Texture = texture; m_Model = model; m_Atlas = atlas;}
	void render()
	{
		if (m_Atlas->shouldRequestNewTextures())
		{
			const std::string pathBegin = "res/textures/";
			m_Atlas->loadTexture(pathBegin + m_Texture, m_Texture);
			return;
		}
		//Checks if texture has changed since last mapped verts
		if (m_LastCycle != m_Atlas->currentTextureCycle())
		{
			Tex::TextureAtlasRGBA::unmapModelVerts(m_Model->getVertices(), m_Model->getVertCount(), m_Texture, m_LastTransform);
			m_LastTransform = m_Atlas->mapModelVerts(m_Model->getVertices(), m_Model->getVertCount(), m_Texture);
			m_LastCycle = m_Atlas->currentTextureCycle();
		}
	}
private:
	std::string m_Texture = "";
	Model::Model<NormalTextureVertex>* m_Model = nullptr;
	Tex::TextureAtlasRGBA* m_Atlas = nullptr;
	Tex::UVTransform m_LastTransform = {0.0f, 1.0f, 1.0f};
	char m_LastCycle = 0;
};

class ModelRender : public RenderComponent
{
public:
	ModelRender() = default;
	ModelRender(Model::Model<NormalTextureVertex>* model) { m_Model = model; }
	void render() { m_Model->render(); };
private:
	Model::Model<NormalTextureVertex>* m_Model = nullptr;
};

#endif