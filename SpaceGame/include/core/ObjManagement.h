#pragma once
#ifndef OBJ_MANAGEMENT_H
#define OBJ_MANAGEMENT_H

#include "core/GameObject.hpp"
#include "vector"
#include "algorithm"
#include "unordered_map"
#include "map"
#include "future"
#include "shared_mutex"

class ObjectManager
{
public:
	//Push - returns where the group is, to be able to add components at runtime
	template<typename T>
	unsigned int pushUpdateGroup(T ptr) { ptr->setID(m_UpdateGroup.size()); m_UpdateGroup.push_back(std::static_pointer_cast<UpdateGroupBase>(ptr)); return m_UpdateGroup.size() - 1; };
	template<typename T>
	unsigned int pushRenderGroup(T ptr) { ptr->setID(m_RenderGroup.size()); m_RenderGroup.push_back(std::static_pointer_cast<RenderGroupBase>(ptr)); return m_RenderGroup.size() - 1; };
	//Overload to allow querying id from given name
	template<typename T>
	unsigned int pushUpdateGroup(T ptr, std::string groupName) { m_GroupIDMap[groupName] = m_UpdateGroup.size(); return pushUpdateGroup(std::static_pointer_cast<UpdateGroupBase>(ptr)); };
	template<typename T>
	unsigned int pushRenderGroup(T ptr, std::string groupName) { m_GroupIDMap[groupName] = m_RenderGroup.size(); return pushRenderGroup(std::static_pointer_cast<RenderGroupBase>(ptr)); };
	
	template<typename T>
	unsigned int pushUpdateHeap(T ptr, std::vector<UpdateComponent*>* compPointers)
	{
		std::static_pointer_cast<UpdateComponent>(ptr);
		ptr->attachToObject(compPointers);
		ptr->updatePointer();

		//Check if any dead components to replace - TODO make work proprly
		for (int i = 0; i < m_UpdateHeap.size(); i++)
		{
			if (m_UpdateHeap[i]->isDead())
			{
				m_UpdateHeap[i] = ptr;
				return i;
			}
		}

		m_UpdateHeap.push_back(ptr);
		return m_UpdateHeap.size() - 1;
	}

	template<typename T>
	unsigned int pushRenderHeap(T ptr, std::vector<RenderComponent*>* compPointers)
	{
		std::static_pointer_cast<RenderComponent>(ptr);
		ptr->attachToObject(compPointers);
		ptr->updatePointer();

		//Check if any dead components to replace
		for (int i = 0; i < m_RenderHeap.size(); i++)
		{
			if (m_RenderHeap[i]->isDead())
			{
				m_RenderHeap[i] = ptr;
				return i;
			}
		}

		m_RenderHeap.push_back(ptr);
		return m_RenderHeap.size() - 1;
	}

	template<typename T>
	unsigned int pushGameObject(T ptr, std::string name) { m_Objects.push_back({ std::static_pointer_cast<GameObject>(ptr), name }); m_ObjIDMap[name] = m_Objects.size() - 1; return m_Objects.size() - 1; }
	template<typename T>
	unsigned int pushGameObject(T ptr) { m_Objects.push_back({ std::static_pointer_cast<GameObject>(ptr), "" }); return m_Objects.size() - 1;}

	void update(double deltaTime);
	void render();
	
	//Always request to lock access via pointers before starting
	//Lock for scope of function called in INCLUDING SCOPE OF LATER CALLED FUNCTIONS
	std::shared_mutex& getGroupMutex() { return m_GroupMutex; }
	std::shared_mutex& getHeapMutex() { return m_HeapMutex; }
	template <typename ExpectedT>
	std::shared_ptr<UpdateComponentGroup<ExpectedT>> updateGroupAt(unsigned int id) const { return std::static_pointer_cast<UpdateComponentGroup<ExpectedT>>(m_UpdateGroup[id]); };
	template <typename ExpectedT>
	std::shared_ptr<RenderComponentGroup<ExpectedT>> renderGroupAt(unsigned int id) const { return std::static_pointer_cast<RenderComponentGroup<ExpectedT>>(m_RenderGroup[id]); };
	std::shared_mutex& getObjectMutex() { return m_ObjMutex; }
	template <typename ExpectedT>
	std::shared_ptr<ExpectedT> objectAt(unsigned int id) const { return std::static_pointer_cast<ExpectedT>(m_Objects[id].obj); }

	//Should also be locked in some capacity when accessed as can change even if not here
	unsigned int queryGroupID(std::string name) { if (m_GroupIDMap.find(name) == m_GroupIDMap.end()) { return m_GroupIDMap.begin()->second; } return m_GroupIDMap[name]; }
	unsigned int queryObjectID(std::string name) {  if (m_ObjIDMap.find(name) == m_ObjIDMap.end()) { return m_ObjIDMap.begin()->second; } return m_ObjIDMap[name]; }
	
	//Remove objects
	void removeObject(unsigned int id)
	{
		if (id >= m_Objects.size())
		{
			EngineLog("Object does not exist: ", id);
			return;
		}
		if (m_Objects[id].obj->isDead())
		{
			return;
		}
		m_Objects[id].obj->messageAll((uint32_t)Message::KILL);
	};
	void removeObject(std::string name) { if (m_ObjIDMap.find(name) == m_ObjIDMap.end()) { EngineLog("Object does not exist: ", name); return; } unsigned int id = m_ObjIDMap[name]; if (m_Objects[id].obj->isDead()) { return; } m_Objects[id].obj->messageAll((uint32_t)Message::KILL);}
	
	//Storage for Game objects - object only removed after components deleted
	struct GameObjectContainer
	{
		std::shared_ptr<GameObject> obj;
		std::string name;
	};

	std::vector<GameObjectContainer>& getObjects() { return m_Objects; }
	unsigned int getObjectCount() { return m_Objects.size(); }

	void reset();
 
private:
	//Keep separate to allow to be called at different times
	std::shared_mutex m_GroupMutex;
	std::vector<std::shared_ptr<UpdateGroupBase>> m_UpdateGroup;
	std::vector<std::shared_ptr<RenderGroupBase>> m_RenderGroup;

	//Heap for when you don't need to warrant grouping - simpler but results in unordered behavour
	std::shared_mutex m_HeapMutex;
	std::vector<std::shared_ptr<UpdateComponent>> m_UpdateHeap;
	std::vector<std::shared_ptr<RenderComponent>> m_RenderHeap;

	//Game object vector
	std::shared_mutex m_ObjMutex; //used to lock access of object and ids as objects are removed in other thread
	std::vector<GameObjectContainer> m_Objects;

	//Id maps
	std::unordered_map<std::string, unsigned int> m_GroupIDMap;
	std::unordered_map<std::string, unsigned int> m_ObjIDMap;

	//Check if objects can be removed - carried out async
	void cleanObjects();
	std::future<void> m_CleanFtr;
	double m_CheckCleanupTimer = 0.0;
};

#endif