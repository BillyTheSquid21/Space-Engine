#pragma once
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "stdint.h"
#include "random"
#include "queue"
#include "renderer/Renderer.hpp"
#include "core/Message.hpp"

//1.System is set up to keep similar components contiguous
//2.Components are put in a group where they will be contiguous
//3.Pointers to groups will be kept in a vector by the manager, and iterated over
//4.Objects will store a vector of pointers to their components, and components a pointer to the vector to be able to update
//	4.a.Component id's are their location in parent array - will not change in runtime
//	4.b.Group id's are their location in object manager array - will not change in runtime
//	4.c.TLDR: Ids should not change in runtime, index's and pointers may
//5.Objects will contain shared data the components can make use of and a messaging system
//6.Objects are not accessed by object manager often
//7.Object manager should keep active components contiguous to increase cache hits

//IMPORTANT - Remember to create the group ptrs as new so they remain in scope

//Declare function pointers for sending messages if needed
//If a component wants access, pass and store the function pointer

//Error handling
enum class ObjectError
{
	VOID_POINTER, INDEX_RANGE, UNPAIRED_COMPONENT,
};

static void ObjectErrorLog(ObjectError errorcode, int additionalInfo)
{
	EngineLog("OBJECT ERROR:");
	switch (errorcode)
	{
	case ObjectError::VOID_POINTER:
		EngineLog("Object or component tried to access NULLPTR. Error Code: ", (int)errorcode);
		EngineLog("Component ID: ", additionalInfo);
		return;
	case ObjectError::INDEX_RANGE:
		EngineLog("Component tried to access index out of range. Error Code: ", (int)errorcode);
		EngineLog("Component ID: ", additionalInfo);
		return;
	case ObjectError::UNPAIRED_COMPONENT:
		EngineLog("Component is not paired to an object! Error Code: ", (int)errorcode);
		return;
	default:
		EngineLog("An Error has ocurred. Error Code: ", (int)errorcode);
		return;
	}
}


template<typename T>
class Component
{
public:
	//Function component does will carry out
	Component() {}
	void recieve(uint32_t message) { m_MessageQueue.push(message); };
	bool isActive() const { return m_Active; }
	void setActive(bool set) { m_Active = set; }
	bool isDead() const { return m_Dead; }
	//Kills component which ensures is also inactive. They will be sorted to end and before pushing the vector back, a new object will check if can replace dead comp
	void kill() { m_Dead = true; m_ParentPointers = nullptr; }

	//By default only checks for deactivate and activate message - can be overidden - ACTIVATE AND DEACTIVATE MUST BE REIMPLEMENTED
	virtual void processMessages() {
		while (m_MessageQueue.size() > 0) {
			uint32_t message = m_MessageQueue.front();
			if (message == (uint32_t)Message::ACTIVATE) { setActive(true); }
			else if (message == (uint32_t)Message::DEACTIVATE) { setActive(false); }
			else if (message == (uint32_t)Message::KILL) { setActive(false); 
			} 
			m_MessageQueue.pop();
		};
	}

	//Attach to parent pointers - called when added to object manager and in its place
	//During runtime will have to pass pointer all the way down
	void attachToObject(std::vector<T*>* compPointers) { m_ParentPointers = compPointers; this->setID(m_ParentPointers->size()); m_ParentPointers->push_back((T*)this); };
	//Call after moving - if resizing whole vector call for each element
	void updatePointer() 
	{ 
		//Needs to return if dead as the object it is tied to may no longer exist in memory at all
		if (m_Dead)
		{
			return;
		}
		if (!m_ParentPointers)
		{
			ObjectErrorLog(ObjectError::UNPAIRED_COMPONENT, 0);
			return;
		}
		if (m_ID >= m_ParentPointers->size()) 
		{ 
			ObjectErrorLog(ObjectError::INDEX_RANGE, m_ID); 
			return; 
		} 
		if (!(*m_ParentPointers)[m_ID]) 
		{ 
			ObjectErrorLog(ObjectError::VOID_POINTER, m_ID); 
			return; 
		} 
		(*m_ParentPointers)[m_ID] = (T*)this; 
	}
	//Id is location inside parent array
	void setID(uint16_t id) { m_ID = id; }
	uint16_t id() const { return m_ID; }

protected:
	uint16_t m_ID = 0;
	bool m_Active = true;
	bool m_Dead = false;
	std::vector<T*>* m_ParentPointers = nullptr;
	std::queue<uint32_t> m_MessageQueue;
};

