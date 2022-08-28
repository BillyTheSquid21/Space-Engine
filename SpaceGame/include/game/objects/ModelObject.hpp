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
	void setRen(Render::Renderer* ren) { m_Model.setRen(ren); }

	std::string m_Texture = "";
	Model::Model<NormalTextureVertex> m_Model;
	Tex::UVTransform m_LastTransform = { 0.0f, 1.0f, 1.0f };
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