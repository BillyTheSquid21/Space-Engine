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
#include "game/objects/TileMap.h"
#include "game/objects/Tree.h"
#include "game/objects/LoadingZone.h"
#include "game/objects/NPCAi.h"

//Root node of all objects is "objects"
namespace WorldParse 
{
	struct XML_Doc_Wrapper
	{
		std::shared_ptr<std::string> data;
		std::shared_ptr<rapidxml::xml_document<>> doc;
	};

	//Tree process is kept separate from objs as involves mainly reading data and packaging into only a single component
	inline XML_Doc_Wrapper ParseLevelXML(World::LevelID id);
	bool ParseLevelObjects(ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, std::shared_mutex& mutex, XML_Doc_Wrapper doc);
	bool ParseLevelTrees(ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc);

	//Sprite loading
	static void LoadSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID);
	static void LoadDirectionalSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID);
	static void LoadWalkingSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID);

	//Sprite help
	inline OvSpr_SpriteData BuildSprDataFromXNode(rapidxml::xml_node<>* node, World::LevelID levelID);
	inline void OverworldScriptOptionals(rapidxml::xml_node<>* node, ObjectManager* manager, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<OvSpr_Sprite> sprite);
	inline void DirectionalSpriteOptionals(rapidxml::xml_node<>* node, std::shared_ptr<OvSpr_DirectionalSprite> sprite);
	inline void WalkingSpriteOptionals(rapidxml::xml_node<>* node, ObjectManager* manager, std::shared_ptr<OvSpr_WalkingSprite> sprite);
}

namespace World
{
	//Level storage - heads of levels to be used to load in data
	class LevelContainer
	{
	public:
		void InitialiseLevels(ObjectManager* obj, Render::Renderer<TextureVertex>* sprRen, Render::Renderer<TextureVertex>* worRen, TileMap* sprMap, TileMap* worMap, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff);
		void BuildFirstLevel(World::LevelID id); //Builds geometry and permissions first - for use when no level has been initialised
		void LoadLevel(World::LevelID id);
		void UnloadLevel(World::LevelID id);
		void render();
		std::vector<Level> m_Levels;
		std::vector<std::mutex> m_LevelMutexes; //Access mutex array to make sure the same level can't load at the same time
	private:
		//Pointer to rendering stuff
		ObjectManager* m_ObjManager;
		FlagArray* m_Flags;
		GameGUI::TextBoxBuffer* m_TextBuffer;
		Render::Renderer<TextureVertex>* m_SpriteRenderer; Render::Renderer<TextureVertex>* m_WorldRenderer;
		TileMap* m_SpriteTileMap; TileMap* m_WorldTileMap;
	};
}


//Format for xml file:
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

#endif