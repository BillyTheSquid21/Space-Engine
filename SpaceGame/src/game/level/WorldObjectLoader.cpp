#include "game/level/WorldObjectLoader.h"

static ObjectType GetType(std::string string)
{
	if (string == "DirSprite")
	{
		return ObjectType::DirectionalSprite;
	}
	else if (string == "WalkSprite")
	{
		return ObjectType::WalkingSprite;
	}
	else if (string == "RunSprite")
	{
		return ObjectType::RunningSprite;
	}
	else if (string == "Tree")
	{
		return ObjectType::Trees;
	}
	else if (string == "Grass")
	{
		return ObjectType::Grass;
	}
	else if (string == "Bridge")
	{
		return ObjectType::Bridge;
	}
	else if (string == "Model")
	{
		return ObjectType::Model;
	}
	else if (string == "LoadingZones")
	{
		return ObjectType::LoadingZone;
	}
	else if (string == "WarpTile")
	{
		return ObjectType::WarpTile;
	}
	else if (string == "ScriptTile")
	{
		return ObjectType::ScriptTile;
	}
	else
	{
		return ObjectType::NULL_TYPE;
	}
}

using namespace StateRender;

XML_Doc_Wrapper WorldParse::ParseLevelXML(World::LevelID id, bool global)
{
	//Setup
	using namespace rapidxml;
	std::string path = "";
	if (!global)
	{
		std::string tmp1 = "res/level/level";
		std::string tmp2 = "_obj.xml";
		path = tmp1 + std::to_string((int)id) + tmp2;
	}
	else 
	{
		path = "res/level/global.xml"; //If the global xml, just load this
	}
	
	std::shared_ptr<xml_document<>> doc(new xml_document<>());
	std::ifstream* ifs = new std::ifstream(path);
	std::shared_ptr<std::string> tmp(new std::string);
	std::stringstream buffer;
	buffer << ifs->rdbuf();
	ifs->close();
	delete ifs;

	//Buffer and parse
	*tmp.get() = buffer.str();
	doc->parse<0>(&((*tmp.get())[0]));

	return { tmp, doc };
}

bool WorldParse::ParseLevelObjects(SGObject::ObjectManager* manager, Overworld* ren, World::LevelID levelID, PlayerData* data, GameGUI::TextBoxBuffer* textBuff, std::shared_mutex& mutex, XML_Doc_Wrapper doc, GameInput* input, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld)
{
	//Setup
	using namespace rapidxml;
	std::shared_lock<std::shared_mutex> fileLock(mutex);
	auto ts = EngineTimer::StartTimer();

	xml_node<>* objRoot = doc.doc->first_node()->first_node("Objects");
	if (!objRoot)
	{
		EngineTimer::EndTimer(ts);
		return false;
	}

	int objCount = 0;
	for (xml_node<>* objNode = objRoot->first_node(); objNode; objNode = objNode->next_sibling())
	{
		xml_attribute<>* pAttr = objNode->first_attribute("name");
		std::string name = pAttr->value();
		xml_node<>* identifyingNode = objNode->first_node();
		ObjectType identifier = GetType(identifyingNode->name());

		switch (identifier)
		{
		case ObjectType::RunningSprite:
			WorldParse::LoadRunningSprite(name, identifyingNode, manager, ren, data, textBuff, levelID, input);
			break;
		case ObjectType::DirectionalSprite:
			WorldParse::LoadDirectionalSprite(name, identifyingNode, manager, ren, data, textBuff, levelID, input);
			break;
		case ObjectType::WalkingSprite:
			WorldParse::LoadWalkingSprite(name, identifyingNode, manager, ren, data, textBuff, levelID, input);
			break;
		case ObjectType::Bridge:
			WorldParse::LoadBridge(name, identifyingNode, manager, ren, levelID);
			break;
		case ObjectType::Model:
			WorldParse::LoadModel(name, identifyingNode, manager, ren, levelID);
			break;
		case ObjectType::WarpTile:
			WorldParse::LoadWarpTile(name, identifyingNode, manager, ren, levelID, ld, uld);
			break;
		case ObjectType::ScriptTile:
			WorldParse::LoadScriptTile(name, identifyingNode, manager, levelID, data, textBuff, input);
			break;
		default:
			break;
		}
		objCount++;
	}
	EngineLog("Time taken to load objects: ", EngineTimer::EndTimer(ts));
	return true;
}

