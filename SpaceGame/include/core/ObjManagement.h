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

namespace SGObject
{
	/**
	* Container to manage all objects in a scene - will improve ID allocation in future
	* Contains render and update groups, as well as a render and update heap for ungrouped components
	* Heap components and game objects must be std::shared_ptr objects
	* Also contains mutexes for different data to ensure threaded capabilities
	*/
	class ObjectManager
	{
	public:
		/**
		* Push an update group to the manager
		*/
		template<typename T>
		unsigned int pushUpdateGroup(T ptr) { ptr->setID(m_UpdateGroup.size()); m_UpdateGroup.push_back(std::static_pointer_cast<SGObject::UpdateGroupBase>(ptr)); return m_UpdateGroup.size() - 1; };
		/**
		* Push a render group to the manager
		*/
		template<typename T>
		unsigned int pushRenderGroup(T ptr) { ptr->setID(m_RenderGroup.size()); m_RenderGroup.push_back(std::static_pointer_cast<SGObject::RenderGroupBase>(ptr)); return m_RenderGroup.size() - 1; };
		/**
		* Push an update group to the manager with a name to identify it
		*/
		template<typename T>
		unsigned int pushUpdateGroup(T ptr, std::string groupName) { m_GroupIDMap[groupName] = m_UpdateGroup.size(); return pushUpdateGroup(std::static_pointer_cast<SGObject::UpdateGroupBase>(ptr)); };
		/**
		* Push a render group to the manager with a name to identify it
		*/
		template<typename T>
		unsigned int pushRenderGroup(T ptr, std::string groupName) { m_GroupIDMap[groupName] = m_RenderGroup.size(); return pushRenderGroup(std::static_pointer_cast<SGObject::RenderGroupBase>(ptr)); };

		/**
		* Push an update component to the update heap
		*/
		template<typename T>
		unsigned int pushUpdateHeap(T ptr, std::vector<SGObject::UpdateComponent*>* compPointers)
		{
			std::static_pointer_cast<SGObject::UpdateComponent>(ptr);
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

		/**
		* Push a render component to the render heap
		*/
		template<typename T>
		unsigned int pushRenderHeap(T ptr, std::vector<SGObject::RenderComponent*>* compPointers)
		{
			std::static_pointer_cast<SGObject::RenderComponent>(ptr);
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

		/**
		* Push a game object
		*/
		template<typename T>
		unsigned int pushGameObject(T ptr) { m_Objects.push_back({ std::static_pointer_cast<SGObject::GObject>(ptr), "" }); return m_Objects.size() - 1; }
		/**
		* Push a game object with a name to identify
		*/
		template<typename T>
		unsigned int pushGameObject(T ptr, std::string name) { m_Objects.push_back({ std::static_pointer_cast<SGObject::GObject>(ptr), name }); m_ObjIDMap[name] = m_Objects.size() - 1; return m_Objects.size() - 1; }

		/**
		* Update components
		*/
		void update(double deltaTime);
		/**
		* Render components
		*/
		void render();

		//Always request to lock access via pointers before starting
		//Lock for scope of function called in INCLUDING SCOPE OF LATER CALLED FUNCTIONS
		std::shared_mutex& getGroupMutex() { return m_GroupMutex; }
		std::shared_mutex& getHeapMutex() { return m_HeapMutex; }
		template <typename ExpectedT>
		std::shared_ptr<SGObject::UpdateCompGroup<ExpectedT>> updateGroupAt(unsigned int id) const { return std::static_pointer_cast<SGObject::UpdateCompGroup<ExpectedT>>(m_UpdateGroup[id]); };
		template <typename ExpectedT>
		std::shared_ptr<SGObject::RenderCompGroup<ExpectedT>> renderGroupAt(unsigned int id) const { return std::static_pointer_cast<SGObject::RenderCompGroup<ExpectedT>>(m_RenderGroup[id]); };
		std::shared_mutex& getObjectMutex() { return m_ObjMutex; }
		template <typename ExpectedT>
		std::shared_ptr<ExpectedT> objectAt(unsigned int id) const { return std::static_pointer_cast<ExpectedT>(m_Objects[id].obj); }

		//Should also be locked in some capacity when accessed as can change even if not here
		unsigned int queryGroupID(std::string name) { if (m_GroupIDMap.find(name) == m_GroupIDMap.end()) { return m_GroupIDMap.begin()->second; } return m_GroupIDMap[name]; }
		unsigned int queryObjectID(std::string name) { if (m_ObjIDMap.find(name) == m_ObjIDMap.end()) { return m_ObjIDMap.begin()->second; } return m_ObjIDMap[name]; }

		/**
		* Remove an object based on its index
		*/
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
		/**
		* Remove an object based on its name
		*/
		void removeObject(std::string name) { if (m_ObjIDMap.find(name) == m_ObjIDMap.end()) { EngineLog("Object does not exist: ", name); return; } unsigned int id = m_ObjIDMap[name]; if (m_Objects[id].obj->isDead()) { return; } m_Objects[id].obj->messageAll((uint32_t)Message::KILL); }

		//Storage for Game objects - object only removed after components deleted
		struct GameObjectContainer
		{
			std::shared_ptr<SGObject::GObject> obj;
			std::string name;
		};

		std::vector<GameObjectContainer>& getObjects() { return m_Objects; }
		unsigned int getObjectCount() { return m_Objects.size(); }

		/**
		* Reset object manager and remove all data
		*/
		void reset();

	private:
		//Keep separate to allow to be called at different times
		std::shared_mutex m_GroupMutex;
		std::vector<std::shared_ptr<SGObject::UpdateGroupBase>> m_UpdateGroup;
		std::vector<std::shared_ptr<SGObject::RenderGroupBase>> m_RenderGroup;

		//Heap for when you don't need to warrant grouping - simpler but results in unordered behavour
		std::shared_mutex m_HeapMutex;
		std::vector<std::shared_ptr<SGObject::UpdateComponent>> m_UpdateHeap;
		std::vector<std::shared_ptr<SGObject::RenderComponent>> m_RenderHeap;

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
}

#endif