#pragma once
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "Renderer.hpp"

class GameObject
{
public:
	void setRenderer(Renderer<Vertex>* ren);
	void render();
	void update(double deltaTime);

	//getter
	float xPos() const { return m_XPos; }
	float yPos() const { return m_YPos; }

protected:
	Renderer<Vertex>* m_Renderer;
	float m_XPos = 0.0f;
	float m_YPos = 0.0f;

};







#endif