bool WorldParse::ParseLevelTrees(SGObject::ObjectManager* manager, Overworld* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc)
{
	//Setup
	using namespace rapidxml;
	std::shared_lock<std::shared_mutex> fileLock(mutex);

	auto ts = EngineTimer::StartTimer();

	xml_node<>* treesRoot = doc.doc->first_node()->first_node("Trees");
	if (!treesRoot)
	{
		EngineTimer::EndTimer(ts);
		return false;
	}

	std::shared_ptr<TreeRenderComponent> treeComp(new TreeRenderComponent(&ren->at(StateRen::OVERWORLD_TREE)));
	std::shared_ptr<TreeObject> tree(new TreeObject());
	tree->setTag((uint16_t)ObjectType::Trees);
	treeComp->reserveTrees(strtoul(treesRoot->first_node("Count")->value(), nullptr, 10));

	Struct2f origin = World::Level::queryOrigin(levelID);
	int treeCount = 0;
	for (xml_node<>* treesNode = treesRoot->first_node("Tree"); treesNode; treesNode = treesNode->next_sibling("Tree"))
	{
		ObjectType identifier = GetType(treesNode->name());
		if (identifier != ObjectType::Trees)
		{
			continue;
		}
		//Add tree from properties
		World::Tile tile = { strtoul(treesNode->first_node("TileX")->value(), nullptr, 10), strtoul(treesNode->first_node("TileZ")->value(), nullptr, 10) };
		World::WorldHeight wLevel = (World::WorldHeight)strtol(treesNode->first_node("WLevel")->value(), nullptr, 10);
		TileUV uv1; TileUV uv2;
		{
			unsigned int tX1 = strtoul(treesNode->first_node("TX1")->value(), nullptr, 10);
			unsigned int tY1 = strtoul(treesNode->first_node("TY1")->value(), nullptr, 10);
			unsigned int tX2 = strtoul(treesNode->first_node("TX2")->value(), nullptr, 10);
			unsigned int tY2 = strtoul(treesNode->first_node("TY2")->value(), nullptr, 10);
			float tWidth = strtof(treesNode->first_node("TW")->value(), nullptr);
			float tHeight = strtof(treesNode->first_node("TH")->value(), nullptr);
			uv1 = ren->tilemap(StateTileMap::OVERWORLD).uvTile(tX1, tY1, tWidth, tHeight);
			uv2 = ren->tilemap(StateTileMap::OVERWORLD).uvTile(tX2, tY2, tWidth, tHeight);
		}
		treeComp->addTree(origin, tile, wLevel, uv1, uv2);
		treeCount++;
	}

	//Add tree comp
	treeComp->generateIndices();
	manager->pushRenderHeap(treeComp, &tree->m_RenderComps);

	//Get tree name
	std::string name;
	if (treesRoot->first_attribute("name"))
	{
		name = treesRoot->first_attribute("name")->value();
	}

	manager->pushGameObject(tree, name);
	EngineLog("Time taken to load trees: ", EngineTimer::EndTimer(ts));
	return true;
}

bool WorldParse::ParseLevelGrass(SGObject::ObjectManager* manager, Overworld* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc)
{
	//Setup
	using namespace rapidxml;
	std::shared_lock<std::shared_mutex> fileLock(mutex);
	auto ts = EngineTimer::StartTimer();

	xml_node<>* tallGrassRoot = doc.doc->first_node()->first_node("TallGrass");
	if (!tallGrassRoot)
	{
		EngineTimer::EndTimer(ts);
		return false;
	}
	std::shared_ptr<TallGrassObject> grass(new TallGrassObject());
	grass->m_LevelID = levelID;
	grass->setTag((uint16_t)ObjectType::Grass);
	std::shared_ptr<TallGrassRenderComponent> grassRen(new TallGrassRenderComponent(&ren->at(StateRen::OVERWORLD_GRASS), &ren->camera, grass.get()));
	grassRen->reserveGrass(strtoul(tallGrassRoot->first_node("Count")->value(), nullptr, 10));

	//Add grass
	Struct2f origin = World::Level::queryOrigin(levelID);
	int grassCount = 0;

	for (xml_node<>* grassNode = tallGrassRoot->first_node("Grass"); grassNode; grassNode = grassNode->next_sibling("Grass"))
	{
		ObjectType identifier = GetType(grassNode->name());
		if (identifier != ObjectType::Grass)
		{
			continue;
		}
		//Add tree from properties
		World::Tile tile = { strtoul(grassNode->first_node("TileX")->value(), nullptr, 10), strtoul(grassNode->first_node("TileZ")->value(), nullptr, 10) };
		World::WorldHeight wLevel = (World::WorldHeight)strtol(grassNode->first_node("WLevel")->value(), nullptr, 10);
		grassRen->addGrass(origin, tile, wLevel, levelID);
		grassCount++;
	}

	//Add grass comp
	manager->pushRenderHeap(grassRen, &grass->m_RenderComps);

	//Get tree name
	std::string name;
	if (tallGrassRoot->first_attribute("name"))
	{
		name = tallGrassRoot->first_attribute("name")->value();
	}

	manager->pushGameObject(grass, name);
	EngineLog("Time taken to load grass: ", EngineTimer::EndTimer(ts));
	return true;
}


