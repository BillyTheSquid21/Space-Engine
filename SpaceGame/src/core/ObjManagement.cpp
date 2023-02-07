#include "core/ObjManagement.h"

std::shared_mutex SGObject::System::m_GroupMutex;
std::vector<std::shared_ptr<SGObject::UpdateGroupBase>> SGObject::System::m_UpdateGroup;
std::vector<std::shared_ptr<SGObject::RenderGroupBase>> SGObject::System::m_RenderGroup;
std::shared_mutex SGObject::System::m_HeapMutex;
std::vector<std::shared_ptr<SGObject::UpdateComponent>> SGObject::System::m_UpdateHeap;
std::vector<std::shared_ptr<SGObject::RenderComponent>> SGObject::System::m_RenderHeap;
std::shared_mutex SGObject::System::m_ObjMutex;
std::vector<SGObject::System::GameObjectContainer> SGObject::System::m_Objects;
std::unique_ptr<std::unordered_map<std::string, unsigned int>> SGObject::System::m_GroupIDMap;
std::unique_ptr<std::unordered_map<std::string, unsigned int>> SGObject::System::m_ObjIDMap;
std::future<void> SGObject::System::m_CleanFtr;
double SGObject::System::m_CheckCleanupTimer = 0.0;
bool SGObject::System::s_Set = false;

bool SGObject::System::init()
{
	EngineLogOk("Object System");
	return true;
}

void SGObject::System::set()
{
	if (s_Set)
	{
		return;
	}

	clean();
	m_GroupIDMap = std::unique_ptr<std::unordered_map<std::string, unsigned int>>(new std::unordered_map<std::string, unsigned int>());
	m_ObjIDMap = std::unique_ptr<std::unordered_map<std::string, unsigned int>>(new std::unordered_map<std::string, unsigned int>());
	s_Set = true;
}

//Push to heap of components
//Threads are currently disabled
void SGObject::System::update(double deltaTime) {
	
	if (!s_Set)
	{
		return;
	}
	
	//Launch async cleanup task every 10 seconds to check for dead objects
	if (m_CheckCleanupTimer > 10.0)
	{
		EngineLog("Cleaning objects...");
		cleanObjects();
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

void SGObject::System::render() {

	if (!s_Set)
	{
		return;
	}

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

void SGObject::System::cleanObjects()
{
	auto ts = EngineTimer::StartTimer();
	int size = m_Objects.size();
	int cleanedTotal = 0;
	for (int i = 0; i < size; i++)
	{
		if (m_Objects[i].obj->isAlive())
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
				(*m_ObjIDMap)[m_Objects[i].name] = i;
			}
		}
	}
	EngineLog("Objects cleaned: ", cleanedTotal);
	EngineLog("Time to clean objects: ", EngineTimer::EndTimer(ts));
}

void SGObject::System::clean()
{
	if (!s_Set)
	{
		return;
	}

	//Clear
	m_UpdateGroup.clear();
	m_RenderGroup.clear();
	m_UpdateHeap.clear();
	m_RenderHeap.clear();
	m_Objects.clear();
	m_GroupIDMap.reset();
	m_ObjIDMap.reset();

	s_Set = false;
}