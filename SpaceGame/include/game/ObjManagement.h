#pragma once
#ifndef OBJ_MANAGEMENT_H
#define OBJ_MANAGEMENT_H

#include "game/GameObject.hpp"

class ObjectManager
{
public:
	//destroys objects when unloaded
	~ObjectManager() { if (m_Objects != NULL) { for (int i = 0; i < m_ObjectCount; i++) { if (m_Objects[i] != NULL) { delete m_Objects[i]; } } delete[] m_Objects; } }


private:
	//Objects - stores pointers to objects
	GameObject** m_Objects;
	unsigned char m_ObjectCount;
	unsigned int m_ObjectBytes;
};

#endif