bool WorldParse::ParseGlobalObjects(SGObject::ObjectManager* manager, XML_Doc_Wrapper doc, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld)
{
	//Setup
	using namespace rapidxml;
	xml_node<>* objRoot = doc.doc->first_node()->first_node();
	if (!objRoot)
	{
		EngineLog("Error loading global objects");
		return false;
	}
	for (xml_node<>* objNode = objRoot->first_node(); objNode; objNode = objNode->next_sibling())
	{
		xml_attribute<>* pAttr = objNode->first_attribute("name");
		std::string name = pAttr->value();
		ObjectType identifier = GetType(objNode->name());
		switch (identifier)
		{
		case ObjectType::LoadingZone:
			WorldParse::LoadLoadingZone(name, objNode, manager, ld, uld);
			break;
		default:
			break;
		}
	}

	return true;
}

void WorldParse::LoadLoadingZone(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld)
{
	using namespace rapidxml; using namespace Ov_Sprite; using namespace SGObject;
	std::shared_ptr<LoadingZone> zone(new LoadingZone());
	zone->setTag((uint16_t)ObjectType::LoadingZone);
	std::shared_ptr<RenderCompGroup<LoadingZoneComponent>> group(new RenderCompGroup<LoadingZoneComponent>());
	manager->pushRenderGroup(group, "LoadingZones");
	//Load each zone
	std::shared_ptr<RunSprite> playerPtr = manager->objectAt<RunSprite>(manager->queryObjectID("Player"));
	for (xml_node<>* zoneNode = node->first_node(); zoneNode; zoneNode = zoneNode->next_sibling())
	{
		//Get info
		xml_node<>* l1Node = zoneNode->first_node("L1ID");
		World::LevelID l1 = (World::LevelID)strtoul(l1Node->value(), nullptr, 10);
		xml_node<>* l2Node = zoneNode->first_node("L2ID");
		World::LevelID l2 = (World::LevelID)strtoul(l2Node->value(), nullptr, 10);
		LoadingZoneComponent comp(playerPtr.get(), l1, l2);
		//Get bounds
		xml_node<>* xNode = zoneNode->first_node("X");
		xml_node<>* zNode = zoneNode->first_node("Z");
		xml_node<>* widthNode = zoneNode->first_node("Width");
		xml_node<>* heightNode = zoneNode->first_node("Height");
		int x = strtol(xNode->value(), nullptr, 10);
		int z = strtol(zNode->value(), nullptr, 10);
		unsigned int width = strtoul(widthNode->value(), nullptr, 10);
		unsigned int height = strtoul(heightNode->value(), nullptr, 10);
		comp.setZoneBounds(x, z, width, height);
		comp.setLoadingFuncs(ld, uld);

		//Add to group
		group->addExistingComponent(&zone->m_RenderComps, comp);
	}
	manager->pushGameObject(zone, "LoadingZone");
}

void WorldParse::LoadSprite(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, Overworld* ren, World::LevelID levelID, bool* interaction)
{
	using rapidxml::xml_node; using namespace Ov_Sprite;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::shared_ptr<Sprite> sprite = Ov_ObjCreation::BuildSprite(data, ren->tilemap(StateTileMap::OVERWORLD_SPRITE), 
		manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender")).get(), 
		&ren->at(StateRen::OVERWORLD_SPRITE), true); //For now block for loaded sprites
	//sprite->setTag((uint16_t)ObjectType::Sprite);

	//Push object
	manager->pushGameObject(sprite, name);
}

void WorldParse::LoadDirectionalSprite(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, Overworld* ren, PlayerData* pdata, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID, GameInput* input)
{
	using rapidxml::xml_node; using namespace Ov_Sprite;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::shared_ptr<DirectionSprite> sprite = Ov_ObjCreation::BuildDirectionalSprite(data, ren->tilemap(StateTileMap::OVERWORLD_SPRITE), manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender")).get(),
			manager->updateGroupAt<SpriteMap>(manager->queryGroupID("SpriteMap")).get(), manager->updateGroupAt<UpdateAnimationFacing>(manager->queryGroupID("UpdateFacing")).get(), &ren->at(StateRen::OVERWORLD_SPRITE));
	sprite->setTag((uint16_t)ObjectType::DirectionalSprite);

	//Optionals
	WorldParse::DirectionalSpriteOptionals(node, sprite);

	//Push object
	manager->pushGameObject(sprite, name);
}

