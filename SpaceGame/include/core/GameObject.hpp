#pragma once
#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

#include "renderer/Renderer.hpp"
#include "renderer/RenderQueue.hpp"
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

//IMPORTANT AS I AM STUPID AND DID THIS - Remember to create the group ptrs as new -_-

//Declare function pointers for sending messages if needed
//If a component wants access, pass and store the function pointer
typedef void (*Message_Update_At)(Message message, unsigned int id);
typedef void (*Message_Render_At)(Message message, unsigned int id);
typedef void (*Message_All_Update)(Message message);
typedef void (*Message_All_Render)(Message message);
typedef void (*Message_All)(Message message);


template<typename T>
class Component
{
public:
	//Function component does will carry out
	Component() {}
	void recieve(Message message) { m_MessageQueue.pushBack(message); };
	bool isActive() const { return m_Active; }
	void setActive(bool set) { m_Active = set; }
	bool isDead() const { return m_Dead; }
	//Kills component which ensures is also inactive. They will be sorted to end and before pushing the vector back, a new object will check if can replace dead comp
	void kill() { m_Dead = true; parentPointers->at(m_ID) = nullptr; setActive(false); }

	//By default only checks for deactivate and activate message - can be overidden - ACTIVATE AND DEACTIVATE MUST BE REIMPLEMENTED
	virtual void processMessages() { while (m_MessageQueue.itemsWaiting()) { Message message = m_MessageQueue.nextInQueue(); 
	if (message == Message::ACTIVATE) { setActive(true); }
	else if (message == Message::DEACTIVATE) { setActive(false); }};}

	//Attach to parent pointers - called when added to object manager and in its place
	//During runtime will have to pass pointer all the way down
	void attachToObject(std::vector<T*>* compPointers) { compPointers->push_back((T*)this); this->setID(compPointers->size() - 1); this->parentPointers = compPointers; };
	//Call after moving - if resizing whole vector call for each element
	void updatePointer() { parentPointers->at(m_ID) = (T*)this; }
private:
	//Id is location inside parent array
	void setID(unsigned char id) { m_ID = id; }
	unsigned char m_ID = 0;
	bool m_Active = true;
	bool m_Dead = false;
	SimpleQueue<Message> m_MessageQueue;
	std::vector<T*>* parentPointers = nullptr;
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
				m_Components.emplace(m_Components.begin() + i, args);
				m_Components[i].updatePointer();
				return;
			}
		}

		//Resize components array
		m_Components.emplace_back(args);
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
	void addComponent(std::vector<RenderComponent*>* compPointers, Args... arguments)
	{
		//Check if any dead
		for (int i = 0; i < m_Components.size(); i++)
		{
			if (m_Components[i].isDead())
			{
				m_Components.emplace(m_Components.begin() + i, arguments...);
				m_Components[i].updatePointer();
				return;
			}
		}

		//Resize components array
		m_Components.emplace_back(arguments...);
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

//Example code for how to insert into dead position

////Check if any dead
//for (int i = 0; i < m_Components.size(); i++)
//{
//	if (m_Components[i].isDead())
//	{
//		m_Components.emplace(m_Components.begin() + i, sprite, ren);
//		m_Components[i].updatePointer();
//		return;
//	}
//}
//
////Resize components array
//m_Components.emplace_back(sprite, ren);
//m_Components[m_Components.size() - 1].attachToObject(compPointers);
//for (int i = 0; i < m_Components.size(); i++)
//{
//	m_Components[i].updatePointer();
//}

class GameObject
{
public:
	void setID(unsigned int id) { m_ID = id; }
	unsigned int id() const { return m_ID; }

	//Messaging functions
	void messageUpdateAt(Message message, unsigned int id) { m_UpdateComps[id]->recieve(message); };
	void messageRenderAt(Message message, unsigned int id) { m_RenderComps[id]->recieve(message); };
	void messageAllUpdate(Message message) { for (int i = 0; i < m_UpdateComps.size(); i++) { messageUpdateAt(message, i); } };
	void messageAllRender(Message message) { for (int i = 0; i < m_RenderComps.size(); i++) { messageRenderAt(message, i); } };
	
	//If says kill, kill all components then mark dead
	void messageAll(Message message) { messageAllUpdate(message); messageAllRender(message); 
	if (message == Message::KILL) { m_Dead = true; }};

	//Check if all components have been decoupled (which can only be done
	//via the kill command, if any pointers aren't null, return false
	bool safeToDelete() {	
	for (int i = 0; i < m_UpdateComps.size(); i++) { if (m_UpdateComps[i] != nullptr) { return false; } }
	for (int i = 0; i < m_RenderComps.size(); i++) { if (m_RenderComps[i] != nullptr) { return false; } return true; }}

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