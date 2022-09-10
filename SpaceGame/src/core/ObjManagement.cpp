#include "core/ObjManagement.h"

//Push to heap of components
//Threads are currently disabled
void SGObject::ObjectManager::update(double deltaTime) {
	//Launch async cleanup task every 10 seconds to check for dead objects
	if (m_CheckCleanupTimer > 10.0)
	{
		EngineLog("Cleaning objects...");
		this->cleanObjects();
		m_CheckCleanupTimer = 0.0;
	}

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
		m_UpdateHeap[i]->processMessages();
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
			m_UpdateHeap[inactiveInd]->updatePointer();
			m_UpdateHeap[activeInd]->updatePointer();
			inactiveInd = -1;
			activeInd = -1;
		}
	}

	m_CheckCleanupTimer += deltaTime;
}

void SGObject::ObjectManager::render() {
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
		m_RenderHeap[i]->processMessages();
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
			m_RenderHeap[inactiveInd]->updatePointer();
			m_RenderHeap[activeInd]->updatePointer();
			inactiveInd = -1;
			activeInd = -1;
		}
	}
}

void SGObject::ObjectManager::cleanObjects()
{
	auto ts = EngineTimer::StartTimer();
	int size = m_Objects.size();
	int cleanedTotal = 0;
	for (int i = 0; i < size; i++)
	{
		if (!m_Objects[i].obj->isDead())
		{
			continue;
		}
		if (m_Objects[i].obj->safeToDelete())
		{
			m_Objects.erase(m_Objects.begin() + i);
			i--;
			size--;
			cleanedTotal++;

			//adjust object id's
			for (int i = 0; i < size; i++)
			{
				m_Objects[i].obj->setID(i);
				m_ObjIDMap[m_Objects[i].name] = i;
			}
		}
	}
	EngineLog("Objects cleaned: ", cleanedTotal);
	EngineLog("Time to clean objects: ", EngineTimer::EndTimer(ts));
}

void SGObject::ObjectManager::reset()
{
	//Clear absolutely everything

	//Clear
	m_UpdateGroup.clear();
	m_RenderGroup.clear();
	m_UpdateHeap.clear();
	m_RenderHeap.clear();
	m_Objects.clear();
	m_GroupIDMap.clear();
	m_ObjIDMap.clear();
}