void WorldParse::LoadWalkingSprite(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, Overworld* ren, PlayerData* pdata, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID, GameInput* input)
{
	using rapidxml::xml_node; using namespace Ov_Sprite;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Permissions - done earlier to get before influence
	World::MovementPermissions* permission = World::GetTilePermission(data.levelID, data.tile, data.height);
	World::MovementPermissions permissionOriginal = *permission;

	//Build base sprite
	std::shared_ptr<WalkSprite> sprite = Ov_ObjCreation::BuildWalkingSprite(data, ren->tilemap(StateTileMap::OVERWORLD_SPRITE), manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender")).get(),
			manager->updateGroupAt<SpriteMap>(manager->queryGroupID("SpriteMap")).get(), manager->updateGroupAt<UpdateAnimationWalking>(manager->queryGroupID("UpdateWalking")).get(), &ren->at(StateRen::OVERWORLD_SPRITE));
	
	sprite->m_LastPermissionPtr = permission; sprite->m_LastPermission = permissionOriginal;
	sprite->setTag((uint16_t)ObjectType::WalkingSprite);

	//Optionals
	WorldParse::WalkingSpriteOptionals(node, manager, sprite);
	WorldParse::OverworldScriptOptionals(node, manager, pdata, textBuff, sprite, input);

	//Push object
	manager->pushGameObject(sprite, name);
}

void WorldParse::LoadRunningSprite(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, Overworld* ren, PlayerData* pdata, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID, GameInput* input)
{
	using rapidxml::xml_node; using namespace Ov_Sprite;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Permissions - done earlier to get before influence
	World::MovementPermissions* permission = World::GetTilePermission(data.levelID, data.tile, data.height);
	World::MovementPermissions permissionOriginal = *permission;

	//Build base sprite
	std::shared_ptr<RunSprite> sprite = Ov_ObjCreation::BuildRunningSprite(data, ren->tilemap(StateTileMap::OVERWORLD_SPRITE), manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender")).get(),
		manager->updateGroupAt<SpriteMap>(manager->queryGroupID("SpriteMap")).get(), manager->updateGroupAt<UpdateAnimationRunning>(manager->queryGroupID("UpdateRunning")).get(), &ren->at(StateRen::OVERWORLD_SPRITE));
	sprite->setTag((uint16_t)ObjectType::RunningSprite);
	sprite->m_LastPermissionPtr = permission; sprite->m_LastPermission = permissionOriginal;

	//Optionals
	WorldParse::WalkingSpriteOptionals(node, manager, sprite);
	WorldParse::OverworldScriptOptionals(node, manager, pdata, textBuff, sprite, input);

	//Push object
	manager->pushGameObject(sprite, name);
}

//TODO - make scripts not heap
void WorldParse::OverworldScriptOptionals(rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, PlayerData* pdata, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<Ov_Sprite::Sprite> sprite, GameInput* input)
{ 
	using namespace Ov_Sprite;
	//Currently working on scripts - TODO - Make work for any sprite and access player properly
	if (node->first_node("NPCScript"))
	{
		std::string filePath = node->first_node("NPCScript")->value();
		std::shared_ptr<RunSprite> player = std::static_pointer_cast<RunSprite>(manager->getObjects()[0].obj);
		std::shared_ptr<NPC_OverworldScript> npcScript = AllocateNPCOvScript(filePath, textBuff, sprite, player);
		manager->pushUpdateHeap(npcScript, &sprite->m_UpdateComps);
	}
}

void WorldParse::DirectionalSpriteOptionals(rapidxml::xml_node<>* node, std::shared_ptr<Ov_Sprite::DirectionSprite> sprite)
{
	//Optionals
	using rapidxml::xml_node;
	xml_node<>* dirNode = node->first_node("Dir");
	if (dirNode)
	{
		unsigned char dir = strtoul(dirNode->value(), nullptr, 10);
		sprite->m_Direction = (World::Direction)dir;
	}
}

