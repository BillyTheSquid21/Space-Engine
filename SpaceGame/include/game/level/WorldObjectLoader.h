#pragma once
#ifndef OBJECT_LOADER_H
#define OBJECT_LOADER_H

#include "fstream"
#include "string"
#include "sstream"
#include "thread"
#include "core/ObjManagement.h"
#include "core/Sound.h"
#include "game/level/World.h"
#include "game/level/TextureSlots.hpp"
#include "game/GUI/GUI.h"
#include "game/objects/ObjectTypes.hpp"
#include "game/objects/PlayerObjects.h"
#include "game/objects/NPCAi.h"
#include "game/objects/Script.hpp"
#include "game/objects/ScriptParse.h"
#include "game/objects/ScriptTile.hpp"
#include "game/objects/TileMap.h"
#include "game/objects/Tree.h"
#include "game/objects/TallGrass.h"
#include "game/objects/LoadingZone.h"
#include "game/objects/WarpTile.h"
#include "game/objects/NPCAi.h"
#include "game/objects/Bridge.h"
#include "game/states/StateRenderers.h"
#include "game/utility/Input.hpp"
#include "game/utility/XmlWrapper.hpp"
#include "mtlib/ThreadPool.h"

//Root node of all objects is "objects"
namespace WorldParse 
{
	//Tree process is kept separate from objs as involves mainly reading data and packaging into only a single component
	inline XML_Doc_Wrapper ParseLevelXML(World::LevelID id, bool global);
	bool ParseLevelObjects(SGObject::ObjectManager* manager, StateRender::Overworld* ren, World::LevelID levelID, PlayerData* pdata, GameGUI::TextBoxBuffer* textBuff, std::shared_mutex& mutex, XML_Doc_Wrapper doc, GameInput* input, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld);
	bool ParseLevelTrees(SGObject::ObjectManager* manager, StateRender::Overworld* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc);
	bool ParseLevelGrass(SGObject::ObjectManager* manager, StateRender::Overworld* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc);
	bool ParseGlobalObjects(SGObject::ObjectManager* manager, XML_Doc_Wrapper doc, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld);

	//Sprite loading
	static void LoadSprite(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, StateRender::Overworld* ren, World::LevelID levelID, bool* interaction);
	static void LoadDirectionalSprite(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, StateRender::Overworld* ren, PlayerData* pdata, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID, GameInput* input);
	static void LoadWalkingSprite(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, StateRender::Overworld* ren, PlayerData* pdata, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID, GameInput* input);
	static void LoadRunningSprite(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, StateRender::Overworld* ren, PlayerData* pdata, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID, GameInput* input);

	//Other loading
	static void LoadBridge(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, StateRender::Overworld* ren, World::LevelID levelID);
	static void LoadModel(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, StateRender::Overworld* ren, World::LevelID levelID);
	static void LoadWarpTile(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, StateRender::Overworld* ren, World::LevelID levelID, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld);
	static void LoadScriptTile(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, World::LevelID levelID, PlayerData* data, GameGUI::TextBoxBuffer* textBuff, GameInput* input);
	static void LoadLoadingZone(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld);
	
	//Sprite help
	inline Ov_Sprite::SpriteData BuildSprDataFromXNode(rapidxml::xml_node<>* node, World::LevelID levelID);
	void OverworldScriptOptionals(rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, PlayerData* data, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<Ov_Sprite::Sprite> sprite, GameInput* input);
	void DirectionalSpriteOptionals(rapidxml::xml_node<>* node, std::shared_ptr<Ov_Sprite::DirectionSprite> sprite);
	void WalkingSpriteOptionals(rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, std::shared_ptr<Ov_Sprite::WalkSprite> sprite);
}

namespace World
{
	//Level storage - heads of levels to be used to load in data
	class LevelContainer
	{
	public:
		void InitialiseLevels(SGObject::ObjectManager* obj, StateRender::Overworld* ren, PlayerData* data, GameGUI::TextBoxBuffer* textBuff, GameInput* input, SGSound::System* system);
		void InitialiseGlobalObjects();
		void BuildFirstLevel(World::LevelID id); //Builds geometry and permissions first - for use when no level has been initialised
		
		//Level signals - allows deferring start of level loading
		void SignalLoadLevel(World::LevelID id) { m_Renderer->isModifyingLevel(); m_ChangeLevel.emplace_back(id, 1); };
		void SignalUnloadLevel(World::LevelID id) { m_Renderer->isModifyingLevel(); m_ChangeLevel.emplace_back(id, 2); };
		void ChangeLevel();
		
