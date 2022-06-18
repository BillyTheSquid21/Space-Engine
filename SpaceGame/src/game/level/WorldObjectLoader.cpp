#include "game/level/WorldObjectLoader.h"

static enum class ObjectType
{
	Trees, DirectionalSprite, WalkingSprite, Bridge, LoadingZone, NULL_TYPE
};

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
	else if (string == "Tree")
	{
		return ObjectType::Trees;
	}
	else if (string == "Bridge")
	{
		return ObjectType::Bridge;
	}
	else if (string == "LoadingZones")
	{
		return ObjectType::LoadingZone;
	}
	else
	{
		return ObjectType::NULL_TYPE;
	}
}

WorldParse::XML_Doc_Wrapper WorldParse::ParseLevelXML(World::LevelID id, bool global)
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

bool WorldParse::ParseLevelObjects(ObjectManager* manager, OverworldRenderer* ren, World::LevelID levelID, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, std::shared_mutex& mutex, XML_Doc_Wrapper doc, GameInput* input)
{
	//Setup
	using namespace rapidxml;
	std::shared_lock<std::shared_mutex> fileLock(mutex);
	auto ts = EngineTimer::StartTimer();

	xml_node<>* objRoot = doc.doc->first_node()->first_node("Objects");
	if (!objRoot)
	{
		EngineLog("Error loading objects: ", (int)levelID);
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
		case ObjectType::DirectionalSprite:
			WorldParse::LoadDirectionalSprite(name, identifyingNode, manager, ren, flags, textBuff, levelID, input);
			break;
		case ObjectType::WalkingSprite:
			WorldParse::LoadWalkingSprite(name, identifyingNode, manager, ren, flags, textBuff, levelID, input);
			break;
		case ObjectType::Bridge:
			WorldParse::LoadBridge(name, identifyingNode, manager, ren, levelID);
			break;
		default:
			break;
		}
		objCount++;
	}
	EngineLog("Time taken to load objects: ", EngineTimer::EndTimer(ts));
	return true;
}

bool WorldParse::ParseLevelTrees(ObjectManager* manager, OverworldRenderer* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc)
{
	//Setup
	using namespace rapidxml;
	std::shared_lock<std::shared_mutex> fileLock(mutex);
	auto ts = EngineTimer::StartTimer();

	xml_node<>* treesRoot = doc.doc->first_node()->first_node("Trees");
	if (!treesRoot)
	{
		EngineLog("Error loading trees: ", (int)levelID);
		return false;
	}

	std::shared_ptr<TreeRenderComponent> treeComp(new TreeRenderComponent(&ren->worldRenderer));
	std::shared_ptr<TreeObject> tree(new TreeObject());
	treeComp->reserveTrees(strtoul(treesRoot->first_node("Count")->value(), nullptr, 10));

	Struct2f origin = World::Level::queryOrigin(levelID);
	int treeCount = 0;
	for (xml_node<>* treesNode = treesRoot->first_node("Tree"); treesNode; treesNode = treesNode->next_sibling("Tree"))
	{
		xml_attribute<>* pAttr = treesRoot->first_attribute("name");
		std::string name = pAttr->value();
		ObjectType identifier = GetType(treesNode->name());
		if (identifier != ObjectType::Trees)
		{
			continue;
		}
		//Add tree from properties
		World::TileLoc tile = { strtoul(treesNode->first_node("TileX")->value(), nullptr, 10), strtoul(treesNode->first_node("TileZ")->value(), nullptr, 10) };
		World::WorldHeight wLevel = (World::WorldHeight)strtol(treesNode->first_node("WLevel")->value(), nullptr, 10);
		TileUV uv1; TileUV uv2;
		{
			unsigned int tX1 = strtoul(treesNode->first_node("TX1")->value(), nullptr, 10);
			unsigned int tY1 = strtoul(treesNode->first_node("TY1")->value(), nullptr, 10);
			unsigned int tX2 = strtoul(treesNode->first_node("TX2")->value(), nullptr, 10);
			unsigned int tY2 = strtoul(treesNode->first_node("TY2")->value(), nullptr, 10);
			float tWidth = strtof(treesNode->first_node("TW")->value(), nullptr);
			float tHeight = strtof(treesNode->first_node("TH")->value(), nullptr);
			uv1 = ren->worldTileMap.uvTile(tX1, tY1, tWidth, tHeight);
			uv2 = ren->worldTileMap.uvTile(tX2, tY2, tWidth, tHeight);
		}
		treeComp->addTree(origin, tile, wLevel, uv1, uv2);
		treeCount++;
	}

	//Add tree comp
	treeComp->generateIndices();
	std::lock_guard<std::shared_mutex> heapLock(manager->getHeapMutex());
	manager->pushRenderHeap(treeComp, &tree->m_RenderComps);

	//Get tree name
	std::string name;
	if (treesRoot->first_attribute("name"))
	{
		name = treesRoot->first_attribute("name")->value();
	}

	std::lock_guard<std::shared_mutex> objLock(manager->getObjectMutex());
	manager->pushGameObject(tree, name);
	EngineLog("Time taken to load trees: ", EngineTimer::EndTimer(ts));
	return true;
}

