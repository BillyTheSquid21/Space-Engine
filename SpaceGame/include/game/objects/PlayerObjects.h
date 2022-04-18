#pragma once
#ifndef PLAYER_OBJECTS_H
#define PLAYER_OBJECTS_H

#include "game/objects/OverworldSprite.h"


//Player walk uses exact data of TilePosition so inherit for this
//TODO - refined many methods etc - however is currently very fast and so should be fine
class PlayerMove : public TilePosition
{
public:
	using TilePosition::TilePosition;
	void update(double deltaTime);
	void setPersistentInput(bool* shift, bool* up, bool* down, bool* left, bool* right) { m_Shift = shift; m_Up = up; m_Down = down, m_Left = left, m_Right = right; };
	void setSingleInput(bool* up, bool* down, bool* left, bool* right) { m_UpSingle = up; m_DownSingle = down; m_LeftSingle = left; m_RightSingle = right; };
	void setSpriteData(bool* walking, bool* running, bool* busy, World::Direction* direction, World::WorldLevel* level, float* y, TextureQuad* sprite) { m_Walking = walking; m_Busy = busy; m_YPos = y; m_Running = running; m_Direction = direction; m_WorldLevel = level; m_Sprite = sprite; }
private:
	bool canWalk();
	bool startWalk();
	bool startRun();
	bool checkInputs();
	void modifyTilePerm();
	bool walkPermHelper();

	//Modify
	void faceDirection();
	void cycleEnd(bool anyHeld);

	bool* m_Up = nullptr; bool* m_Down = nullptr;
	bool* m_Left = nullptr; bool* m_Right = nullptr;
	bool* m_UpSingle = nullptr; bool* m_DownSingle = nullptr;
	bool* m_LeftSingle = nullptr; bool* m_RightSingle = nullptr;
	bool* m_Shift = nullptr;
	float* m_YPos = nullptr;

	//Ascend stores whether was just going up or down
	//OnSlope stores whether is on a slope to get order of moving along and up slope right
	char m_Ascend = 0; //0 if no, 1 if up, -1 if down
	bool m_CurrentIsSlope = false; //Says whether currently on slope
	bool m_NextIsSlope = false;
	bool m_MoveVerticalFirst = false;

	double m_Timer = 0.0; bool* m_Walking = nullptr; bool* m_Running = nullptr; bool* m_Busy = nullptr;
	World::Direction* m_Direction = nullptr; World::WorldLevel* m_WorldLevel = nullptr;
	TextureQuad* m_Sprite;
};

class PlayerCameraLock : public RenderComponent
{
public:
	PlayerCameraLock() = default;
	PlayerCameraLock(float* x, float* y, float* z, Camera* cam) { m_XPos = x; m_YPos = y; m_ZPos = z; m_Camera = cam; }
	void render();
private:
	Camera* m_Camera = nullptr;
	float* m_XPos = nullptr;
	float* m_YPos = nullptr;
	float* m_ZPos = nullptr;
};

#endif