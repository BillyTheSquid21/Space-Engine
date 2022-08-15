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
		World::WorldHeight height;
		int gDist; //Dist from start
		int hDist; //Est dist to destination - (done as along then up)
		bool operator==(PathNode& node) { return (node.tile == this->tile) && (node.height == this->height); }
	};

	struct Path
	{
		std::vector<PathNode> path;
		std::vector<World::Direction> directions;
		int directionsIndex = -1; //-1 when blank, -2 when invalid
		World::Tile dest;
	};

	struct Permission
	{
		World::Level::PermVectorFragment permission;
		World::WorldHeight height;
	};

	Path GetPath(OvSpr_WalkingSprite* subject, World::Tile dest, World::WorldHeight height);
	std::vector<PathNode> FindPath(OvSpr_WalkingSprite* subject, World::Tile dest, World::WorldHeight height);
	void ValidatePath(OvSpr_WalkingSprite* subject, World::Tile dest, World::WorldHeight height, Path& path);
	static bool CheckPermission(PathNode& lastNode, PathNode& nextNode, std::vector<Permission>& perm, World::LevelDimensions dim, World::LevelID id);
}

#endif