bool WorldParse::ParseGlobalObjects(ObjectManager* manager, XML_Doc_Wrapper doc, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld)
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

void WorldParse::LoadLoadingZone(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld)
{
	using namespace rapidxml;
	std::shared_ptr<LoadingZone> zone(new LoadingZone());
	std::shared_ptr<RenderComponentGroup<LoadingZoneComponent>> group(new RenderComponentGroup<LoadingZoneComponent>());
	std::lock_guard<std::shared_mutex> groupLock(manager->getGroupMutex());
	manager->pushRenderGroup(group, "LoadingZones");
	//Load each zone
	std::shared_ptr<OvSpr_RunningSprite> playerPtr = manager->objectAt<OvSpr_RunningSprite>(manager->queryObjectID("Player"));
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
	std::lock_guard<std::shared_mutex> objLock(manager->getObjectMutex());
	manager->pushGameObject(zone, "LoadingZone");
}

void WorldParse::LoadSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, OverworldRenderer* ren, World::LevelID levelID, bool* interaction)
{
	using rapidxml::xml_node;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	OvSpr_SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::lock_guard<std::shared_mutex> gLock(manager->getGroupMutex());
	std::shared_ptr<OvSpr_Sprite> sprite = Ov_ObjCreation::BuildSprite(data, ren->spriteTileMap, manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender")).get(), &ren->spriteRenderer);

	//Push object
	std::lock_guard<std::shared_mutex> objLock(manager->getObjectMutex());
	manager->pushGameObject(sprite, name);
}

void WorldParse::LoadDirectionalSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, OverworldRenderer* ren, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID, GameInput* input)
{
	using rapidxml::xml_node;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	OvSpr_SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::lock_guard<std::shared_mutex> gLock(manager->getGroupMutex());
	std::shared_ptr<OvSpr_DirectionalSprite> sprite = Ov_ObjCreation::BuildDirectionalSprite(data, ren->spriteTileMap, manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender")).get(),
			manager->updateGroupAt<SpriteMap>(manager->queryGroupID("SpriteMap")).get(), manager->updateGroupAt<UpdateAnimationFacing>(manager->queryGroupID("UpdateFacing")).get(), &ren->spriteRenderer);

	//Optionals
	WorldParse::DirectionalSpriteOptionals(node, sprite);

	//Push object
	std::lock_guard<std::shared_mutex> oLock(manager->getObjectMutex());
	manager->pushGameObject(sprite, name);
}

void WorldParse::LoadWalkingSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, OverworldRenderer* ren, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, World::LevelID levelID, GameInput* input)
{
	using rapidxml::xml_node;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	OvSpr_SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::lock_guard<std::shared_mutex> gLock(manager->getGroupMutex());
	std::shared_ptr<OvSpr_WalkingSprite> sprite = Ov_ObjCreation::BuildWalkingSprite(data, ren->spriteTileMap, manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender")).get(),
			manager->updateGroupAt<SpriteMap>(manager->queryGroupID("SpriteMap")).get(), manager->updateGroupAt<UpdateAnimationWalking>(manager->queryGroupID("UpdateWalking")).get(), &ren->spriteRenderer);
	
	//Optionals
	WorldParse::WalkingSpriteOptionals(node, manager, sprite);
	WorldParse::OverworldScriptOptionals(node, manager, flags, textBuff, sprite, input);

	//Push object
	std::lock_guard<std::shared_mutex> oLock(manager->getObjectMutex());
	manager->pushGameObject(sprite, name);
}

