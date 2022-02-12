#include "core/ObjManagement.h"

void ObjectManager::loadObject(GameObject* obj) {
	m_Objects.push_back(obj);
}

void ObjectManager::unloadObject(unsigned int objID) {
	for (int i = 0; i < m_Objects.size(); i++) {
		if (m_Objects[i]->ID() == objID) {
			m_Objects.erase(m_Objects.begin() + i);
			return;
		}
	}
	EngineLog("Object was not found");
}

void ObjectManager::update(double deltaTime, double time) {
	for (int i = 0; i < m_Objects.size(); i++) {
		if (!m_Objects[i]->active()) {
			m_Objects.erase(m_Objects.begin() + i);
			i--;
			continue;
		}
		m_Objects[i]->update(deltaTime, time);
	}
}

void ObjectManager::render() {
	for (int i = 0; i < m_Objects.size(); i++) {
		m_Objects[i]->render();
	}
}