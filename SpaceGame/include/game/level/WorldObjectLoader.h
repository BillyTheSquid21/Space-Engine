#pragma once
#ifndef OBJECT_LOADER_H
#define OBJECT_LOADER_H

#include <fstream>
#include <string>
#include <sstream>
#include <rapidxml/rapidxml.hpp>

#include "core/ObjManagement.h"
#include "game/level/World.h"

#include "game/objects/PlayerObjects.h"
#include "game/objects/NPCAi.h"
#include "game/objects/Script.hpp"
#include "game/objects/TileMap.h"
#include "game/objects/Tree.h"
#include "game/objects/LoadingZone.h"
#include "game/objects/NPCAi.h"

//Root node of all objects is "objects"
namespace WorldParse 
{
	struct XML_Doc_Wrapper
	{
		std::string data;
		std::shared_ptr<rapidxml::xml_document<>> doc;
	};

	XML_Doc_Wrapper ParseLevelXML(World::LevelID id);
	bool ParseLevelObjects(ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc);
	bool ParseLevelTrees(ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc);

	//Sprite loading
	static void LoadSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID);
	static void LoadDirectionalSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID);
	static void LoadWalkingSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID);

	//Sprite help
	inline OvSpr_SpriteData BuildSprDataFromXNode(rapidxml::xml_node<>* node, World::LevelID levelID);
	inline void DirectionalSpriteOptionals(rapidxml::xml_node<>* node, std::shared_ptr<OvSpr_DirectionalSprite> sprite);
	inline void WalkingSpriteOptionals(rapidxml::xml_node<>* node, ObjectManager* manager, std::shared_ptr<OvSpr_WalkingSprite> sprite);

}

namespace World
{
	//Level storage - heads of levels to be used to load in data
	class LevelContainer
	{
	public:
		void InitialiseLevels(ObjectManager* obj, Render::Renderer<TextureVertex>* sprRen, Render::Renderer<TextureVertex>* worRen, TileMap* sprMap, TileMap* worMap);
		void LoadLevel(World::LevelID id);
		void UnloadLevel(World::LevelID id);
		void render();
		std::vector<Level> levels;

	private:
		//Pointer to rendering stuff
		ObjectManager* m_ObjManager;
		Render::Renderer<TextureVertex>* m_SpriteRenderer; Render::Renderer<TextureVertex>* m_WorldRenderer;
		TileMap* m_SpriteTileMap; TileMap* m_WorldTileMap;
	};
}


//Format for xml file:
//<object name = "name here"> -> if name contains "levelx", the object will be unloaded with level
//<Sprite type>
//<!--REQ--> //builds the data structure
//< tileX>3 < / tileX >
//< tileZ>3 < / tileZ >
//< wlevel>0 < / wlevel >
//< tX>0 < / tX >
//< tY>0 < / tY >
//<!--Opt-->
// insert optional here
//< / Sprite type>
//< / object>

//Sprite type tags:
	//Sprite
	//DirSprite
	//WalkSprite
	//RunSprite

//Sprite optional tags:
	//dir
	//randWalk - contains whether is active by default

#endif