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
#include "utility/SegArray.hpp"
#include "utility/Random.h"
#include "functional"
#include "atomic"
#include "mtlib/ThreadPool.h"

//In this file the renderers for each state with anything more than trivial is stored.
//I used SegArrays to store components as this ensures the address will not change while the state is running
//For dynamic resources, ensure you use push and pop if address is referenced

//Stores index of specific reserved shaders in array
enum class StateShader
{
	//Overworld
	OVERWORLD = 0, OVERWORLD_SHADOW = 1, OVERWORLD_GRASS = 2,
	OVERWORLD_GRASS_SHADOW = 3, OVERWORLD_BATTLE = 4, OVERWORLD_FADE_OUT = 5,
	OVERWORLD_FADE_IN = 6, OVERWORLD_TREE = 7, OVERWORLD_TREE_SHADOW = 8, OVERWORLD_COUNT = 9,

	//Battle
	BATTLE = 0, BATTLE_COUNT = 1
};

//Stores index of specific reserved renderers in array
enum class StateRen
{
	//Overworld
	OVERWORLD = 0, OVERWORLD_GRASS = 1, OVERWORLD_SPRITE = 2, 
	OVERWORLD_POKEMON = 3, OVERWORLD_MODEL = 4, OVERWORLD_TREE = 5, OVERWORLD_COUNT = 6,

	//Battle
	BATTLE_PLATFORM = 0, BATTLE_BACKGROUND = 1, BATTLE_POKEMONA = 2, BATTLE_POKEMONB = 3,
	BATTLE_COUNT = 4
};

//Stores index of specific reserved transitions in array
enum class StateTrans
{
	OVERWORLD_BATTLE = 0, OVERWORLD_FADE_OUT = 1, OVERWORLD_FADE_IN = 2,
	OVERWORLD_COUNT = 3
};

//Stores index of specific reserved textures in array
enum class StateTex
{
	//Overworld
	OVERWORLD = 0, OVERWORLD_SPRITE = 1, OVERWORLD_POKEMON = 2, OVERWORLD_WIND_A = 3, OVERWORLD_WIND_B = 4,
	OVERWORLD_COUNT = 5,
	
	//Battle
	BATTLE_PLATFORM = 0, BATTLE_BACKGROUND = 1, BATTLE_POKEMONA = 2, BATTLE_POKEMONB = 3,
	BATTLE_COUNT = 4
};

//Stores index of specific reserved tilemaps in array
enum class StateTileMap
{
	OVERWORLD = 0, OVERWORLD_SPRITE = 1, OVERWORLD_POKEMON = 2, OVERWORLD_COUNT = 3
};

//TODO - add other slots

//Overworld renderer
class OverworldRenderer
{
public:
	Camera camera;

	//Test transition
	bool readyToShow = false;

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

	//Get renderer
	Render::Renderer& operator[](StateRen slot) { return renderers[(int)slot]; }
	Render::Renderer& at(StateRen slot) { return renderers[(int)slot]; }

	//Get texture
	Texture& texture(StateTex slot) { return textures[(int)slot]; }
	Tex::TextureAtlasRGBA& atlas() { return textureAtlas; }

	//Get shader
	Shader& shader(StateShader slot) { return shaders[(int)slot]; }

	//Get transition
	Transition& transition(StateTrans slot) { return transitions[(int)slot]; }

	//Get tilemap
	TileMap& tilemap(StateTileMap slot) { return tileMaps[(int)slot]; }

	//Lighting
	glm::vec3 m_LightDir = glm::vec3(0.6f, 0.5f, 0.52f);
	glm::vec3 m_LightColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_LightScaled = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 m_LightScaleFactor = glm::vec3(0.5f, 0.5f, 0.5f);

	int lightScene = 0;

	//Start battle function
	std::function<void()> m_StateToBattle;

	//Perlin wind
	//Have two buffers to interpolate between
	//-1->0->1 is 0 -> 128 -> 256 as vectors
	SGRandom::Perlin2D<uint8_t, 3, 13, 512> m_PerlinGenerator;
	std::vector<unsigned char> m_BufferA;
	std::vector<unsigned char> m_BufferB;
	std::function<bool(int, int, float)> m_ScrollNoise;
	bool m_CurrBuffer = 0; //0 is A, 1 is B
	std::atomic_bool m_NoiseReady = false;
	double m_WindTimer = 0.0;
	MtLib::ThreadPool* m_Pool;
	double m_WindSampleInterval = 5.0;
	float m_WindWeightA = 0.0f;

private:
	//Objects
	ObjectManager* objects = nullptr;

	//Rendering
	SegArray<Shader, (int)StateShader::OVERWORLD_COUNT> shaders;
	SegArray<Render::Renderer, (int)StateRen::OVERWORLD_COUNT> renderers;
	SegArray<Texture, (int)StateTex::OVERWORLD_COUNT> textures;

	//Textures
	Tex::TextureAtlasRGBA textureAtlas;

	//Transitions
	SegArray<Transition, (int)StateTrans::OVERWORLD_COUNT> transitions;
	float fadeTime = 0.5f;

	//Tilemaps
	SegArray<TileMap, (int)StateTileMap::OVERWORLD_COUNT> tileMaps;

	//Shadow map
	ShadowMap shadowMap = ShadowMap(2048, 2048);

	unsigned int SCREEN_WIDTH; unsigned int SCREEN_HEIGHT;

	bool m_RemapModels = false;
	int m_LevelsLeftLoading = 0;
	double m_Time = 0.0f; //Time variable - just keeps going up indefinately
};

//battle
class BattleRenderer
{
public:

	//Rendering
	Shader sceneShader;
	Camera camera;

	//Get renderer
	Render::Renderer& operator[](StateRen slot) { return renderers[(int)slot]; }
	Render::Renderer& at(StateRen slot) { return renderers[(int)slot]; }
	Render::Renderer& operator[](int slot) { return renderers[slot]; }
	Render::Renderer& at(int slot) { return renderers[slot]; }

	//Get texture
	Texture& texture(StateTex slot) { return textures[(int)slot]; }
	Texture& texture(int slot) { return textures[slot]; }

	//Get shader
	Shader& shader(StateShader slot) { return shaders[(int)slot]; }

	SegArray<Render::Renderer, (int)StateRen::BATTLE_COUNT> renderers;
	SegArray<Texture, (int)StateTex::BATTLE_COUNT> textures;
	SegArray<Shader, (int)StateShader::BATTLE_COUNT> shaders;

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