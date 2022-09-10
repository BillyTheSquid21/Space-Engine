#include "game/utility/PathFinding.h"

//Debug that prints path - edits permissions so please do not use unless debugging
static void PrintPath(std::vector<PathFinding::PathNode>& vec, World::Level::PermVectorFragment perm, World::LevelDimensions dim)
{
	PathFinding::PathNode node = vec[vec.size() - 1];
	while (node.parentIndex != -1)
	{
		perm.pointer[node.tile.x * dim.levelH + node.tile.z] = (World::MovementPermissions)2;
		node = vec.at(node.parentIndex);
	}

	EngineLog("W: ", dim.levelW, " H: ", dim.levelH);
	for (int z = dim.levelH - 1; z > -1; z--)
	{
		for (int x = 0; x < dim.levelW; x++)
		{
			int repres = 0;
			if (perm.pointer[x * dim.levelH + z] != World::MovementPermissions::CLEAR)
			{
				repres = 1;
			}
			if ((int)perm.pointer[x * dim.levelH + z] == 2)
			{
				repres = 2;
			}
			std::cout << repres << " ";
		}
		std::cout << "\n";
	}
}

void PathFinding::ValidatePath(Ov_Sprite::WalkSprite* subject, World::Tile dest, World::WorldHeight height, Path& path)
{
	using namespace World;
	//Check each tile on path to ensure is still clear
	Level::PermVectorFragment layer = Level::queryPermissions(subject->m_CurrentLevel, subject->m_WorldLevel);
	LevelDimensions dimensions = Level::queryDimensions(subject->m_CurrentLevel);

	MovementPermissions permission; bool blockage = false;
	for (int i = 0; i < path.path.size(); i++)
	{
		//Ignores subjects tile
		Tile tile = path.path[i].tile;
		if (tile == subject->m_Tile)
		{
			continue;
		}

		permission = layer.pointer[tile.x * dimensions.levelH + tile.z];
		if (permission != MovementPermissions::CLEAR)
		{
			blockage = true;
			break;
		}
	}

	if (!blockage && dest == path.dest)
	{
		return;
	}

	//If blocked, find new path
	path = GetPath(subject, dest, height);
}

PathFinding::Path PathFinding::GetPath(Ov_Sprite::WalkSprite* subject, World::Tile dest, World::WorldHeight height)
{
	std::vector<PathNode> nodes;
	std::vector<World::Direction> directions;

	//If at destination, don't calculate
	if (subject->m_Tile == dest && subject->m_WorldLevel == height)
	{
		nodes.emplace_back(-1, dest, height, 0.0f, 0.0f);
		directions.push_back(World::Direction::DIRECTION_NULL);
		return { nodes, directions, -2, dest };
	}

	//Get vector of tiles
	nodes = FindPath(subject, dest, height);

	//Create vector of directions and init to nodes size
	int size = 1;
	PathNode node = nodes[nodes.size() - 1];
	while (node.parentIndex != -1)
	{
		node = nodes[node.parentIndex];
		size++;
	}
	size--;
	directions.resize(size);

	//Go backwards through list, and compare children to parents for dir
	PathNode childNode = nodes[nodes.size() - 1];
	int directionsIndex = directions.size() - 1;
	while (childNode.parentIndex != -1)
	{
		World::Direction direction;
		PathNode parentNode = nodes[childNode.parentIndex];
		direction = World::DirectionOfAdjacentTile(parentNode.tile, childNode.tile);
		directions[directionsIndex] = direction;
		childNode = nodes[childNode.parentIndex];
		directionsIndex--;
	}

	return { nodes, directions, 0, dest };
}

