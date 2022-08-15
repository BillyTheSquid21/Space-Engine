#pragma once
#ifndef STATE_RENDERERS_HPP
#define STATE_RENDERERS_HPP

#include "renderer/Renderer.hpp"
#include "renderer/Texture.h"
#include "game/objects/ModelObject.hpp"
#include "renderer/ShadowMap.h"
#include "renderer/Transition.hpp"

#include "game/level/World.h"

#include "game/objects/TileMap.h"
#include "mtlib/ThreadPool.h"

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
	Shader transitionShader;
	Camera camera;

	Render::Renderer<NormalTextureVertex> worldRenderer;
	Render::Renderer<NormalTextureVertex> grassRenderer;
	Render::Renderer<NormalTextureVertex> spriteRenderer;
	Render::Renderer<NormalTextureVertex> pokemonRenderer;
	Render::Renderer<NormalTextureVertex> modelRenderer;

	//Textures
	Texture worldTexture;
	Texture spriteTexture;
	Texture pokemonTexture;

	//Test transition
	Transition battleTransition;

	//Maps
	TileMap worldTileMap = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	TileMap spriteTileMap = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	TileMap pokemonTileMap = TileMap(1.0f, 1.0f, 1.0f, 1.0f);
	Tex::TextureAtlasRGBA modelAtlas;

	//Setup renderer one time
	void initialiseRenderer(unsigned int width, unsigned int height);
	void loadRendererData(); //Load data for renderer
	void generateAtlas(); //Generate atlas from loaded model textures
	void purgeData();
	void bufferRenderData();
	void update(double deltaTime);
	void draw(); //Draw scene renderer
	void ensureModelMapping(unsigned int objectCount);

	//Slot identifiers
	static constexpr int TEXTURE_SLOT = 0;
	static constexpr int SHADOWS_SLOT = 1;

	//Lighting
	glm::vec3 m_LightDir = glm::vec3(0.6f, 0.5f, 0.52f);
	glm::vec3 m_LightColor = glm::vec3(0.5f, 0.5f, 0.5f);
	glm::vec3 m_LightScaled = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_LightScaleFactor = glm::vec3(1.0f, 1.0f, 1.0f);

	unsigned int SCREEN_WIDTH; unsigned int SCREEN_HEIGHT;
	ShadowMap shadowMap = ShadowMap(2048, 2048);
	int lightScene = 0;

	unsigned int lastObjectCount = 0;

	//Test
	std::function<void()> m_StateToBattle;
};

//battle
class BattleRenderer
{
public:

	//Rendering
	Shader sceneShader;
	Camera camera;

	Render::Renderer<TextureVertex> worldRenderer;
	Render::Renderer<TextureVertex> backgroundRenderer;
	Render::Renderer<TextureVertex> pokemonARenderer;
	Render::Renderer<TextureVertex> pokemonBRenderer;

	//Textures
	Texture platformTexture;
	Texture backgroundTexture;
	static const int SPRITE_WIDTH = 96;
	Texture pokemonATexture;
	Texture pokemonBTexture;

	//Setup renderer one time
	void initialiseRenderer(unsigned int width, unsigned int height);
	void loadRendererData(); //Load data for renderer
	void purgeData();
	void bufferRenderData();
	void draw(); //Draw scene renderer

	//Load texture for pokemon
	void loadPokemonTextureA(std::string name);
	void loadPokemonTextureB(std::string name);
	const std::string FRONT_TEX_PATH = "res/textures/pokemon/front/";
	const std::string BACK_TEX_PATH = "res/textures/pokemon/back/";
	const std::string PNG_EXT = ".png";

	//Slot identifiers
	static constexpr int TEXTURE_SLOT = 0;

	unsigned int SCREEN_WIDTH; unsigned int SCREEN_HEIGHT;
};

#endif