#pragma once
#ifndef OVERWORLD_SCRIPT_H
#define OVERWORLD_SCRIPT_H

#include "vector"
#include "stdint.h"
#include "game/objects/OverworldSprite.h"
#include "game/objects/Script.hpp"
#include "game/gui/GUI.h"
#include "game/data/PlayerData.hpp"
#include "game/utility/GameText.h"
#include "game/utility/Input.hpp"
#include "game/utility/PathFinding.h"

//Script component to be applied to object
inline bool WaitScript(float& timer, float endTime, double deltaTime)
{
	if (timer >= endTime)
	{
		timer = 0.0f;
		return true;
	}
	timer += deltaTime;
	return false;
}

inline static bool WaitInput(GameInput* input)
{
	if (input->HELD_E || input->PRESSED_E || input->PRESSED_X)
	{
		return true;
	}
	return false;
}

inline static int GetItemIndexInBag(PlayerBag& bag, int id)
{
	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (bag.items[i].item.id == id)
		{
			return i;
		}
	}
	return 0;
}

class OverworldScript : public UpdateComponent
{
public:
	OverworldScript() = default;
	OverworldScript(Script script, uint16_t size, std::shared_ptr<OvSpr_RunningSprite> player) { m_Script = script; m_Size = size; m_Player = player; }
	
	static void init(PlayerData* data, GameInput* input, std::function<void(World::LevelID)> load, std::function<void(World::LevelID)> unload) { m_Data = data; m_Input = input; m_LoadLv = load; m_UnloadLv = unload; };
	void setScript(Script script, uint16_t size) { m_Script = script; m_Size = size; m_Index = 0; }
	void linkText(GameGUI::TextBoxBuffer* buffer) { m_TextBuffer = buffer; }
	void forceSkip() { m_Index++; }
	void setPath(PathFinding::Path path) { m_Path = path; }
	int getIndex() { return m_Index; }
	ScriptInstruction getInstruction() { if (m_Index >= m_Size) { return m_Script[0].instruction; } return m_Script[m_Index].instruction; }
	ScriptElement& getElement(int index) { return m_Script[index]; }
	void update(double deltaTime)
	{
		if (m_Index >= m_Size)
		{
			m_Index = 0;
		}
		process(deltaTime);
	}

