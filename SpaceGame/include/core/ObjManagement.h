#pragma once
#ifndef OBJ_MANAGEMENT_H
#define OBJ_MANAGEMENT_H

#include "core/GameObject.hpp"
#include <vector>

class ObjectManager
{
public:
	//destroys objects when unloaded
	~ObjectManager() { for (int i = 0; i < m_Objects.size(); i++) { if (m_Objects[i] != NULL) { delete m_Objects[i]; } } }
	void loadObject(GameObject* obj);
	void unloadObject(unsigned int objID);
	void update(double deltaTime, double time);
	void render();

private:
	//Objects - stores pointers to objects
	std::vector<GameObject*> m_Objects;
};

#endif