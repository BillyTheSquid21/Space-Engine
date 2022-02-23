#pragma once
#ifndef OBJ_MANAGEMENT_H
#define OBJ_MANAGEMENT_H

#include "core/GameObject.hpp"
#include <vector>
#include <algorithm>

class ObjectManager
{
public:
	//Push - returns where the group is, to be able to add components at runtime
	unsigned int pushUpdateGroup(std::shared_ptr<UpdateComponentGroup> ptr) { ptr->setID(m_UpdateGroup.size()); m_UpdateGroup.push_back(ptr); return m_UpdateGroup.size() - 1; };
	unsigned int pushRenderGroup(std::shared_ptr<RenderComponentGroup> ptr) { ptr->setID(m_RenderGroup.size()); m_RenderGroup.push_back(ptr); return m_RenderGroup.size() - 1; };
	unsigned int pushUpdateHeap(std::shared_ptr<UpdateComponent> ptr) { m_UpdateHeap.push_back(ptr); return m_UpdateHeap.size() - 1; }
	unsigned int pushRenderHeap(std::shared_ptr<RenderComponent> ptr) { m_RenderHeap.push_back(ptr); return m_RenderHeap.size() - 1; }
	unsigned int pushGameObject(std::shared_ptr<GameObject> ptr) { m_Objects.push_back(ptr); return m_Objects.size() - 1; }

	void update(double deltaTime);
	void render();

	std::shared_ptr<UpdateComponentGroup> getUpdateGroup(unsigned int id) const { return m_UpdateGroup[id]; };
	std::shared_ptr<RenderComponentGroup> getRenderGroup(unsigned int id) const { return m_RenderGroup[id]; };
private:
	//Keep separate to allow to be called at different times
	std::vector<std::shared_ptr<UpdateComponentGroup>> m_UpdateGroup;
	std::vector<std::shared_ptr<RenderComponentGroup>> m_RenderGroup;

	//Heap for when you don't need to warrant grouping - simpler but results in unordered behavour
	std::vector<std::shared_ptr<UpdateComponent>> m_UpdateHeap;
	std::vector<std::shared_ptr<RenderComponent>> m_RenderHeap;

	//Storage for Game objects - object only removed after components deleted
	//TODO - make game objects check if all components are null in other thread
	//Then lock vector and remove
	std::vector<std::shared_ptr<GameObject>> m_Objects;
};

#endif