#include "game/level/WorldObjectLoader.h"

enum class ObjectType
{
	Trees, DirectionalSprite, WalkingSprite, NULL_TYPE
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
	else
	{
		return ObjectType::NULL_TYPE;
	}
}

WorldParse::XML_Doc_Wrapper WorldParse::ParseLevelXML(World::LevelID id)
{
	//Setup
	using namespace rapidxml;
	std::string path = "";
	{
		std::string tmp1 = "res/level/level";
		std::string tmp2 = "_obj.xml";
		path = tmp1 + std::to_string((int)id) + tmp2;
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

bool WorldParse::ParseLevelObjects(ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID, FlagArray* flags, std::shared_mutex& mutex, XML_Doc_Wrapper doc)
{
	//Setup
	using namespace rapidxml;
	std::shared_lock<std::shared_mutex> fileLock(mutex);
	auto ts = EngineTimer::StartTimer();

	//Load

	//If no trees present, stop
	xml_node<>* objRoot = doc.doc->first_node()->first_node("objects");
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
			WorldParse::LoadDirectionalSprite(name, identifyingNode, manager, map, ren, flags, levelID);
			break;
		case ObjectType::WalkingSprite:
		{
			WorldParse::LoadWalkingSprite(name, identifyingNode, manager, map, ren, flags, levelID);
			break;
		}
		default:
			break;
		}
		objCount++;
	}
	EngineLog("Time taken to load objects: ", EngineTimer::EndTimer(ts));
	return true;
}

bool WorldParse::ParseLevelTrees(ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID, std::shared_mutex& mutex, XML_Doc_Wrapper doc)
{
	//Setup
	using namespace rapidxml;
	std::shared_lock<std::shared_mutex> fileLock(mutex);
	auto ts = EngineTimer::StartTimer();

	xml_node<>* treesRoot = doc.doc->first_node()->first_node("trees");
	if (!treesRoot)
	{
		EngineLog("Error loading trees: ", (int)levelID);
		return false;
	}

	std::shared_ptr<TreeRenderComponent> treeComp(new TreeRenderComponent(ren));
	std::shared_ptr<TreeObject> tree(new TreeObject());
	treeComp->reserveTrees(strtoul(treesRoot->first_node("count")->value(), nullptr, 10));

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
		World::TileLoc tile = { strtoul(treesNode->first_node("tileX")->value(), nullptr, 10), strtoul(treesNode->first_node("tileZ")->value(), nullptr, 10) };
		World::WorldLevel wLevel = (World::WorldLevel)strtol(treesNode->first_node("wlevel")->value(), nullptr, 10);
		UVData uv1; UVData uv2;
		{
			unsigned int tX1 = strtoul(treesNode->first_node("tX1")->value(), nullptr, 10);
			unsigned int tY1 = strtoul(treesNode->first_node("tY1")->value(), nullptr, 10);
			unsigned int tX2 = strtoul(treesNode->first_node("tX2")->value(), nullptr, 10);
			unsigned int tY2 = strtoul(treesNode->first_node("tY2")->value(), nullptr, 10);
			float tWidth = strtof(treesNode->first_node("tW")->value(), nullptr);
			float tHeight = strtof(treesNode->first_node("tH")->value(), nullptr);
			uv1 = map->uvTile(tX1, tY1, tWidth, tHeight);
			uv2 = map->uvTile(tX2, tY2, tWidth, tHeight);
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

//Loaders
void WorldParse::LoadSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID)
{
	using rapidxml::xml_node;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	OvSpr_SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::lock_guard<std::shared_mutex> gLock(manager->getGroupMutex());
	std::shared_ptr<OvSpr_Sprite> sprite = Ov_ObjCreation::BuildSprite(data, *map, manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender")).get(), ren);

	//Push object
	std::lock_guard<std::shared_mutex> objLock(manager->getObjectMutex());
	manager->pushGameObject(sprite, name);
}

void WorldParse::LoadDirectionalSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, FlagArray* flags, World::LevelID levelID)
{
	using rapidxml::xml_node;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	OvSpr_SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::lock_guard<std::shared_mutex> gLock(manager->getGroupMutex());
	std::shared_ptr<OvSpr_DirectionalSprite> sprite = Ov_ObjCreation::BuildDirectionalSprite(data, *map, manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender")).get(),
			manager->updateGroupAt<SpriteMap>(manager->queryGroupID("SpriteMap")).get(), manager->updateGroupAt<UpdateAnimationFacing>(manager->queryGroupID("UpdateFacing")).get(), ren);
	
	//Check for script
	if (node->first_node("npcScript"))
	{
		std::string filePath = node->first_node("npcScript")->value();
		ScriptParse::ScriptWrapper script = ScriptParse::ParseScriptFromText(filePath);
		std::lock_guard<std::shared_mutex> oLock(manager->getObjectMutex());
		std::shared_ptr<OvSpr_RunningSprite> player = std::static_pointer_cast<OvSpr_RunningSprite>(manager->getObjects()[0].obj);
		//NPC_Script<OvSpr_DirectionalSprite> npcScript(script.script, script.size, player.get(), flags);
	}

	//Optionals
	WorldParse::DirectionalSpriteOptionals(node, sprite);

	//Push object
	std::lock_guard<std::shared_mutex> oLock(manager->getObjectMutex());
	manager->pushGameObject(sprite, name);
}

void WorldParse::LoadWalkingSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, FlagArray* flags, World::LevelID levelID)
{
	using rapidxml::xml_node;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	OvSpr_SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::lock_guard<std::shared_mutex> gLock(manager->getGroupMutex());
	std::shared_ptr<OvSpr_WalkingSprite> sprite = Ov_ObjCreation::BuildWalkingSprite(data, *map, manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender")).get(),
			manager->updateGroupAt<SpriteMap>(manager->queryGroupID("SpriteMap")).get(), manager->updateGroupAt<UpdateAnimationWalking>(manager->queryGroupID("UpdateWalking")).get(), ren);
	
	//Optionals
	WorldParse::WalkingSpriteOptionals(node, manager, sprite);

	//Push object
	std::lock_guard<std::shared_mutex> oLock(manager->getObjectMutex());
	manager->pushGameObject(sprite, name);
}

