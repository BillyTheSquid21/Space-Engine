#pragma once
#ifndef OBJ_MANAGEMENT_H
#define OBJ_MANAGEMENT_H

#include "core/GameObject.hpp"
#include <vector>
#include <algorithm>
#include <unordered_map>

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
		ptr->attachToObject(compPointers);
		std::static_pointer_cast<UpdateComponent>(ptr);

		//Check if any dead components to replace
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
		ptr->attachToObject(compPointers);
		std::static_pointer_cast<RenderComponent>(ptr);

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
	unsigned int pushGameObject(T ptr) { m_Objects.push_back(std::static_pointer_cast<GameObject>(ptr)); return m_Objects.size() - 1; }

	void update(double deltaTime);
	void render();
	
	template <typename ExpectedT>
	std::shared_ptr<UpdateComponentGroup<ExpectedT>> updateGroup(unsigned int id) const { return m_UpdateGroup[id]; };
	std::shared_ptr<RenderGroupBase> renderGroup(unsigned int id) const { return m_RenderGroup[id]; };

	unsigned int queryGroupID(std::string name) { if (m_GroupIDMap.find(name) == m_GroupIDMap.end()) { return m_GroupIDMap.begin()->second; } return m_GroupIDMap[name]; }

private:
	//Keep separate to allow to be called at different times
	std::vector<std::shared_ptr<UpdateGroupBase>> m_UpdateGroup;
	std::vector<std::shared_ptr<RenderGroupBase>> m_RenderGroup;

	//Heap for when you don't need to warrant grouping - simpler but results in unordered behavour
	std::vector<std::shared_ptr<UpdateComponent>> m_UpdateHeap;
	std::vector<std::shared_ptr<RenderComponent>> m_RenderHeap;

	//Storage for Game objects - object only removed after components deleted
	//TODO - make game objects check if all components are null in other thread
	//Then lock vector and remove
	std::vector<std::shared_ptr<GameObject>> m_Objects;
	std::unordered_map<std::string, unsigned int> m_GroupIDMap;
};

#endif