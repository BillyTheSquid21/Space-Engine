#pragma once
#ifndef STATE_RENDERERS_HPP
#define STATE_RENDERERS_HPP

#include "renderer/Renderer.hpp"
#include "renderer/Texture.h"
#include "renderer/Model.h"

#include "game/level/World.h"

#include "game/objects/TileMap.h"

#define OV_THREAD_COUNT 3

//Keeps renderer classes for each state close to easily pass around functions

//Overworld renderer
class OverworldRenderer
{
public:

	//Rendering
	Shader shader;
	Camera camera;

	Render::Renderer<NormalTextureVertex> worldRenderer;
	Render::Renderer<NormalTextureVertex> spriteRenderer;
	Render::InstanceRenderer<NormalTextureVertex> modelRenderer;

	//Textures
	Texture worldTexture;
	Texture spriteTexture;

	//Maps
	TileMap worldTileMap = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	TileMap spriteTileMap = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	Tex::TextureAtlasRGBA modelAtlas;

	//Setup renderer one time
	void initialiseRenderer(unsigned int width, unsigned int height);
	void loadRendererData(); //Load data for renderer
	void generateAtlas(); //Generate atlas from loaded model textures
	void purgeData();
	void bindShader(){	shader.bind();	}
	void bufferRenderData();
	void draw(); //Draw scene renderer

	//Slot identifiers
	static constexpr int WORLD_SLOT = 0;
	static constexpr int SPRITE_SLOT = 1;
	static constexpr int ATLAS_SLOT = 2;

	//Player position for lighting
	glm::vec3 m_LightDir = glm::vec3(0.0f, 1.0f, 1.0f);
	int m_LightScene = 0;
};

#endif