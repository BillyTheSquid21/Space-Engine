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

void PathFinding::ValidatePath(OvSpr_WalkingSprite* subject, World::Tile dest, Path& path)
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

	if (!blockage)
	{
		return;
	}

	//If blocked, find new path
	EngineLog("Path blocked, recalculating!");
	path = GetPath(subject, dest);
}

PathFinding::Path PathFinding::GetPath(OvSpr_WalkingSprite* subject, World::Tile dest)
{
	//Get vector of tiles
	std::vector<PathNode> nodes = FindPath(subject, dest);

	//Create vector of directions and init to nodes size
	std::vector<World::Direction> directions;
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

	return { nodes, directions, 0 };
}

std::vector<PathFinding::PathNode> PathFinding::FindPath(OvSpr_WalkingSprite* subject, World::Tile dest)
{
	using namespace World;
	//Get permissions fragment of current layer
	auto timer = EngineTimer::StartTimer();
	Level::PermVectorFragment layer = Level::queryPermissions(subject->m_CurrentLevel, subject->m_WorldLevel);
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

	PathNode startNode = { -1, start, 0, distStartToEnd };
	openList.push_back(startNode);

	bool targetFound = false;

	//2. repeat steps
	//2.a Look for node with lowest (g+h) on open list
	constexpr int MAX_STEPS = 100; //To avoid being stuck in a loop of trying to find a location
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

				//if is not clear, continue
				MovementPermissions permission = layer.pointer[nextTile.x * dimensions.levelH + nextTile.z];
				if (permission != MovementPermissions::CLEAR)
				{
					continue;
				}

				//2.c.i if is on the closed list, ignore
				for (int i = 0; i < closedList.size(); i++)
				{
					if (closedList[i].tile == nextTile)
					{
						continue;
					}
				}

				//2.c.ii if isnt on open list, add to open list
				//make current node the parent and note dists
				PathNode nextNode = { -1, nextTile, 0, 0 };
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