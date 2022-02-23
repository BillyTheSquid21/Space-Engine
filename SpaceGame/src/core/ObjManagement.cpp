#include "core/ObjManagement.h"

void ObjectManager::update(double deltaTime) {
	for (unsigned int i = 0; i < m_UpdateGroup.size(); i++) {
		m_UpdateGroup[i]->iterate(deltaTime);
	}
	//Iterate heap
	//Line must be organised here
	//If a pair is found both will not be -1, and can be swapped
	int inactiveInd = -1;
	int activeInd = -1;

	//Bools to start checking
	bool inactiveFound = false;
	bool activeAfterInactive = false;

	for (unsigned int i = 0; i < m_UpdateHeap.size(); i++) {
		if (m_UpdateHeap[i]->isActive()) {
			m_UpdateHeap[i]->update(deltaTime);
			if (inactiveFound) {
				if (!activeAfterInactive) {
					activeAfterInactive = true;
					activeInd = i;
				}
				else {
					activeInd = i;
				}
			}
		}
		else if (!inactiveFound) {
			inactiveInd = i;
			inactiveFound = true;
		}
		else if (inactiveFound && activeAfterInactive) {
			inactiveInd = i;
		}
		if (inactiveInd != -1 && activeInd != -1) {
			std::iter_swap(m_UpdateHeap.begin() + inactiveInd, m_UpdateHeap.begin() + activeInd);
			inactiveInd = -1;
			activeInd = -1;
		}
	}
}

void ObjectManager::render() {
	for (int i = 0; i < m_RenderGroup.size(); i++) {
		m_RenderGroup[i]->iterate();
	}
	//Iterate heap
	//Line must be organised here
	//If a pair is found both will not be -1, and can be swapped
	int inactiveInd = -1;
	int activeInd = -1;

	//Bools to start checking
	bool inactiveFound = false;
	bool activeAfterInactive = false;

	for (unsigned int i = 0; i < m_RenderHeap.size(); i++) {
		if (m_RenderHeap[i]->isActive()) {
			m_RenderHeap[i]->render();
			if (inactiveFound) {
				if (!activeAfterInactive) {
					activeAfterInactive = true;
					activeInd = i;
				}
				else {
					activeInd = i;
				}
			}
		}
		else if (!inactiveFound) {
			inactiveInd = i;
			inactiveFound = true;
		}
		else if (inactiveFound && activeAfterInactive) {
			inactiveInd = i;
		}
		if (inactiveInd != -1 && activeInd != -1) {
			std::iter_swap(m_RenderHeap.begin() + inactiveInd, m_RenderHeap.begin() + activeInd);
			inactiveInd = -1;
			activeInd = -1;
		}
	}
}