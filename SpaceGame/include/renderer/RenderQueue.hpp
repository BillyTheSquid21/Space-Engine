#pragma once
#ifndef RENDER_QUEUE_HPP
#define RENDER_QUEUE_HPP

#include "SGUtil.h"

//Render queue to minimise resizing arrays often
template<typename T>
struct QueueNode
{
	T object;
	std::shared_ptr<QueueNode> nextNode = nullptr;
};

template<typename T>
struct RenderQueueNode
{
	T object;
	unsigned int vertFloats = 0;
	const unsigned int* indices = nullptr;
	unsigned int indicesCount = 0;
	std::shared_ptr<RenderQueueNode> nextNode = nullptr;
};

//Same as queue node but doesnt point anywhere - is the format for return
template<typename T>
struct RenderContainer
{
	T object;
	unsigned int vertFloats = 0;
	const unsigned int* indices = nullptr;
	unsigned int indicesCount = 0;
};

//Simplest queue system - assumes using queue node, can be overloaded (eg RenderQueueNode)
template<typename T, class nodetype>
class Queue
{
public:
	void pushBack(T object) {
		std::shared_ptr<QueueNode<T>> objectPointer(new QueueNode<T>());

		//Assign pointers to data
		objectPointer->object = object;
		if (m_FrontElement == NULL) {
			m_FrontElement = objectPointer;
			m_QueueSize++;
			return;
		}
		std::shared_ptr<QueueNode<T>> currentPointer = m_FrontElement;
		for (int i = 0; i < m_QueueSize; i++) {
			if (currentPointer->nextNode == NULL) {
				currentPointer->nextNode = objectPointer;
				m_QueueSize++;
				return;
			}
			currentPointer = currentPointer->nextNode;
		}

	};
	T nextInQueue() {
		T returnValue = m_FrontElement->object;

		//set front element to next
		m_FrontElement = m_FrontElement->nextNode;
		m_QueueSize--;

		return returnValue;
	}

	bool itemsWaiting() const { if (m_QueueSize == 0) { return false; } return true; }
	unsigned int size() const { return m_QueueSize; }

protected:
	unsigned int m_QueueSize = 0;
	std::shared_ptr<nodetype> m_FrontElement = nullptr;
};

//Implementation of base queue
template<typename T>
class SimpleQueue : public Queue<T, QueueNode<T>>{};

template<typename T>
class RenderQueue : public Queue<T, RenderQueueNode<T>>
{
public:
	void pushBack(T object, unsigned int floatCount, const unsigned int* indices, unsigned int indicesSize) {

		//Pointer
		std::shared_ptr<RenderQueueNode<T>> objectPointer(new RenderQueueNode<T>());

		//Assign pointers to data
		objectPointer->object = object;
		objectPointer->indices = indices;
		objectPointer->indicesCount = indicesSize;
		m_TotalIndiceInts += indicesSize;
		objectPointer->vertFloats = floatCount;
		m_TotalVerticeFloats += floatCount;

		if (this->m_FrontElement == NULL) {
			this->m_FrontElement = objectPointer;
			this->m_QueueSize++;
			return;
		}
		std::shared_ptr<RenderQueueNode<T>> currentPointer = this->m_FrontElement;
		for (int i = 0; i < this->m_QueueSize; i++) {
			if (currentPointer->nextNode == NULL) {
				currentPointer->nextNode = objectPointer;
				this->m_QueueSize++;
				return;
			}
			currentPointer = currentPointer->nextNode;
		}

	};
	RenderContainer<T> nextInQueue() {

		//Get renderer specific data
		unsigned int vertFloats = this->m_FrontElement->vertFloats;
		const unsigned int* indices = this->m_FrontElement->indices;
		unsigned int indicesSize = this->m_FrontElement->indicesCount;

		//Carry out queue instructions as in base class
		T returnValue = Queue<T, RenderQueueNode<T>>::nextInQueue();

		//decrement total floats and ints
		m_TotalVerticeFloats -= vertFloats;
		m_TotalIndiceInts -= indicesSize;

		//Return required data for drawing
		return { returnValue, vertFloats, indices, indicesSize };
	}
	//Renderer specific methods
	unsigned int vertFloatCount() const { return m_TotalVerticeFloats; }
	unsigned int indIntCount() const { return m_TotalIndiceInts; }

private:
	//makes original push back not public
	using Queue<T, RenderQueueNode<T>>::pushBack;
	unsigned int m_TotalVerticeFloats = 0;
	unsigned int m_TotalIndiceInts = 0;
};

#endif

