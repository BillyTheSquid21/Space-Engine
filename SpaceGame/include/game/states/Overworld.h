#pragma once
#ifndef OVERWORLD_H
#define OVERWORLD_H

#include "GL/glew.h"
#include "core/State.hpp"
#include "core/ObjManagement.h"
#include "core/Sound.h"
#include "core/Message.hpp"
#include "game/level/World.h"
#include "game/level/WorldObjectLoader.h"
#include "game/objects/Bridge.h"
#include "game/objects/ObjectTypes.hpp"
#include "game/gui/GUI.h"
#include "game/gui/OverworldGUI.h"
#include "game/gui/OptionsGUI.h"
#include "game/utility/GameText.h"
#include "game/states/StateRenderers.h"
#include "game/utility/Input.hpp"
#include "game/objects/TallGrass.h"
#include "game/objects/WarpTile.h"
#include "game/objects/ScriptTile.hpp"
#include "game/objects/PokemonFollow.h"
#include "game/data/PlayerData.hpp"
#include "game/pokemon/Pokemon.h"
#include "game/pokemon/PokemonIO.h"
#include "game/pokemon/PokemonLevel.h"
#include "mtlib/ThreadPool.h"
#include "functional"

class Overworld : public SG::State
{
public:
	/**
	* Initialise the Overworld state
	* @param width width of the screen
	* @param height height of the screen
	* @param data pointer to the player data
	* @param levelEntry first level to be loaded
	* @param fonts pointer to the font container
	* @param flags pointer to the flag array
	* @param input pointer to the game input
	*/
	void init(
		int width, 
		int height, 
		PlayerData* data, 
		World::LevelID levelEntry, 
		FontContainer* fonts, 
		FlagArray* flags, 
		GameInput* input,
		SGSound::System* system
	);

	/**
	* Link to the start battle function to be able to switch state
	*/
	void setBattleFunction(std::function<void(Party*, Party*)> battle);

	void update(double deltaTime, double time);
	void render();
	void loadRequiredData();
	void purgeRequiredData();
	void handleInput(int key, int scancode, int action, int mods);

private:
	int m_Width; int m_Height;

	void loadObjectData();
	void createGroups();
	void handleOverworldMusic();
	void clearMusicChannels();

	//Group ptrs
	SGObject::UpdateCompGroup<Ov_Sprite::TilePosition>* m_TileGroup;
	SGObject::UpdateCompGroup<Ov_Sprite::SpriteMap>* m_MapGroup;
	SGObject::UpdateCompGroup<SpriteAnim<SGRender::NTVertex, Norm_Tex_Quad>>* m_AnimGroup;
	SGObject::UpdateCompGroup<Ov_Sprite::UpdateAnimationRunning>* m_RunGroup;
	SGObject::UpdateCompGroup<Ov_Sprite::UpdateAnimationFacing>* m_FaceGroup;
	SGObject::UpdateCompGroup<Ov_Sprite::UpdateAnimationWalking>* m_WalkGroup;
	SGObject::UpdateCompGroup<NPC_RandWalk>* m_RandWGroup;
	SGObject::UpdateCompGroup<WarpTileUpdateComponent>* m_WarpGroup;
	SGObject::RenderCompGroup<ModelRender>* m_ModRenGroup;
	SGObject::RenderCompGroup<Ov_Sprite::SpriteRender>* m_SpriteGroup;

	//State renderer
	StateRender::Overworld m_Renderer;

	//Current level - defualts to the entry level
	World::LevelID m_CurrentLevel = World::LevelID::LEVEL_ENTRY;

	//Test level
	World::LevelContainer m_Levels;
	std::shared_ptr<Ov_Sprite::RunSprite> m_PlayerPtr;
	World::LevelID m_LastLevelID = World::LevelID::LEVEL_NULL;

	//Test obj mng
	SGObject::ObjectManager m_ObjManager;

	//Test font
	FontContainer* m_Fonts;

	//Test flags
	FlagArray* m_Flags;

	//Test sound
	SGSound::System* m_System = nullptr;
	FMOD::Channel* m_MusicChannels[2]{ NULL, NULL };
	SGSound::sound_id m_MusicIDS[2]{ };
	char m_Channel = 0;

	//Data
	PlayerData* m_Data;
	bool m_StartBattle = false;

	//Test battle
	Party enemy;

	//Test pointer
	bool m_AnimatePointer = true;

	//Test GUI
	GameGUI::GUIContainer m_HUD;
	std::string m_RenderTime = "";
	std::string m_UpdateTime = "";
	std::string m_CurrentLevelStr = "";
	std::string m_CurrentTileStr = ""; 
	std::string m_ObjectCountStr = "";
	std::string m_Song = "";
	int* m_LocationX = nullptr;
	int* m_LocationZ = nullptr;
	GameGUI::TextBoxBuffer m_TextBuff;
	float m_SampleTime = 0.0f;
	bool m_Sample = false;
	bool m_ShowDebug = false;
	bool m_ShowMenu = false;
	bool m_ShowOptions = false;
	bool m_Noclip = false;

	//Test battle
	void startBattle();
	std::function<void(Party*, Party*)> m_BattleEnable;

	GameInput* m_Input;

	MtLib::ThreadPool* m_Pool = nullptr;

};


#endif