class RenderComponent : public Component<RenderComponent>
{
public:
	//Will carry out render instructions
	virtual void render() {};
};

class UpdateComponent : public Component<UpdateComponent>
{
public:
	//Will carry out update instructions
	virtual void update(double deltaTime) {};
};

//Group iteration
template<typename T>
void IterateRenderComps(std::vector<T>& renderComps) {
	
	static_assert(std::is_base_of<RenderComponent, T>::value, "T must inherit from RenderComponent, did you mean to use IterateUpdateComps()?");
	//Line must be organised here
	//If a pair is found both will not be -1, and can be swapped
	int inactiveInd = -1;
	int activeInd = -1;

	//Bools to start checking
	bool inactiveFound = false;
	bool activeAfterInactive = false;

	for (unsigned int i = 0; i < renderComps.size(); i++) {
		//Process messages
		renderComps[i].processMessages();
		if (renderComps[i].isActive()) {
			renderComps[i].render();
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
			std::iter_swap(renderComps.begin() + inactiveInd, renderComps.begin() + activeInd);
			renderComps[inactiveInd].updatePointer();
			renderComps[activeInd].updatePointer();
			inactiveInd = -1;
			activeInd = -1;
		}
	}
}

template<typename T>
void IterateUpdateComps(std::vector<T>& updateComps, double deltaTime) {
	
	static_assert(std::is_base_of<UpdateComponent, T>::value, "ERROR: T must inherit from UpdateComponent, did you mean to use IterateUpdateComps()?");
	//Line must be organised here
	//If a pair is found both will not be -1, and can be swapped
	int inactiveInd = -1;
	int activeInd = -1;

	//Bools to start checking
	bool inactiveFound = false;
	bool activeAfterInactive = false;

	for (unsigned int i = 0; i < updateComps.size(); i++) {
		updateComps[i].processMessages();
		if (updateComps[i].isActive()) {
			updateComps[i].update(deltaTime);
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
			std::iter_swap(updateComps.begin() + inactiveInd, updateComps.begin() + activeInd);
			updateComps[inactiveInd].updatePointer();
			updateComps[activeInd].updatePointer();
			inactiveInd = -1;
			activeInd = -1;
		}
	}
}

//Base form of containers that iterates through contiguous identical-type components
class UpdateGroupBase
{
public:
	UpdateGroupBase() {}
	void setID(unsigned int id) { m_ID = id; }
	virtual void iterate(double deltaTime) {};
private:
	unsigned int m_ID = 0;
};

template <typename T>
class UpdateComponentGroup : public UpdateGroupBase
{
public:
	using UpdateGroupBase::UpdateGroupBase;
	void iterate(double deltaTime) { IterateUpdateComps<T>(m_Components, deltaTime); };
	template<typename... Args>
	void addComponent(std::vector<UpdateComponent*>* compPointers, Args... args)
	{
		//Check if any dead
		for (int i = 0; i < m_Components.size(); i++)
		{
			if (m_Components[i].isDead())
			{
				T obj = T(args...);
				m_Components[i] = obj;
				m_Components[i].attachToObject(compPointers);
				m_Components[i].updatePointer();
				return;
			}
		}

		//Resize components array
		m_Components.emplace_back(args...);
		m_Components[m_Components.size() - 1].attachToObject(compPointers);
		for (int i = 0; i < m_Components.size(); i++)
		{
			m_Components[i].updatePointer();
		}
	}
	void addExistingComponent(std::vector<UpdateComponent*>* compPointers, T obj)
	{
		//Check if any dead
		for (int i = 0; i < m_Components.size(); i++)
		{
			if (m_Components[i].isDead())
			{
				m_Components[i] = obj;
				m_Components[i].attachToObject(compPointers);
				m_Components[i].updatePointer();
				return;
			}
		}

		//Resize components array
		m_Components.push_back(obj);
		m_Components[m_Components.size() - 1].attachToObject(compPointers);
		for (int i = 0; i < m_Components.size(); i++)
		{
			m_Components[i].updatePointer();
		}
	}
private:
	unsigned int m_ID = 0;
	std::vector<T> m_Components;
};

class RenderGroupBase
{
public:
	RenderGroupBase() {}
	void setID(unsigned int id) { m_ID = id; }
	virtual void iterate() {};
private:
	unsigned int m_ID = 0;
};

