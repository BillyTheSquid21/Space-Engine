#pragma once
#ifndef OBJECT_LOADER_H
#define OBJECT_LOADER_H

#include <fstream>
#include <string>
#include <sstream>
#include <rapidxml/rapidxml.hpp>
#include <thread>

#include "core/ObjManagement.h"
#include "game/level/World.h"

#include "game/GUI/GUI.h"

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

#include "mtlib/ThreadPool.h"

//Root node of all objects is "objects"
namespace WorldParse 
{
	struct XML_Doc_Wrapper
	{
		std::shared_ptr<std::string> data;
		std::shared_ptr<rapidxml::xml_document<>> doc;
	};

	//Tree process is kept separate from objs as involves mainly reading data and packaging into only a single component
	inline XML_Doc_Wrapper ParseLevelXML(World::LevelID id, bool global);
	bool ParseLevelObjects(ObjectManager* manager, OverworldRenderer* ren, World::LevelID levelID, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, std::shared_mutex& mutex, XML_Doc_Wrapper doc, GameInput* input, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld);
	bool ParseLevelTrees(ObjectManager* manager, OverworldRenderer* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc);
	bool ParseLevelGrass(ObjectManager* manager, OverworldRenderer* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc);
	bool ParseGlobalObjects(ObjectManager* manager, XML_Doc_Wrapper doc, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld);

	//Sprite loading
	static void LoadSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, OverworldRenderer* ren, World::LevelID levelID, bool* interaction);
	static void LoadDirectionalSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, OverworldRenderer* ren, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID, GameInput* input);
	static void LoadWalkingSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, OverworldRenderer* ren, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID, GameInput* input);

	//Other loading
	static void LoadBridge(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, OverworldRenderer* ren, World::LevelID levelID);
	static void LoadModel(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, OverworldRenderer* ren, World::LevelID levelID);
	static void LoadWarpTile(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, World::LevelID levelID, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld);
	static void LoadScriptTile(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, World::LevelID levelID, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, GameInput* input);
	static void LoadLoadingZone(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld);
	
	//Sprite help
	inline OvSpr_SpriteData BuildSprDataFromXNode(rapidxml::xml_node<>* node, World::LevelID levelID);
	void OverworldScriptOptionals(rapidxml::xml_node<>* node, ObjectManager* manager, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<OvSpr_Sprite> sprite, GameInput* input);
	void DirectionalSpriteOptionals(rapidxml::xml_node<>* node, std::shared_ptr<OvSpr_DirectionalSprite> sprite);
	void WalkingSpriteOptionals(rapidxml::xml_node<>* node, ObjectManager* manager, std::shared_ptr<OvSpr_WalkingSprite> sprite);
}

namespace World
{
	//Level storage - heads of levels to be used to load in data
	class LevelContainer
	{
	public:
		void InitialiseLevels(ObjectManager* obj, OverworldRenderer* ren, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, GameInput* input);
		void InitialiseGlobalObjects();
		void BuildFirstLevel(World::LevelID id); //Builds geometry and permissions first - for use when no level has been initialised
		void LoadLevel(World::LevelID id);
		void UnloadLevel(World::LevelID id);
		void UnloadAll();
		void render();
		std::vector<Level> m_Levels;
		std::vector<std::mutex> m_LevelMutexes; //Access mutex array to make sure the same level can't load at the same time
	private:
		//Pointer to rendering stuff
		ObjectManager* m_ObjManager;
		FlagArray* m_Flags;
		GameGUI::TextBoxBuffer* m_TextBuffer;
		OverworldRenderer* m_Renderer;
		GameInput* m_Input;
		std::function<void(World::LevelID)> m_LoadingPtr;
		std::function<void(World::LevelID)> m_UnloadingPtr;
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