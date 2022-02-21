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
//8.The user is responsible for implementing sorting a given group to increase cache hits

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
	SimpleQueue<Message> m_MessageQueue;
	std::vector<T*>* parentPointers;
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

//Base form of containers that iterates through contiguous identical-type components
class UpdateComponentGroup
{
public:
	UpdateComponentGroup() {}
	void setID(unsigned int id) { m_ID = id; }
	virtual void iterate(double deltaTime) {};
private:
	unsigned int m_ID = 0;
};

class RenderComponentGroup
{
public:
	void setID(unsigned int id) { m_ID = id; }
	virtual void iterate() {};
private:
	unsigned int m_ID = 0;
};

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
	
	//If all components are deactivated, deactive object as a whole
	void messageAll(Message message) { messageAllUpdate(message); messageAllRender(message); 
	if (message == Message::ACTIVATE) { setActive(true); }
	else if (message == Message::DEACTIVATE) { setActive(false); } };

	//Locations in memory of all attached components - must update in component if changes
	std::vector<UpdateComponent*> m_UpdateComps;
	std::vector<RenderComponent*> m_RenderComps;

protected:
	//Activation
	bool active() const { return m_Active; }
	void setActive(bool set) { m_Active = set; }
	unsigned int m_ID;
	bool m_Active = true;
};

#endif