void WorldParse::DirectionalSpriteOptionals(rapidxml::xml_node<>* node, std::shared_ptr<OvSpr_DirectionalSprite> sprite)
{
	//Optionals
	using rapidxml::xml_node;
	xml_node<>* dirNode = node->first_node("dir");
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
	xml_node<>* randWNode = node->first_node("randWalk");
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
	xml_node<>* xTileNode = node->first_node("tileX");
	unsigned int xTile = strtoul(xTileNode->value(), nullptr, 10);
	xml_node<>* zTileNode = node->first_node("tileZ");
	unsigned int zTile = strtoul(zTileNode->value(), nullptr, 10);
	//Check to make sure node is not occupied, if is, shuffle till a free spot is found
	{
		World::LevelPermission perm = World::RetrievePermission(levelID, { xTile, zTile });
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
			perm = World::RetrievePermission(levelID, { xTile, zTile });
			attempts++;
		}
		//If still fails to find a spot, will just intersect
	}
	xml_node<>* worldLNode = node->first_node("wlevel");
	World::WorldLevel worldLevel = (World::WorldLevel)strtol(worldLNode->value(), nullptr, 10);
	xml_node<>* tXNode = node->first_node("tX");
	xml_node<>* tYNode = node->first_node("tY");
	World::TileTexture tex = { strtoul(tXNode->value(), nullptr, 10), strtoul(tYNode->value(), nullptr, 10) };
	data = { {xTile,zTile}, worldLevel, levelID, tex };
	return data;
}

void World::LevelContainer::InitialiseLevels(ObjectManager* obj, Render::Renderer<TextureVertex>* sprRen, Render::Renderer<TextureVertex>* worRen, TileMap* sprMap, TileMap* worMap, FlagArray* flags)
{
	m_ObjManager = obj; m_SpriteRenderer = sprRen; m_WorldRenderer = worRen; m_SpriteTileMap = sprMap; m_WorldTileMap = worMap; m_Flags = flags;
	m_Levels.resize((int)World::LevelID::LEVEL_NULL);
	std::vector<std::mutex> mutexes((int)World::LevelID::LEVEL_NULL);
	m_LevelMutexes.swap(mutexes);
	for (int i = 0; i < m_Levels.size(); i++)
	{
		m_Levels[i].setID((World::LevelID)i);
	}
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
	if (m_Levels[(int)id].buildLevel(m_WorldRenderer, m_WorldTileMap))
	{
		using namespace  WorldParse;
		xml_node<>* root;
		xml_node<>* objRoot;
		xml_node<>* treeRoot;
		std::future<bool> f1; std::future<bool> f2; std::shared_mutex mutex;
		XML_Doc_Wrapper doc = ParseLevelXML(id);

		root = doc.doc->first_node();
		if (!root)
		{
			EngineLog("Root node not found. Error on level: ", (int)id);
			return;
		}

		//Check for if trees or objects are meant to be present to avoid sending off a redundant thread
		objRoot = root->first_node("objects");
		treeRoot = root->first_node("trees");

		if (objRoot)
		{
			f1 = std::async(std::launch::async, &ParseLevelObjects, m_ObjManager, m_SpriteTileMap, m_SpriteRenderer, id, m_Flags, std::ref(mutex), doc);
		}
		if (treeRoot)
		{
			f2 = std::async(std::launch::async, &ParseLevelTrees, m_ObjManager, m_WorldTileMap, m_WorldRenderer, id, std::ref(mutex), doc);
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
			if (objs[i].name.find("level" + std::to_string((int)id)) != std::string::npos)
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