	virtual bool process(double deltaTime)
	{
		//Carries out core functionality here that all implementations use
		//Then returns whether index is fully processed
		ScriptElement el = m_Script[m_Index];
		switch (el.instruction)
		{
		case ScriptInstruction::BLANK_OP:
			m_Index++;
			return true;
		case ScriptInstruction::JMP:
			m_Index = el.info.jmpInfo.line;
			return true;
		case ScriptInstruction::JMP_IF:
			if (m_Data->flags.at(el.info.jmpIfInfo.flagLoc))
			{
				m_Index = el.info.jmpIfInfo.line;
				return true;
			}
			m_Index++;
			return true;
		case ScriptInstruction::SET_FLAG:
			m_Data->flags.at(el.info.flgInfo.flagLoc) = el.info.flgInfo.state;
			m_Index++;
			return true;
		case ScriptInstruction::WAIT_SEC:
			if (WaitScript(m_Timer, 1.0f, deltaTime))
			{
				m_Index++;
			}
			return true;
		case ScriptInstruction::PLAYER_LOCK:
			m_Player->m_Busy = el.info.boolInfo.state;
			m_Index++;
			return true;
		case ScriptInstruction::PLAYER_FACE:
			m_Player->m_Direction = (World::Direction)el.info.dirInfo.direction;
			m_Index++;
			return true;
		case ScriptInstruction::PLAYER_WALK:
			if (SpriteWalk(m_Player.get(), m_Player->m_Controlled, deltaTime))
			{
				m_Index++;
			}
			return true;
		case ScriptInstruction::PLAYER_RUN:
			if (SpriteRun(m_Player.get(), m_Player->m_Controlled, deltaTime))
			{
				m_Index++;
			}
			return true;
		case ScriptInstruction::PLAYER_WALK_TO_TILE:
			if (m_Path.directionsIndex == -1)
			{
				m_Path = PathFinding::GetPath(m_Player.get(), { (int)el.info.tileInfo.x, (int)el.info.tileInfo.z }, (World::WorldHeight)el.info.tileInfo.h);
				if (m_Path.directionsIndex == -2)
				{
					m_Index++;
					return true;
				}
				m_Player->m_Direction = m_Path.directions[m_Path.directionsIndex];
				m_Path.directionsIndex++;
			}
			else if (m_Path.directionsIndex >= m_Path.directions.size())
			{
				if (SpriteWalk(m_Player.get(), m_Player->m_Controlled, deltaTime))
				{
					PathFinding::Path path; m_Path = path;
					m_Index++;
				}
				return true;
			}
			else if (SpriteWalk(m_Player.get(), m_Player->m_Controlled, deltaTime))
			{
				PathFinding::ValidatePath(m_Player.get(), { (int)el.info.tileInfo.x, (int)el.info.tileInfo.z }, (World::WorldHeight)el.info.tileInfo.h, m_Path);
				m_Player->m_Direction = m_Path.directions[m_Path.directionsIndex];
				m_Path.directionsIndex++;
			}
			return true;
		case ScriptInstruction::PLAYER_RUN_TO_TILE:
			if (m_Path.directionsIndex == -1)
			{
				m_Path = PathFinding::GetPath(m_Player.get(), { (int)el.info.tileInfo.x, (int)el.info.tileInfo.z }, (World::WorldHeight)el.info.tileInfo.h);
				if (m_Path.directionsIndex == -2)
				{
					m_Index++;
					return true;
				}
				m_Player->m_Direction = m_Path.directions[m_Path.directionsIndex];
				m_Path.directionsIndex++;
			}
			else if (m_Path.directionsIndex >= m_Path.directions.size())
			{
				if (SpriteRun(m_Player.get(), m_Player->m_Controlled, deltaTime))
				{
					PathFinding::Path path; m_Path = path;
					m_Index++;
				}
				return true;
			}
			else if (SpriteRun(m_Player.get(), m_Player->m_Controlled, deltaTime))
			{
				PathFinding::ValidatePath(m_Player.get(), { (int)el.info.tileInfo.x, (int)el.info.tileInfo.z }, (World::WorldHeight)el.info.tileInfo.h, m_Path);
				m_Player->m_Direction = m_Path.directions[m_Path.directionsIndex];
				m_Path.directionsIndex++;
			}
			return true;
		case ScriptInstruction::OPEN_MSG_BOX:
			m_TextBuffer->showTextBox = true;
			m_Index++;
			return true;
		case ScriptInstruction::CLOSE_MSG_BOX:
			m_TextBuffer->showTextBox = false;
			m_Index++;
			return true;
		case ScriptInstruction::MSG:
			if (!m_Messaging)
			{
				m_TextBuffer->buffer.pushBuffer(GameText::FindMessage(el.info.msgInfo));
				m_TextBuffer->buffer.nextPage();
				m_Messaging = true;
				return true;
			}
			else if (!m_TextBuffer->buffer.isReady())
			{
				if (!WaitScript(m_Timer, 0.4f, deltaTime))
				{
					return true;
				}
				if (WaitInput(m_Input) && m_TextBuffer->buffer.isNextPageReady())
				{
					m_TextBuffer->buffer.nextPage();
					m_Timer = 0.0f;
					return true;
				}
				return true;
			}
			if (!WaitScript(m_Timer, 0.4f, deltaTime))
			{
				return true;
			}
			m_Messaging = false;
			m_Index++;
			return true;
		case ScriptInstruction::CLEAR_TEXT:
			if (!m_TextBuffer->buffer.isNextPageReady())
			{
				return true;
			}
			m_TextBuffer->buffer.line1 = "";
			m_TextBuffer->buffer.line2 = "";
			m_Index++;
			return true;
		case ScriptInstruction::WAIT_INPUT:
			if (WaitInput(m_Input))
			{
				m_Index++;
			}
			return true;
		case ScriptInstruction::WAIT_FOR:
			if (!WaitScript(m_Timer, ((float)el.info.waitInfo.millis)/1000.0f, deltaTime))
			{
				return true;
			}
			m_Index++;
			return true;
		case ScriptInstruction::GIVE_ITEM:
			{
				int itemIndex = GetItemIndexInBag(m_Data->bag, el.info.itemInfo.itemID);
				m_Data->bag.items[itemIndex].count += el.info.itemInfo.quantity;
			}
			m_Index++;
			return true;
		case ScriptInstruction::TAKE_ITEM:
			{
				int itemIndex = GetItemIndexInBag(m_Data->bag, el.info.itemInfo.itemID);
				m_Data->bag.items[itemIndex].count -= el.info.itemInfo.quantity;
			}
			m_Index++;
			return true;
		case ScriptInstruction::SET_PLAYER_TILE:
			{
				//Get current tile
				World::Tile tileSrc = m_Player->m_Tile;
				//Get distance to new one
				int distX = el.info.tileInfo.x - tileSrc.x - 1;
				int distZ = el.info.tileInfo.z - tileSrc.z - 1;

				m_Player->m_XPos += distX * World::TILE_SIZE;
				m_Player->m_ZPos -= distZ * World::TILE_SIZE;
				Translate<NormalTextureVertex>(&m_Player->m_Sprite, distX * World::TILE_SIZE, 0.0f, -distZ * World::TILE_SIZE, Shape::QUAD);
			}

			m_Index++;
			return true;
		case ScriptInstruction::WARP_PLAYER:
			//Load destination level
			m_LoadLv((World::LevelID)el.info.warpInfo.dest);
			//Unload current level
			m_UnloadLv((World::LevelID)el.info.warpInfo.origin);
			{
				//Teleport to location
				Struct2f destOrigin = World::Level::queryOrigin((World::LevelID)el.info.warpInfo.dest);
				float newXPos = destOrigin.a + World::TILE_SIZE + World::TILE_SIZE / 2;
				float newZPos = destOrigin.b - World::TILE_SIZE - World::TILE_SIZE / 2;
				Position<NormalTextureVertex>(&m_Player->m_Sprite, { m_Player->m_XPos - World::TILE_SIZE / 2, m_Player->m_YPos, m_Player->m_ZPos }, { newXPos - World::TILE_SIZE / 2, m_Player->m_YPos, newZPos }, Shape::QUAD);
				m_Player->m_XPos = newXPos; m_Player->m_ZPos = newZPos;
				m_Player->m_CurrentLevel = (World::LevelID)el.info.warpInfo.dest;
				m_Player->m_Tile = { 0,0 };
			}
			m_Index++;
			//Process next instruction for tile set
			process(deltaTime);
			return true;
		}
		return false;
	}
public:

