#include "core/ObjManagement.h"

void ObjectManager::update(double deltaTime) {
	for (int i = 0; i < m_UpdateGroup.size(); i++) {
		m_UpdateGroup[i]->iterate(deltaTime);
	}
	//Line must be organised here
	for (int i = 0; i < m_UpdateHeap.size(); i++) {
		m_UpdateHeap[i]->update(deltaTime);
	}
}

void ObjectManager::render() {
	for (int i = 0; i < m_RenderGroup.size(); i++) {
		m_RenderGroup[i]->iterate();
	}
	//Line must be organised here - TODO implement making active contiguous
	for (int i = 0; i < m_RenderHeap.size(); i++) {
		m_RenderHeap[i]->render();
	}
}