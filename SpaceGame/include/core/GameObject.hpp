#pragma once
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "renderer/Renderer.hpp"

class Component
{
public:
	//Function component does will carry out
	Component() {}
	void setID(unsigned char id) { m_ID = id; }
	virtual void function(double deltaTime, double time) {};
private:
	unsigned char m_ID = 0;
};

static unsigned int CumulativeID = 0;

class GameObject
{
public:
	GameObject() { m_ID = CumulativeID; CumulativeID++; }

	//Removes components and deletes whole array
	virtual ~GameObject() { for (int i = 0; i < m_Components.size(); i++) { delete m_Components[i]; }  }

	void setRenderer(Renderer<TextureVertex>* ren) { m_Renderer = ren; };
	virtual void render() { EngineLog("Render: ", m_ID); };
	virtual void update(double deltaTime, double time) { EngineLog("Update: ", m_ID); };
	void addComponent(Component* component) { m_Components.push_back(component); }

	//Getters
	unsigned int ID() const { return m_ID; }
	bool active() const { return m_Active; }
	void deactivate() { m_Active = false; }
	float m_XPos = 0.0f;
	float m_YPos = 0.0f;
	float m_ZPos = 0.0f;

protected:
	//ID
	unsigned int m_ID;
	unsigned int m_NextCompID = 0;
	bool m_Active = true;

	//Components - stores pointers to components
	std::vector<Component*> m_Components;

	//Rendering and pos
	Renderer<TextureVertex>* m_Renderer = nullptr;
};

#endif