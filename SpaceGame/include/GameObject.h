#pragma once
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Renderer.h"

class GameObject
{
public:
	void setRenderer(Renderer* ren);
	void render();
	void update(double deltaTime);

	//getter
	float xPos() const { return m_XPos; }
	float yPos() const { return m_YPos; }

protected:
	Renderer* m_Renderer;
	float m_XPos = 0.0f;
	float m_YPos = 0.0f;

};







#endif