#pragma once
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "renderer/Renderer.hpp"

class Component
{
public:
	//Function component does will carry out
	virtual void function() {};
};

class GameObject
{
public:
	//Removes components and deletes whole array
	virtual ~GameObject() { if (m_Components != NULL) { for (int i = 0; i < m_ComponentCount; i++) { delete m_Components[i]; } delete[] m_Components; } }

	void setRenderer(Renderer<Vertex>* ren) { m_Renderer = ren; };
	virtual void render() {};
	virtual void update(double deltaTime) {};

	//Getters
	float xPos() const { return m_XPos; }
	float yPos() const { return m_YPos; }

protected:
	//Components - stores pointers to components
	Component** m_Components;
	unsigned char m_ComponentCount;
	unsigned int m_ComponentBytes;

	//Rendering and pos
	Renderer<Vertex>* m_Renderer;
	float m_XPos = 0.0f;
	float m_YPos = 0.0f;
};







#endif