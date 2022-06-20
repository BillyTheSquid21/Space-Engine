#pragma once
#ifndef STATE_RENDERERS_HPP
#define STATE_RENDERERS_HPP

#include "renderer/Renderer.hpp"
#include "renderer/Texture.h"
#include "renderer/Model.hpp"
#include "renderer/ShadowMap.h"

#include "game/level/World.h"

#include "game/objects/TileMap.h"

//Keeps renderer classes for each state close to easily pass around functions

//Overworld renderer
class OverworldRenderer
{
public:

	//Rendering
	Shader sceneShader;
	Shader sceneShadows;
	Shader grassShader;
	Shader grassShadows;
	Camera camera;

	Render::Renderer<NormalTextureVertex> worldRenderer;
	Render::Renderer<NormalTextureVertex> grassRenderer;
	Render::Renderer<NormalTextureVertex> spriteRenderer;
	Render::Renderer<NormalTextureVertex> modelRenderer;

	//Textures
	Texture worldTexture;
	Texture spriteTexture;
	Texture debugTexture;

	//Maps
	TileMap worldTileMap = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	TileMap spriteTileMap = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	Tex::TextureAtlasRGBA modelAtlas;

	//Setup renderer one time
	void initialiseRenderer(unsigned int width, unsigned int height);
	void loadRendererData(); //Load data for renderer
	void generateAtlas(); //Generate atlas from loaded model textures
	void purgeData();
	void bufferRenderData();
	void draw(); //Draw scene renderer

	//Slot identifiers
	static constexpr int TEXTURE_SLOT = 0;
	static constexpr int SHADOWS_SLOT = 1;

	//Lighting
	glm::vec3 m_LightDir = glm::vec3(0.6f, 0.5f, 0.52f);
	glm::vec3 m_LightColor = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 m_LightScaled = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_LightScaleFactor = glm::vec3(1.0f, 1.0f, 1.0f);

	unsigned int SCREEN_WIDTH; unsigned int SCREEN_HEIGHT;
	ShadowMap shadowMap = ShadowMap(2048,2048);
	int lightScene = 1;

	//model test
	Model::Model<NormalTextureVertex> model = Model::Model<NormalTextureVertex>("res/model/untitled.obj");
};

#endif