void WorldParse::WalkingSpriteOptionals(rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, std::shared_ptr<Ov_Sprite::WalkSprite> sprite)
{
	//Optionals
	using rapidxml::xml_node; using namespace Ov_Sprite; using namespace SGObject;
	//Do optionals for directional which should be available to walking
	WorldParse::DirectionalSpriteOptionals(node, std::static_pointer_cast<DirectionSprite>(sprite));
	xml_node<>* randWNode = node->first_node("RandWalk");
	if (randWNode)
	{
		std::shared_ptr<UpdateCompGroup<NPC_RandWalk>> tmp = manager->updateGroupAt<NPC_RandWalk>(manager->queryGroupID("RandWalk"));
		NPC_RandWalk rWalk(sprite);

		//Check active and add
		bool active = (bool)strtoul(randWNode->value(), nullptr, 10);
		if (!active)
		{
			rWalk.setActive(false);
		}
		tmp->addExistingComponent(&sprite->m_UpdateComps, rWalk);
	}
}

Ov_Sprite::SpriteData WorldParse::BuildSprDataFromXNode(rapidxml::xml_node<>* node, World::LevelID levelID)
{
	using rapidxml::xml_node; using namespace Ov_Sprite;
	SpriteData data;

	xml_node<>* worldLNode = node->first_node("WLevel");
	World::WorldHeight worldLevel = (World::WorldHeight)strtol(worldLNode->value(), nullptr, 10);
	xml_node<>* tXNode = node->first_node("TX");
	xml_node<>* tYNode = node->first_node("TY");
	World::TileTexture tex = { strtoul(tXNode->value(), nullptr, 10), strtoul(tYNode->value(), nullptr, 10) };

	//Position in tiles
	xml_node<>* xTileNode = node->first_node("TileX");
	int xTile = strtol(xTileNode->value(), nullptr, 10);
	xml_node<>* zTileNode = node->first_node("TileZ");
	int zTile = strtol(zTileNode->value(), nullptr, 10);
	
	//Check to make sure node is not occupied, if is, shuffle till a free spot is found
	{
		World::LevelPermission perm = World::RetrievePermission(levelID, { xTile, zTile }, worldLevel);
		int attempts = 0; constexpr int MAX_ATTEMPTS = 30; //Will try to find a clear spot for 30 tries
		while (perm.perm == World::MovementPermissions::SPRITE_BLOCKING && attempts < MAX_ATTEMPTS)
		{
			World::LevelDimensions dim = World::Level::queryDimensions(levelID);
			if (xTile < dim.levelW)
			{
				xTile++;
			}
			else if (zTile < dim.levelH)
			{
				zTile++;
			}
			else if (xTile > dim.levelW)
			{
				xTile--;
			}
			else if (zTile > dim.levelH)
			{
				zTile--;
			}
			perm = World::RetrievePermission(levelID, { xTile, zTile }, worldLevel);
			attempts++;
		}
		//If still fails to find a spot, will just intersect
	}

	data = { {xTile,zTile}, worldLevel, levelID, tex };
	return data;
}

//Loaders
void WorldParse::LoadWarpTile(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, Overworld* ren, World::LevelID levelID, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld)
{
	using rapidxml::xml_node; using namespace Ov_Sprite;
	//Get data
	xml_node<>* tileXCurr = node->first_node("TileXCurr");
	xml_node<>* tileZCurr = node->first_node("TileZCurr");
	World::Tile tileCurrent = { strtoul(tileXCurr->value(), nullptr, 10), strtoul(tileZCurr->value(), nullptr, 10) };
	xml_node<>* tileXDest = node->first_node("TileXDest");
	xml_node<>* tileZDest = node->first_node("TileZDest");
	World::Tile tileDestination = { strtoul(tileXDest->value(), nullptr, 10), strtoul(tileZDest->value(), nullptr, 10) };
	xml_node<>* wLevelCurr = node->first_node("WLevelCurr");
	World::WorldHeight heightCurrent = (World::WorldHeight)strtoul(wLevelCurr->value(), nullptr, 10);
	xml_node<>* wLevelDest = node->first_node("WLevelDest");
	World::WorldHeight heightDestination = (World::WorldHeight)strtoul(wLevelDest->value(), nullptr, 10);
	xml_node<>* idDest = node->first_node("LevelIDDest");
	World::LevelID levelIDDestination = (World::LevelID)strtoul(idDest->value(), nullptr, 10);

	//Create object
	RunSprite* player = (RunSprite*)manager->getObjects()[0].obj.get();
	
	std::shared_ptr<WarpTile> warpTile(new WarpTile());
	warpTile->setTag((uint16_t)ObjectType::WarpTile);
	WarpTileUpdateComponent warpTileUpdate(player, tileCurrent, heightCurrent, levelID, tileDestination, heightDestination, levelIDDestination);
	warpTileUpdate.setLoadingFuncs(ld, uld);
	warpTileUpdate.linkFadeTransition(&ren->transition(StateTrans::OVERWORLD_FADE_OUT));
	manager->updateGroupAt<WarpTileUpdateComponent>(manager->queryGroupID("WarpTile"))->addExistingComponent(&warpTile->m_UpdateComps, warpTileUpdate);
	manager->pushGameObject(warpTile, name);
}

