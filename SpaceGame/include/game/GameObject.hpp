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
	virtual ~GameObject() { if (m_Components != NULL) { for (int i = 0; i < m_ComponentCount; i++) { delete m_Components[i]; } delete[] m_Components; } }

	void setRenderer(Renderer<Vertex>* ren) { m_Renderer = ren; };
	virtual void render() { EngineLog("Render: ", m_ID); };
	virtual void update(double deltaTime, double time) { EngineLog("Update: ", m_ID); };

	//Getters
	float xPos() const { return m_XPos; }
	float yPos() const { return m_YPos; }
	unsigned int ID() const { return m_ID; }
	bool active() const { return m_Active; }
	void deactivate() { m_Active = false; }

protected:
	//ID
	unsigned int m_ID;
	unsigned int m_NextCompID = 0;
	bool m_Active = true;

	//Components - stores pointers to components
	Component** m_Components = nullptr;
	unsigned char m_ComponentCount = 0;
	unsigned int m_ComponentBytes = 0;

	//Rendering and pos
	Renderer<Vertex>* m_Renderer = nullptr;
	float m_XPos = 0.0f;
	float m_YPos = 0.0f;
};

//Example derived comp and object
class DerivedComponent : public Component
{
public:
	void setXY(float* xPos, float* yPos, Quad* q) { m_X = xPos; m_Y = yPos; quad = q; }
	void function(double deltaTime, double time) { TranslateShape((Vertex*)quad, 1.0f * deltaTime, 0.0f, 0.0f, Shape::QUAD); }
	float* m_X; float* m_Y; Quad* quad;
};

class DerivedObject : public GameObject
{
public:
	void generate(float size) { quad = CreateQuad(0.0f, 0.0f, size, size, 0.0f, 0.0f, 1.0f, 1.0f);
		m_ComponentCount = 1;
		m_Components = new Component*[m_ComponentCount];
		DerivedComponent* comp = new DerivedComponent();
		comp->setXY(&m_XPos, &m_YPos, &quad);
		Component* compBase = (Component*)comp;
		compBase->setID(0);
		m_Components[0] = compBase;
	};
	void render() { m_Renderer->commit((Vertex*)&quad, GetFloatCount(Shape::QUAD), Primitive::Q_IND, Primitive::Q_IND_COUNT); }
	void update(double deltaTime, double time) { EngineLog("Derived Update: ", m_ID); if (time > 15.0) { deactivate(); } for (int i = 0; i < m_ComponentCount; i++) { m_Components[i]->function(deltaTime, time); } }
	Quad quad;
};





#endif