void WorldParse::OverworldScriptOptionals(rapidxml::xml_node<>* node, ObjectManager* manager, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, std::shared_ptr<OvSpr_Sprite> sprite, GameInput* input)
{
	//Currently working on scripts - TODO - Make work for any sprite and access player properly
	if (node->first_node("NPCScript"))
	{
		std::string filePath = node->first_node("NPCScript")->value();
		ScriptParse::ScriptWrapper script = ScriptParse::ParseScriptFromText(filePath);
		std::lock_guard<std::shared_mutex> oLock(manager->getObjectMutex());
		std::shared_ptr<OvSpr_RunningSprite> player = std::static_pointer_cast<OvSpr_RunningSprite>(manager->getObjects()[0].obj);
		std::shared_ptr<NPC_OverworldScript> npcScript(new NPC_OverworldScript(script.script, script.size, player.get(), flags, input));
		npcScript->linkText(&textBuff->t1, &textBuff->t2, &textBuff->showTextBox);
		npcScript->linkNPC(std::static_pointer_cast<OvSpr_RunningSprite>(sprite)); //if sprite type doesnt support command, undefined behaviour - TODO fix
		manager->pushUpdateHeap(npcScript, &sprite->m_UpdateComps);
	}
}

void WorldParse::DirectionalSpriteOptionals(rapidxml::xml_node<>* node, std::shared_ptr<OvSpr_DirectionalSprite> sprite)
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

void WorldParse::WalkingSpriteOptionals(rapidxml::xml_node<>* node, ObjectManager* manager, std::shared_ptr<OvSpr_WalkingSprite> sprite)
{
	//Optionals
	using rapidxml::xml_node;
	//Do optionals for directional which should be available to walking
	WorldParse::DirectionalSpriteOptionals(node, std::static_pointer_cast<OvSpr_DirectionalSprite>(sprite));
	xml_node<>* randWNode = node->first_node("RandWalk");
	if (randWNode)
	{
		std::shared_ptr<UpdateComponentGroup<NPC_RandWalk>> tmp = manager->updateGroupAt<NPC_RandWalk>(manager->queryGroupID("RandWalk"));
		NPC_RandWalk rWalk(&sprite->m_Direction, &sprite->m_WorldLevel, &sprite->m_YPos, &sprite->m_Busy, &sprite->m_Walking, &sprite->m_Sprite, &sprite->m_Timer);
		rWalk.linkLocation(&sprite->m_TileX, &sprite->m_TileZ, &sprite->m_XPos, &sprite->m_ZPos, &sprite->m_CurrentLevel);

		//Check active and add
		bool active = (bool)strtoul(randWNode->value(), nullptr, 10);
		if (!active)
		{
			rWalk.setActive(false);
		}
		tmp->addExistingComponent(&sprite->m_UpdateComps, rWalk);
	}
}

OvSpr_SpriteData WorldParse::BuildSprDataFromXNode(rapidxml::xml_node<>* node, World::LevelID levelID)
{
	using rapidxml::xml_node;
	OvSpr_SpriteData data;

	xml_node<>* worldLNode = node->first_node("WLevel");
	World::WorldHeight worldLevel = (World::WorldHeight)strtol(worldLNode->value(), nullptr, 10);
	xml_node<>* tXNode = node->first_node("TX");
	xml_node<>* tYNode = node->first_node("TY");
	World::TileTexture tex = { strtoul(tXNode->value(), nullptr, 10), strtoul(tYNode->value(), nullptr, 10) };

	//Position in tiles
	xml_node<>* xTileNode = node->first_node("TileX");
	unsigned int xTile = strtoul(xTileNode->value(), nullptr, 10);
	xml_node<>* zTileNode = node->first_node("TileZ");
	unsigned int zTile = strtoul(zTileNode->value(), nullptr, 10);
	
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
void WorldParse::LoadBridge(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, OverworldRenderer* ren, World::LevelID levelID)
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
	TileUV tex1 = ren->worldTileMap.uvTile(t1.textureX, t1.textureY);
	TileUV tex2 = ren->worldTileMap.uvTile(t2.textureX, t2.textureY);

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
	World::TileLoc tile = { strtoul(tileXNode->value(), nullptr, 10), strtoul(tileZNode->value(), nullptr, 10) };
	Struct2f offset = World::Level::queryOrigin(levelID);
	offset.a += tile.x * World::TILE_SIZE;
	offset.b -= tile.z * World::TILE_SIZE;

	std::shared_ptr<Bridge> bridge(new Bridge());
	std::shared_ptr<BridgeRenderComponent> bridgeRen(new BridgeRenderComponent(offset, worldLevel, width, height, tex1, tex2, &ren->worldRenderer, horizontal));
	bridgeRen->generateIndices();

	std::lock_guard<std::shared_mutex> heapLock(manager->getHeapMutex());
	manager->pushRenderHeap(bridgeRen, &bridge->m_RenderComps);
	std::lock_guard<std::shared_mutex> oLock(manager->getObjectMutex());
	manager->pushGameObject(bridge, name);
}