void WorldParse::LoadScriptTile(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, World::LevelID levelID, PlayerData* data, GameGUI::TextBoxBuffer* textBuff, GameInput* input)
{
	using rapidxml::xml_node; using namespace Ov_Sprite; using namespace SGObject;
	//Get data
	xml_node<>* tileX = node->first_node("TileX");
	xml_node<>* tileZ = node->first_node("TileZ");
	World::Tile tile = { strtoul(tileX->value(), nullptr, 10), strtoul(tileZ->value(), nullptr, 10) };
	xml_node<>* wLevel = node->first_node("WLevel");
	World::WorldHeight height = (World::WorldHeight)strtoul(wLevel->value(), nullptr, 10);

	//For now all scripts are on heap - common templates can have their own group in theory later
	std::shared_ptr<ScriptTile> scriptTile(new ScriptTile());
	scriptTile->setTag((uint16_t)ObjectType::ScriptTile);

	//std::lock_guard<std::shared_mutex> objLock(manager->getObjectMutex());
	std::shared_ptr<RunSprite> player = std::static_pointer_cast<RunSprite, GObject>(manager->getObjects()[0].obj);

	//See what type of script and do accordingly
	xml_node<>* scriptType;

	scriptType = node->first_node("Overworld_Script");
	if (scriptType)
	{
		std::shared_ptr<ScriptTileUpdateComponent<OverworldScript>> scriptUpdate(new ScriptTileUpdateComponent<OverworldScript>(player.get(), tile, height, levelID));
		//Create OW Script - add when creation function added
	}
	scriptType = node->first_node("NPC_Script");
	if (scriptType)
	{
		std::shared_ptr<ScriptTileUpdateComponent<NPC_OverworldScript>> scriptUpdate(new ScriptTileUpdateComponent<NPC_OverworldScript>(player.get(), tile, height, levelID));
		xml_node<>* npcNode = node->first_node("NPC_Target");
		std::string npcName = npcNode->value();
		std::shared_ptr<RunSprite> npc = manager->objectAt<RunSprite>(manager->queryObjectID(npcName));
		//get script
		std::string scriptName = scriptType->value();
		NPC_OverworldScript script = CreateNPCOvScript(scriptName, textBuff, npc, player);
		scriptUpdate->setScript(script);
		//upload
		manager->pushUpdateHeap(scriptUpdate, &scriptTile->m_UpdateComps);
		manager->pushGameObject(scriptTile, name);
	}
}

static void LoadModelAsync(std::string name, std::string tex, std::string model, glm::vec3 offset, glm::vec3 scaleFactor, Overworld* ren, SGObject::ObjectManager* manager)
{
	using namespace SGRender; using namespace Geometry;
	std::shared_ptr<ModelObject> modelObj(new ModelObject(tex, model, ren->atlas()));
	modelObj->setRen(&ren->at(StateRen::OVERWORLD_MODEL));
	modelObj->setTag((uint16_t)ObjectType::Model);

	//Scale then translate
	SimpleScale<NTVertex>(modelObj->m_Model.getVertices(), scaleFactor, modelObj->m_Model.getVertCount());
	Translate<NTVertex>(modelObj->m_Model.getVertices(), offset.x, offset.y, offset.z, modelObj->m_Model.getVertCount());

	//Add atlas update and render to groups
	manager->renderGroupAt<ModelRender>(manager->queryGroupID("ModelRender"))->addComponent(&modelObj->m_RenderComps, &modelObj->m_Model);
	manager->pushGameObject(modelObj, name);
}

