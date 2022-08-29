#pragma once
#ifndef STATE_RENDERERS_HPP
#define STATE_RENDERERS_HPP

#include "core/ObjManagement.h"
#include "renderer/Renderer.hpp"
#include "renderer/Texture.h"
#include "game/objects/ModelObject.hpp"
#include "renderer/ShadowMap.h"
#include "renderer/Transition.hpp"
#include "game/objects/ObjectTypes.hpp"
#include "game/level/World.h"
#include "game/level/TextureSlots.hpp"
#include "game/objects/TileMap.h"
#include "mtlib/ThreadPool.h"

#define OVERWORLD_SHADER 0
#define OVERWORLD_SHADOW_SHADER 1
#define GRASS_SHADER 2
#define GRASS_SHADOW_SHADER 3
#define BATTLE_TRANSITION_SHADER 4
#define FADE_OUT_SHADER 5
#define FADE_IN_SHADER 6
#define OVERWORLD_SHADER_COUNT 7

//Keeps renderer classes for each state close to easily pass around functions

//Overworld renderer
class OverworldRenderer
{
public:

	//Rendering
	std::vector<Shader> shaders;
	Camera camera;

	Render::Renderer worldRenderer;
	Render::Renderer grassRenderer;
	Render::Renderer spriteRenderer;
	Render::Renderer pokemonRenderer;
	Render::Renderer modelRenderer;

	//Textures
	Texture worldTexture;
	Texture spriteTexture;
	Texture pokemonTexture;
	Tex::TextureAtlasRGBA modelAtlas;

	//Test transition
	Transition battleTransition;
	Transition fadeOut;
	Transition fadeIn;
	float m_FadeTime = 0.5f;
	bool m_ReadyToShow = false;

	//Maps
	TileMap worldTileMap = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	TileMap spriteTileMap = TileMap(640.0f, 320.0f, 32.0f, 32.0f);
	TileMap pokemonTileMap = TileMap(1.0f, 1.0f, 1.0f, 1.0f);

	//Objects
	ObjectManager* objects = nullptr;

	//Setup renderer one time
	void initialiseRenderer(unsigned int width, unsigned int height, ObjectManager* obj);
	void loadRendererData(); //Load data for renderer
	void generateAtlas(); //Generate atlas from loaded model textures
	void purgeData();
	void bufferRenderData();
	void update(double deltaTime);
	void draw(); //Draw scene renderer
	void mapModelTextures();
	void signalMapModelTextures() { m_RemapModels = true; }

	//Notifies renderer to wait until a level has been modified to change model mapping
	void isModifyingLevel() { m_LevelsLeftLoading++; }
	void hasLevelModified() { m_LevelsLeftLoading--; }

	//Lighting
	glm::vec3 m_LightDir = glm::vec3(0.6f, 0.5f, 0.52f);
	glm::vec3 m_LightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_LightScaled = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_LightScaleFactor = glm::vec3(0.5f, 0.5f, 0.5f);

	unsigned int SCREEN_WIDTH; unsigned int SCREEN_HEIGHT;
	ShadowMap shadowMap = ShadowMap(2048, 2048);
	int lightScene = 0;

	bool m_RemapModels = false;
	int m_LevelsLeftLoading = 0;

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

	Render::Renderer worldRenderer;
	Render::Renderer backgroundRenderer;
	Render::Renderer pokemonARenderer;
	Render::Renderer pokemonBRenderer;

	//Textures
	Texture platformTexture;
	Texture backgroundTexture;
	Texture pokemonATexture;
	Texture pokemonBTexture;
	static const int SPRITE_WIDTH = 96;

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

	unsigned int SCREEN_WIDTH; unsigned int SCREEN_HEIGHT;
};

#endif