		void LoadLevel(World::LevelID id);
		void UnloadLevel(World::LevelID id);
		void UnloadAll();
		void render();
		std::vector<Level> m_Levels;
		std::vector<std::mutex> m_LevelMutexes; //Access mutex array to make sure the same level can't load at the same time
	private:
		//Load level		
		struct LevelLoad
		{
			World::LevelID id;
			char state = 0;
		};
		std::vector<LevelLoad> m_ChangeLevel;

		//Pointer to rendering stuff
		SGObject::ObjectManager* m_ObjManager;
		PlayerData* m_Data;
		GameGUI::TextBoxBuffer* m_TextBuffer;
		StateRender::Overworld* m_Renderer;
		GameInput* m_Input;
		std::function<void(World::LevelID)> m_LoadingPtr;
		std::function<void(World::LevelID)> m_UnloadingPtr;

		//Sound
		SGSound::System* m_System;
		FMOD::Channel* m_Music[2] = { nullptr, nullptr }; //Allows music to be faded and swapped without creating too many channels
		SGSound::sound_id m_IDS[2] = {};
		char m_CurrentSlot = 0; 
	};
}


//Format for xml file:
// 
// <Trees name = "LevelX_Trees">
// <Count>count</Count>
// <Tree>
//		<TileX>x< / TileX >
//		< TileZ>z< / TileZ >
//		< WLevel>l< / WLevel >
//		< TX1>tx1 < / TX1 > //Location of bottom of texture
//		< TY1>ty1 < / TY1 >
//		< TX2>tx2 < / TX2 >
//		< TY2>ty2 < / TY2 >
//		< TW>width < / TW >
//		< TH>height < / TH >
//	< / Tree>
// 
// 	<TallGrass name = "Level0_Grass">
//	<Count>10 < / Count >
//	< TXFrame1>0 < / TXFrame1 >	//Textures for each of the 3 frames
//	< TYFrame1>0 < / TYFrame1 >
//	< TXFrame2>0 < / TXFrame2 >
//	< TYFrame2>0 < / TYFrame2 >
//	< TXFrame3>0 < / TXFrame3 >
//	< TYFrame3>0 < / TYFrame3 >
//	<Grass>
//		<TileX>< / TileX>
//		<TileZ>< / TileZ>
//		<WLevel></WLevel>
//	< / Grass>
//	< / TallGrass>
// 
//<Object name = "name here"> -> if name contains "levelx", the object will be unloaded with level
//<Sprite type>
//<!--REQ--> //builds the data structure
//< TileX>3 < / TileX >
//< TileZ>3 < / TileZ >
//< WLevel>0 < / WLevel >
//< TX>0 < / TX >
//< TY>0 < / TY >
//<!--Opt-->
// insert optional here
//< / Sprite type>
//< / Object>

//Sprite type tags:
	//Sprite
	//DirSprite
	//WalkSprite
	//RunSprite

//Sprite optional tags:
	//Dir
	//RandWalk - contains whether is active by default
	//NPCScript - path to script to run


//Other objects
	//Bridge - has dimensions and texture
	//
	// 		<Object name="Level0_ScriptTile">
    //			<ScriptTile>
    //				< TileX>3 < / TileX >
    //				< TileZ>3 < / TileZ >
    //				< WLevel>0 < / WLevel >
    //				< NPC_Script>"test.sgs" < / NPC_Script >
	//				< NPC_Target>"npc" < / NPC_Target >
	//				< Overworld_Script>"test.sgs" < / Overworld_Script >
    //			< / ScriptTile>
    //		< / Object>
	// 
	// 
	//	<WarpTile>
    //		<TileXCurr>10 < / TileXCurr >
    //		< TileZCurr>2 < / TileZCurr >
    //		< TileXDest>0 < / TileXDest >
    //		< TileZDest>0 < / TileZDest >
    //		< WLevelCurr>0 < / WLevelCurr >
    //		< WLevelDest>0 < / WLevelDest >
    //		< LevelIDDest>0 < / LevelIDDest >
    //	< / WarpTile>
	// 
	//	<Model>
	//		< OffsetX>0 < / OffsetX >
	//		< OffsetY>0 < / OffsetY >
	//		< OffsetZ>0 < / OffsetZ >
	//		< Scale>1.0 < / Scale >
	//		<Model>untitled.obj< / Model>
	//		<Texture>debug.png< / Texture>
	//	< / Model>

#endif