static void WorldParse::LoadModel(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, Overworld* ren, World::LevelID levelID)
{
	using rapidxml::xml_node;

	//Get data
	xml_node<>* offX = node->first_node("OffsetX");
	xml_node<>* offY = node->first_node("OffsetY");
	xml_node<>* offZ = node->first_node("OffsetZ");
	xml_node<>* scale = node->first_node("Scale");
	xml_node<>* mod = node->first_node("Model");
	xml_node<>* tex = node->first_node("Texture");

	glm::vec3 offset; glm::vec3 scaleFactor;
	{
		Struct2f origin = World::Level::queryOrigin(levelID);
		float x = strtof(offX->value(), nullptr);
		float y = strtof(offY->value(), nullptr);
		float z = strtof(offZ->value(), nullptr);
		offset = glm::vec3(x + origin.a, y, -z + origin.b);
		float scl = strtof(scale->value(), nullptr);
		scaleFactor = glm::vec3(scl, scl, scl);
	}

	std::string texture = tex->value(); std::string model = mod->value();

	//Load on thread pool - TODO Implement and sync together
	LoadModelAsync(name, texture, model, offset, scaleFactor, ren, manager);
}

void WorldParse::LoadBridge(std::string name, rapidxml::xml_node<>* node, SGObject::ObjectManager* manager, Overworld* ren, World::LevelID levelID)
{
	using rapidxml::xml_node;

	//Get data
	xml_node<>* worldLNode = node->first_node("WLevel");
	World::WorldHeight worldLevel = (World::WorldHeight)strtol(worldLNode->value(), nullptr, 10);
	xml_node<>* tX1Node = node->first_node("TX1");
	xml_node<>* tY1Node = node->first_node("TY1");
	World::TileTexture t1 = { strtoul(tX1Node->value(), nullptr, 10), strtoul(tY1Node->value(), nullptr, 10) };
	xml_node<>* tX2Node = node->first_node("TX2");
	xml_node<>* tY2Node = node->first_node("TY2");
	World::TileTexture t2 = { strtoul(tX2Node->value(), nullptr, 10), strtoul(tY2Node->value(), nullptr, 10) };
	TileUV tex1 = ren->tilemap(StateTileMap::OVERWORLD).uvTile(t1.textureX, t1.textureY);
	TileUV tex2 = ren->tilemap(StateTileMap::OVERWORLD).uvTile(t2.textureX, t2.textureY);

	//Get dim
	xml_node<>* wNode = node->first_node("Width");
	xml_node<>* hNode = node->first_node("Height");
	unsigned int width = strtoul(wNode->value(), nullptr, 10);
	unsigned int height = strtoul(hNode->value(), nullptr, 10);

	//Sets horizontal
	bool horizontal = false;
	if (width > height)
	{
		horizontal = true;
	}

	//Get pos
	xml_node<>* tileXNode = node->first_node("TileX");
	xml_node<>* tileZNode = node->first_node("TileZ");
	World::Tile tile = { strtoul(tileXNode->value(), nullptr, 10), strtoul(tileZNode->value(), nullptr, 10) };
	Struct2f offset = World::Level::queryOrigin(levelID);
	offset.a += tile.x * World::TILE_SIZE;
	offset.b -= tile.z * World::TILE_SIZE;

	std::shared_ptr<Bridge> bridge(new Bridge());
	bridge->setTag((uint16_t)ObjectType::Bridge);
	std::shared_ptr<BridgeRenderComponent> bridgeRen(new BridgeRenderComponent(offset, worldLevel, width, height, tex1, tex2, &ren->at(StateRen::OVERWORLD), horizontal));
	bridgeRen->generateIndices();

	manager->pushRenderHeap(bridgeRen, &bridge->m_RenderComps);
	manager->pushGameObject(bridge, name);
}

void World::LevelContainer::InitialiseLevels(SGObject::ObjectManager* obj, Overworld* ren, PlayerData* data, GameGUI::TextBoxBuffer* textBuff, GameInput* input)
{
	m_Input = input; m_ObjManager = obj; m_Renderer = ren; m_Data = data; m_TextBuffer = textBuff;
	m_Levels.resize((int)World::LevelID::LEVEL_NULL);
	std::vector<std::mutex> mutexes((int)World::LevelID::LEVEL_NULL);
	m_LevelMutexes.swap(mutexes);
	for (int i = 0; i < m_Levels.size(); i++)
	{
		m_Levels[i].setID((World::LevelID)i);
	}
}

void World::LevelContainer::BuildFirstLevel(World::LevelID id)
{
	m_Levels[(int)id].buildLevel(&m_Renderer->at(StateRen::OVERWORLD), &m_Renderer->tilemap(StateTileMap::OVERWORLD), &m_Renderer->texture(StateTex::OVERWORLD), m_Renderer->lightColor(), m_Renderer->lightDirection());
}

