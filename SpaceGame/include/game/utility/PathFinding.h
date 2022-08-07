#pragma once
#ifndef PATH_FINDING_H
#define PATH_FINDING_H

#include "cmath"
#include "limits"
#include "functional"
#include "game/level/World.h"
#include "game/objects/OverworldSprite.h"

namespace PathFinding
{
	struct PathNode
	{
		int parentIndex = -1;
		World::Tile tile;
		int gDist; //Dist from start
		int hDist; //Est dist to destination - (done as along then up)
	};

	struct Path
	{
		std::vector<PathNode> path;
		std::vector<World::Direction> directions;
		unsigned int directionsIndex = -1;
	};

	Path GetPath(OvSpr_WalkingSprite* subject, World::Tile dest);
	std::vector<PathNode> FindPath(OvSpr_WalkingSprite* subject, World::Tile dest);
	void ValidatePath(OvSpr_WalkingSprite* subject, World::Tile dest, Path& path);
}

#endif