	//Private helper functions
	bool SpriteWalk(OvSpr_WalkingSprite* subject, bool& setBusy, double deltaTime)
	{
		if (!subject->m_Walking)
		{
			if (!Ov_Translation::CheckCanWalk(subject))
			{
				return true;
			}
			subject->m_Walking = true;
			setBusy = true;
			World::ModifyTilePerm(subject->m_CurrentLevel, subject->m_Direction, subject->m_Tile, subject->m_WorldLevel, subject->m_LastPermission, subject->m_LastPermissionPtr);
		}
		else if (!Ov_Translation::SpriteWalk(subject, deltaTime))
		{
			setBusy = false;
			return true;
		}
		return false;
	}

	bool SpriteRun(OvSpr_RunningSprite* subject, bool& setBusy, double deltaTime)
	{
		if (!subject->m_Running)
		{
			if (!Ov_Translation::CheckCanWalk(subject))
			{
				return true;
			}
			subject->m_Running = true;
			setBusy = true;
			World::ModifyTilePerm(subject->m_CurrentLevel, subject->m_Direction, subject->m_Tile, subject->m_WorldLevel, subject->m_LastPermission, subject->m_LastPermissionPtr);
		}
		else if (!Ov_Translation::SpriteRun(subject, deltaTime))
		{
			setBusy = false;
			return true;
		}
		return false;
	}

	Script m_Script;
	uint16_t m_Index = 0;
	uint16_t m_Size;
	std::shared_ptr<OvSpr_RunningSprite> m_Player;
	GameGUI::TextBoxBuffer* m_TextBuffer = nullptr;
	bool m_Messaging = false;
	PathFinding::Path m_Path; //TODO - store less variables here
	static std::function<void(World::LevelID)> m_LoadLv;
	static std::function<void(World::LevelID)> m_UnloadLv;
	static GameInput* m_Input;
	static PlayerData* m_Data;
private:
	float m_Timer = 0.0;
};

#endif