template <typename T>
class RenderComponentGroup : public RenderGroupBase
{
public:
	using RenderGroupBase::RenderGroupBase;
	void iterate() { IterateRenderComps<T>(m_Components); };
	template<typename... Args>
	void addComponent(std::vector<RenderComponent*>* compPointers, Args... args)
	{
		//Check if any dead
		for (int i = 0; i < m_Components.size(); i++)
		{
			if (m_Components[i].isDead())
			{
				T obj = T(args...);
				m_Components[i] = obj;
				m_Components[i].attachToObject(compPointers);
				m_Components[i].updatePointer();
				return;
			}
		}

		//Resize components array
		m_Components.emplace_back(args...);
		m_Components[m_Components.size() - 1].attachToObject(compPointers);
		for (int i = 0; i < m_Components.size(); i++)
		{
			m_Components[i].updatePointer();
		}
	}
	void addExistingComponent(std::vector<RenderComponent*>* compPointers, T obj)
	{
		//Check if any dead
		for (int i = 0; i < m_Components.size(); i++)
		{
			if (m_Components[i].isDead())
			{
				m_Components[i] = obj;
				m_Components[i].attachToObject(compPointers);
				m_Components[i].updatePointer();
				return;
			}
		}

		//Resize components array
		m_Components.push_back(obj);
		m_Components[m_Components.size() - 1].attachToObject(compPointers);
		for (int i = 0; i < m_Components.size(); i++)
		{
			m_Components[i].updatePointer();
		}
	}
private:
	unsigned int m_ID = 0;
	std::vector<T> m_Components;
};

class GameObject
{
public:
	void setID(unsigned int id) { m_ID = id; }
	unsigned int id() const { return m_ID; }

	//Messaging functions
	virtual void messageUpdateAt(uint32_t message, unsigned int id)
	{
		if (!m_UpdateComps[id]) { ObjectErrorLog(ObjectError::VOID_POINTER, id); return; }
		m_UpdateComps[id]->recieve(message);
	};
	
	virtual void messageRenderAt(uint32_t message, unsigned int id)
	{
		if (!m_RenderComps[id]) { ObjectErrorLog(ObjectError::VOID_POINTER, id); return; }
		m_RenderComps[id]->recieve(message);
	};
	
	virtual void messageAllUpdate(uint32_t message) { for (int i = 0; i < m_UpdateComps.size(); i++) { messageUpdateAt(message, i); } };
	virtual void messageAllRender(uint32_t message) { for (int i = 0; i < m_RenderComps.size(); i++) { messageRenderAt(message, i); } };
	virtual void messageAllExceptUpdate(uint32_t message, unsigned int id) { for (int i = 0; i < m_UpdateComps.size(); i++) { if (id != i) { messageUpdateAt(message, i); } } };
	virtual void messageAllExceptRender(uint32_t message, unsigned int id) { for (int i = 0; i < m_RenderComps.size(); i++) { if (id != i) { messageRenderAt(message, i); } } };

	//If says kill, kill all components then mark dead
	virtual void messageAll(uint32_t message) { messageAllRender(message); messageAllUpdate(message);
	if (message == (uint32_t)Message::KILL) { m_Dead = true; }};

	//Checks all are dead
	bool safeToDelete() {
		for (int i = 0; i < m_UpdateComps.size(); i++) 
		{ 
			if (!m_UpdateComps[i]) 
			{ 
				continue; 
			} 
			if (m_UpdateComps[i]->isActive()) 
			{ 
				return false;
			} 
			m_UpdateComps[i]->kill();
		}
		for (int i = 0; i < m_RenderComps.size(); i++) 
		{ 
			if (!m_RenderComps[i]) 
			{ 
				continue; 
			} 
			if (m_RenderComps[i]->isActive()) 
			{
				return false;
			}
			m_RenderComps[i]->kill();
		}
		return true;
	}

	bool isDead() const { return m_Dead; }

	//Locations in memory of all attached components - must update in component if changes
	std::vector<UpdateComponent*> m_UpdateComps;
	std::vector<RenderComponent*> m_RenderComps;

protected:
	//Activation
	bool active() const { return m_Active; }
	void setActive(bool set) { m_Active = set; }
	unsigned int m_ID;
	bool m_Active = true;
	bool m_Dead = false;
};

#endif