void World::LevelContainer::InitialiseGlobalObjects()
{
	XML_Doc_Wrapper doc = WorldParse::ParseLevelXML(World::LevelID::LEVEL_NULL, true);
	std::function<void(World::LevelID)> ld = std::bind(&World::LevelContainer::SignalLoadLevel, this, std::placeholders::_1);
	std::function<void(World::LevelID)> uld = std::bind(&World::LevelContainer::SignalUnloadLevel, this, std::placeholders::_1);
	WorldParse::ParseGlobalObjects(m_ObjManager, doc, ld, uld);
	m_LoadingPtr = ld; m_UnloadingPtr = uld;
}

void World::LevelContainer::ChangeLevel()
{
	if (m_ChangeLevel.size() <= 0)
	{
		return;
	}
	for (int i = 0; i < m_ChangeLevel.size(); i++)
	{
		LevelLoad info = m_ChangeLevel.back();

		if (info.state == 1)
		{
			LoadLevel(info.id);
		}
		else if (info.state == 2)
		{
			UnloadLevel(info.id);
		}
		m_ChangeLevel.pop_back();
	}
}

void World::LevelContainer::LoadLevel(World::LevelID id)
{
	//Locks if loading somewhere else, and then checks if has been loaded
	std::lock_guard<std::mutex> levelAccessLock(m_LevelMutexes[(int)id]);
	if (m_Levels[(int)id].loaded())
	{
		m_Renderer->hasLevelModified();
		return;
	}

	using namespace rapidxml;
	if (m_Levels[(int)id].buildLevel(&m_Renderer->at(StateRen::OVERWORLD), &m_Renderer->tilemap(StateTileMap::OVERWORLD_SPRITE), &m_Renderer->texture(StateTex::OVERWORLD), m_Renderer->lightColor(), m_Renderer->lightDirection()))
	{
		using namespace  WorldParse;
		xml_node<>* root;
		xml_node<>* objRoot;
		xml_node<>* treeRoot;
		xml_node<>* grassRoot;
		XML_Doc_Wrapper doc = ParseLevelXML(id, false);

		std::shared_mutex mutex;

		root = doc.doc->first_node();
		if (!root)
		{
			EngineLog("Root node not found. Error on level: ", (int)id);
			m_Renderer->hasLevelModified();
			return;
		}

		//Check for if trees or objects are meant to be present to avoid sending off a redundant thread
		objRoot = root->first_node("Objects");
		treeRoot = root->first_node("Trees");
		grassRoot = root->first_node("TallGrass");

		if (objRoot)
		{
			ParseLevelObjects(m_ObjManager, m_Renderer, id, m_Data, m_TextBuffer, std::ref(mutex), doc, m_Input, m_LoadingPtr, m_UnloadingPtr);
		}
		if (treeRoot)
		{
			ParseLevelTrees(m_ObjManager, m_Renderer, id, std::ref(mutex), doc);
		}
		if (grassRoot)
		{
			ParseLevelGrass(m_ObjManager, m_Renderer, id, std::ref(mutex), doc);
		}

		//Map textures for newly loaded models
		m_Renderer->signalMapModelTextures();
		m_Renderer->hasLevelModified();

		m_Levels[(int)id].setLoaded(true);
	}

	if (!m_Renderer->readyToShow)
	{
		m_Renderer->transition(StateTrans::OVERWORLD_FADE_IN).start();
		m_Renderer->transition(StateTrans::OVERWORLD_FADE_OUT).stop();
		m_Renderer->readyToShow = true;
	}
}

void World::LevelContainer::UnloadLevel(World::LevelID id)
{
	//Locks if loading somewhere else then checks if already unloaded
	std::lock_guard<std::mutex> levelAccessLock(m_LevelMutexes[(int)id]);
	if (!m_Levels[(int)id].loaded())
	{
		m_Renderer->hasLevelModified();
		return;
	}
	
	//Purge all level tagged objects
	std::vector<SGObject::ObjectManager::GameObjectContainer>& objs = m_ObjManager->getObjects();
	for (int i = 0; i < objs.size(); i++)
	{
		if (objs[i].name.find("Level" + std::to_string((int)id)) != std::string::npos)
		{
			m_ObjManager->removeObject(i);
		}
	}

	m_Levels[(int)id].purgeLevel();
	m_Levels[(int)id].setLoaded(false);

	//Map textures for remaining models
	m_Renderer->signalMapModelTextures();
	m_Renderer->hasLevelModified();
}

void World::LevelContainer::UnloadAll()
{
	for (int i = 0; i < m_Levels.size(); i++)
	{
		UnloadLevel((World::LevelID)i);
	}
}

void World::LevelContainer::render()
{
	for (int i = 0; i < m_Levels.size(); i++)
	{
		m_Levels[i].render();
	}
}