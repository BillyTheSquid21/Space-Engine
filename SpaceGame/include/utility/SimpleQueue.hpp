#pragma once
#ifndef SIMPLE_QUEUE_H
#define SIMPLE_QUEUE_H

#include "SGUtil.h"

//Render queue to minimise resizing arrays often
template<typename T>
struct QueueNode
{
	T object;
	std::shared_ptr<QueueNode> nextNode = nullptr;
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
class SimpleQueue : public Queue<T, QueueNode<T>> {};

#endif