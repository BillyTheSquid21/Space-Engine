#pragma once
#ifndef LOADING_ZONE_H
#define LOADING_ZONE_H

#include "core/GameObject.hpp"
#include "game/objects/OverworldSprite.h"
#include <functional>
#include "mtlib/ThreadPool.h"

//Class to pin all loading zones of a region to - eg all overworld, all inside building
//Means you can prevent loading an overworld level while in a house
class LoadingZone : public GameObject {};

//Loading zone "instance" - takes care of what rendering is carried out
class LoadingZoneComponent : public RenderComponent
{
public:
	LoadingZoneComponent(OvSpr_RunningSprite* spr, World::LevelID l1, World::LevelID l2) { m_PlayerPointer = spr; m_L1_ID = l1; m_L2_ID = l2; m_Pool = MtLib::ThreadPool::Fetch(); };
	
	void setZoneBounds(float x, float z, float w, float h) { m_Origin.a = x; m_Origin.b = -1 * z; m_Bounds.a = w; m_Bounds.b = h; }
	void setLoadingFuncs(std::function<void(World::LevelID)> ld, std::function<void(World::LevelID)> uld) { m_LoadLv = ld; m_UnloadLv = uld; };
	void render();
private:
	//Player data
	OvSpr_RunningSprite* m_PlayerPointer = nullptr;

	//Loading zone data
	Struct2f m_Origin = {0.0f, 0.0f}; //Lower left
	Struct2f m_Bounds = {0.0f, 0.0f}; //Width and height going out
	World::LevelID m_L1_ID;
	World::LevelID m_L2_ID;
	bool m_PlayerInside = false;

	std::function<void(World::LevelID)> m_LoadLv; 
	std::function<void(World::LevelID)> m_UnloadLv;
	MtLib::ThreadPool* m_Pool;
};

#endif