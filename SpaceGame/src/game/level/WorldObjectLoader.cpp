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

void ParseLevelObjects(ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID)
{
	//Setup
	using namespace rapidxml;
	xml_document<>* doc = new xml_document<>();
	std::ifstream* ifs = new std::ifstream("res/level/level0_obj.xml");
	std::stringstream buffer;
	buffer << ifs->rdbuf();
	ifs->close();
	delete ifs;

	//Buffer and parse
	std::string tmp(buffer.str());
	doc->parse<0>(&tmp[0]);

	//Load
	xml_node<>* objRoot = doc->first_node("objects");

	for (xml_node<>* objNode = objRoot->first_node(); objNode; objNode = objNode->next_sibling())
	{
		xml_attribute<>* pAttr = objNode->first_attribute("name");
		std::string name = pAttr->value();
		xml_node<>* identifyingNode = objNode->first_node();
		ObjectType identifier = GetType(identifyingNode->name());
		
		switch (identifier)
		{
		case ObjectType::DirectionalSprite:
			LoadDirectionalSprite(name, identifyingNode, manager, map, ren, levelID);
			break;
		case ObjectType::WalkingSprite:
			LoadWalkingSprite(name, identifyingNode, manager, map, ren, levelID);
			break;
		default:
			break;
		}
	}
	delete doc;
}

void ParseLevelTrees(ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID) 
{
	//Setup
	using namespace rapidxml;
	xml_document<>* doc = new xml_document<>();
	std::ifstream* ifs = new std::ifstream("res/level/level0_obj.xml");
	std::stringstream buffer;
	buffer << ifs->rdbuf();
	ifs->close();
	delete ifs;

	//Buffer and parse
	std::string tmp(buffer.str());
	doc->parse<0>(&tmp[0]);

	//If no trees present, stop
	xml_node<>* treesRoot = doc->first_node("trees");
	if (!treesRoot)
	{
		return;
	}

	std::shared_ptr<TreeRenderComponent> treeComp(new TreeRenderComponent(ren));
	std::shared_ptr<TreeObject> tree(new TreeObject());
	treeComp->reserveTrees(strtoul(treesRoot->first_node("count")->value(), nullptr, 10));

	Struct2f origin = World::Level::queryOrigin(levelID);
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
	}

	//Add tree comp
	treeComp->generateIndices();
	manager->pushRenderHeap(treeComp, &tree->m_RenderComps);
	manager->pushGameObject(tree);

	delete doc;
}

//Loaders
void LoadSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID)
{
	using rapidxml::xml_node;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	OvSpr_SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::shared_ptr<OvSpr_Sprite> sprite = Ov_ObjCreation::BuildSprite(data, *map, manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender", true)).get(), ren);

	//Push object
	manager->pushGameObject(sprite, name);
}

void LoadDirectionalSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID)
{
	using rapidxml::xml_node;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	OvSpr_SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::shared_mutex& mutex = manager->GetGroupMutex();
	std::lock_guard<std::shared_mutex> lock(mutex);
	std::shared_ptr<OvSpr_DirectionalSprite> sprite = Ov_ObjCreation::BuildDirectionalSprite(data, *map, manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender", true)).get(),
		manager->updateGroupAt<SpriteMap>(manager->queryGroupID("SpriteMap", true)).get(), manager->updateGroupAt<UpdateAnimationFacing>(manager->queryGroupID("UpdateFacing", true)).get(), ren);
	
	//Optionals
	DirectionalSpriteOptionals(node, sprite);

	//Push object
	manager->pushGameObject(sprite, name);
}

void LoadWalkingSprite(std::string name, rapidxml::xml_node<>* node, ObjectManager* manager, TileMap* map, Render::Renderer<TextureVertex>* ren, World::LevelID levelID)
{
	using rapidxml::xml_node;
	//Check for nodes that MUST exist - those that make up the spritedata struct
	OvSpr_SpriteData data = BuildSprDataFromXNode(node, levelID);

	//Build base sprite
	std::shared_mutex& mutex = manager->GetGroupMutex();
	std::lock_guard<std::shared_mutex> lock(mutex);
	std::shared_ptr<OvSpr_WalkingSprite> sprite = Ov_ObjCreation::BuildWalkingSprite(data, *map, manager->renderGroupAt<SpriteRender>(manager->queryGroupID("SpriteRender", true)).get(),
		manager->updateGroupAt<SpriteMap>(manager->queryGroupID("SpriteMap", true)).get(), manager->updateGroupAt<UpdateAnimationWalking>(manager->queryGroupID("UpdateWalking", true)).get(), ren);

	//Optionals
	WalkingSpriteOptionals(node, manager, sprite);

	//Push object
	manager->pushGameObject(sprite, name);
}

void DirectionalSpriteOptionals(rapidxml::xml_node<>* node, std::shared_ptr<OvSpr_DirectionalSprite> sprite)
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

void WalkingSpriteOptionals(rapidxml::xml_node<>* node, ObjectManager* manager, std::shared_ptr<OvSpr_WalkingSprite> sprite)
{
	//Optionals
	using rapidxml::xml_node;
	//Do optionals for directional which should be available to walking
	DirectionalSpriteOptionals(node, std::static_pointer_cast<OvSpr_DirectionalSprite>(sprite));
	xml_node<>* randWNode = node->first_node("randWalk");
	if (randWNode)
	{
		std::shared_ptr<UpdateComponentGroup<NPC_RandWalk>> tmp = manager->updateGroupAt<NPC_RandWalk>(manager->queryGroupID("RandWalk", true));
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

OvSpr_SpriteData BuildSprDataFromXNode(rapidxml::xml_node<>* node, World::LevelID levelID)
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