void World::LevelContainer::InitialiseLevels(ObjectManager* obj, OverworldRenderer* ren, FlagArray* flags, GameGUI::TextBoxBuffer* textBuff, GameInput* input)
{
	m_Input = input; m_ObjManager = obj; m_Renderer = ren; m_Flags = flags; m_TextBuffer = textBuff;
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
	m_Levels[(int)id].buildLevel(&m_Renderer->worldRenderer, &m_Renderer->worldTileMap);
}

void World::LevelContainer::InitialiseGlobalObjects()
{
	WorldParse::XML_Doc_Wrapper doc = WorldParse::ParseLevelXML(World::LevelID::LEVEL_NULL, true);
	std::function<void(World::LevelID)> ld = std::bind(&World::LevelContainer::LoadLevel, this, std::placeholders::_1);
	std::function<void(World::LevelID)> uld = std::bind(&World::LevelContainer::UnloadLevel, this, std::placeholders::_1);
	WorldParse::ParseGlobalObjects(m_ObjManager, doc, ld, uld);
}

void World::LevelContainer::LoadLevel(World::LevelID id)
{
	//Locks if loading somewhere else, and then checks if has been loaded
	std::lock_guard<std::mutex> levelAccessLock(m_LevelMutexes[(int)id]);
	if (m_Levels[(int)id].loaded())
	{
		return;
	}

	using namespace rapidxml;
	if (m_Levels[(int)id].buildLevel(&m_Renderer->worldRenderer, &m_Renderer->worldTileMap))
	{
		using namespace  WorldParse;
		xml_node<>* root;
		xml_node<>* objRoot;
		xml_node<>* treeRoot;
		std::future<bool> f1; std::future<bool> f2; std::shared_mutex mutex;
		XML_Doc_Wrapper doc = ParseLevelXML(id, false);

		root = doc.doc->first_node();
		if (!root)
		{
			EngineLog("Root node not found. Error on level: ", (int)id);
			return;
		}

		//Check for if trees or objects are meant to be present to avoid sending off a redundant thread
		objRoot = root->first_node("Objects");
		treeRoot = root->first_node("Trees");

		if (objRoot)
		{
			f1 = std::async(std::launch::async, &ParseLevelObjects, m_ObjManager, m_Renderer, id, m_Flags, m_TextBuffer, std::ref(mutex), doc, m_Input);
		}
		if (treeRoot)
		{
			f2 = std::async(std::launch::async, &ParseLevelTrees, m_ObjManager, m_Renderer, id, std::ref(mutex), doc);
		}
	}
	m_Levels[(int)id].setLoaded(true);
}

void World::LevelContainer::UnloadLevel(World::LevelID id)
{
	//Locks if loading somewhere else then checks if already unloaded
	std::lock_guard<std::mutex> levelAccessLock(m_LevelMutexes[(int)id]);
	if (!m_Levels[(int)id].loaded())
	{
		return;
	}
	
	//Purge all level tagged objects
	{
		std::lock_guard<std::shared_mutex> objLock(m_ObjManager->getObjectMutex());
		std::vector<ObjectManager::GameObjectContainer>& objs = m_ObjManager->getObjects();
		for (int i = 0; i < objs.size(); i++)
		{
			if (objs[i].name.find("Level" + std::to_string((int)id)) != std::string::npos)
			{
				m_ObjManager->removeObject(i);
			}
		}
	}

	m_Levels[(int)id].purgeLevel();
	m_Levels[(int)id].setLoaded(false);
}

void World::LevelContainer::render()
{
	for (int i = 0; i < m_Levels.size(); i++)
	{
		m_Levels[i].render();
	}
}