std::vector<PathFinding::PathNode> PathFinding::FindPath(Ov_Sprite::WalkSprite* subject, World::Tile dest, World::WorldHeight height)
{
	using namespace World;
	//Get permissions fragment of current layer
	auto timer = EngineTimer::StartTimer();

	std::vector<Permission> permissions;
	permissions.emplace_back(Level::queryPermissions(subject->m_CurrentLevel, subject->m_WorldLevel), subject->m_WorldLevel);
	LevelDimensions dimensions = Level::queryDimensions(subject->m_CurrentLevel);

	//find path
	std::vector<PathNode> openList;
	std::vector<PathNode> closedList;

	//1. add start node
	//1.a start node dist
	Tile start = subject->m_Tile;
	int distX = abs(start.x - dest.x);
	int distZ = abs(start.z - dest.z);
	int distStartToEnd = distX + distZ;

	PathNode startNode = { -1, start, subject->m_WorldLevel, 0, distStartToEnd };
	openList.push_back(startNode);

	bool targetFound = false;

	//2. repeat steps
	//2.a Look for node with lowest (g+h) on open list
	constexpr int MAX_STEPS = 1000; //To avoid being stuck in a loop of trying to find a location
	int step = 0;
	while (!targetFound && openList.size() > 0 && step < MAX_STEPS)
	{
		int lowestDist = std::numeric_limits<int>::max();
		int currentNodeIndex = -1;
		for (int i = 0; i < openList.size(); i++)
		{
			PathNode node = openList[i];
			int dist = node.gDist + node.hDist;
			if (dist < lowestDist)
			{
				lowestDist = dist;
				currentNodeIndex = i;
			}
		}

		//2.b switch lowest f node to closed list
		closedList.push_back(openList[currentNodeIndex]);
		if (currentNodeIndex != -1)
		{
			openList.erase(openList.begin() + currentNodeIndex);
		}
		currentNodeIndex = closedList.size() - 1;

		//Check if node was target
		if (closedList[currentNodeIndex].tile == dest)
		{
			targetFound = true;
		}

		//2.c iterate through all nodes reachable from current
		for (int x = -1; x <= 1; x++)
		{
			for (int z = -1; z <= 1; z++)
			{
				if (x == 0 && z == 0)
				{
					continue; //Skip current node
				}

				if (z != 0 && x != 0)
				{
					continue; //Skip diagonals
				}

				//Get next tile
				Tile nextTile = closedList[currentNodeIndex].tile;
				nextTile.x += x; nextTile.z += z;

				//if out of bounds, continue
				if (nextTile.x < 0 || nextTile.x >= dimensions.levelW
					|| nextTile.z < 0 || nextTile.z >= dimensions.levelH)
				{
					continue;
				}

				//If in bounds, make node
				PathNode nextNode = { -1, nextTile, closedList[currentNodeIndex].height, 0, 0 };

				//Check permissions for any new logic
				if (!CheckPermission(closedList[currentNodeIndex], nextNode, permissions, dimensions, subject->m_CurrentLevel))
				{
					continue;
				}

				//2.c.i if is on the closed list, ignore
				for (int i = 0; i < closedList.size(); i++)
				{
					if (closedList[i] == nextNode)
					{
						continue;
					}
				}

				//2.c.ii if isnt on open list, add to open list
				//make current node the parent and note dists
				bool onOpenList = false;
				int openListIndex = 0;
				for (int i = 0; i < openList.size(); i++)
				{
					if (openList[i].tile == nextTile)
					{
						onOpenList = true;
						openListIndex = i;
						break;
					}
				}

				//Set parent
				nextNode.parentIndex = currentNodeIndex;
				
				//Note dists
				//Get from start to tiles
				Tile startToParent = closedList[nextNode.parentIndex].tile - start;
				Tile startToNext = nextTile - start;
				//Take abs of tiles
				startToParent.x = abs(startToParent.x); startToParent.z = abs(startToParent.z);
				startToNext.x = abs(startToNext.x); startToNext.z = abs(startToNext.z);
				//add or sub one from dist depending on if closer
				int changeInDist = 0;
				if (startToNext.x < startToParent.x || startToNext.z < startToParent.z)
				{
					changeInDist++;
				}
				else
				{
					changeInDist--;
				}
				//Note dist from start
				nextNode.gDist = closedList[nextNode.parentIndex].gDist + changeInDist;
				nextNode.gDist = abs(nextNode.gDist);

				//Note dist to dest
				int distX = abs(nextTile.x - dest.x);
				int distZ = abs(nextTile.z - dest.z);
				int distNextToEnd = distX + distZ;
				nextNode.hDist = distNextToEnd;

				if (!onOpenList)
				{
					openList.push_back(nextNode);
					continue;
				}

				//If is a better path, replace entry on open list
				if (openList[openListIndex].hDist + openList[openListIndex].gDist
				> nextNode.gDist + nextNode.hDist)
				{
					openList[openListIndex] = nextNode;
				}
			}
		}
		step++;
	}

	EngineLog("Time to find path: ", EngineTimer::EndTimer(timer), "s");
	return closedList;
}

bool PathFinding::CheckPermission(PathNode& lastNode, PathNode& nextNode, std::vector<Permission>& perm, World::LevelDimensions dim, World::LevelID id)
{
	using namespace World;
	
	//Find permissions for current level
	MovementPermissions permission = MovementPermissions::WALL;
	bool permissionsFound = false;
	for (int i = 0; i < perm.size(); i++)
	{
		if (lastNode.height == perm[i].height)
		{
			permission = perm[i].permission.pointer[nextNode.tile.x * dim.levelH + nextNode.tile.z];
			permissionsFound = true;
			break;
		}
	}

	if (!permissionsFound)
	{
		Permission permissionReq = { Level::queryPermissions(id, lastNode.height), lastNode.height };
		permission = permissionReq.permission.pointer[nextNode.tile.x * dim.levelH + nextNode.tile.z];
		perm.push_back(permissionReq);
	}

	//Get direction from last node to this
	World::Direction direction = World::DirectionOfAdjacentTile(lastNode.tile, nextNode.tile);
	//Check permissions for additional logic
	switch (permission)
	{
	case MovementPermissions::CLEAR:
		return true;
	case MovementPermissions::STAIRS_NORTH:
		if (direction == World::Direction::SOUTH)
		{
			nextNode.height = (World::WorldHeight)((int)nextNode.height - 1);
			return true;
		}
		else if (direction == World::Direction::NORTH)
		{
			nextNode.height = (World::WorldHeight)((int)nextNode.height + 1);
			return true;
		}
		return false;
	case MovementPermissions::STAIRS_SOUTH:
		if (direction == World::Direction::NORTH)
		{
			nextNode.height = (World::WorldHeight)((int)nextNode.height - 1);
			return true;
		}
		else if (direction == World::Direction::SOUTH)
		{
			nextNode.height = (World::WorldHeight)((int)nextNode.height + 1);
			return true;
		}
		return false;
	case MovementPermissions::STAIRS_EAST:
		if (direction == World::Direction::WEST)
		{
			nextNode.height = (World::WorldHeight)((int)nextNode.height - 1);
			return true;
		}
		else if (direction == World::Direction::EAST)
		{
			nextNode.height = (World::WorldHeight)((int)nextNode.height + 1);
			return true;
		}
		return false;
	case MovementPermissions::STAIRS_WEST:
		if (direction == World::Direction::EAST)
		{
			nextNode.height = (World::WorldHeight)((int)nextNode.height - 1);
			return true;
		}
		else if (direction == World::Direction::WEST)
		{
			nextNode.height = (World::WorldHeight)((int)nextNode.height + 1);
			return true;
		}
		return false;
	default:
		return false;
	}
	return true;
}