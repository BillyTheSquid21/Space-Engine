#pragma once
#ifndef RENDER_QUEUE_HPP
#define RENDER_QUEUE_HPP

#include "SGUtil.h"

//Render queue to minimise resizing arrays often
template<typename T>
struct QueueNode
{
	T object;
	unsigned int vertFloats = 0;
	const unsigned int* indices = nullptr;
	unsigned int indicesCount = 0;
	std::shared_ptr<QueueNode> nextNode = nullptr;
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

template<typename T>
class RenderQueue
{
public:
	void pushBack(T object, unsigned int floatCount, const unsigned int* indices, unsigned int indicesSize) {
		std::shared_ptr<QueueNode<T>> objectPointer(new QueueNode<T>());

		//Assign pointers to data
		objectPointer->object = object;
		objectPointer->indices = indices;
		objectPointer->indicesCount = indicesSize;
		m_TotalIndiceInts += indicesSize;
		objectPointer->vertFloats = floatCount;
		m_TotalVerticeFloats += floatCount;

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
	RenderContainer<T> nextInQueue() {
		T returnValue = m_FrontElement->object;
		unsigned int vertFloats = m_FrontElement->vertFloats;
		const unsigned int* indices = m_FrontElement->indices;
		unsigned int indicesSize = m_FrontElement->indicesCount;

		//set front element to next
		m_FrontElement = m_FrontElement->nextNode;
		m_QueueSize--;

		//decrement total floats and ints
		m_TotalVerticeFloats -= vertFloats;
		m_TotalIndiceInts -= indicesSize;

		return { returnValue, vertFloats, indices, indicesSize };
	}

	bool itemsWaiting() const { if (m_QueueSize == 0) { return false; } return true; }

	unsigned int size() const { return m_QueueSize; }
	unsigned int vertFloatCount() const { return m_TotalVerticeFloats; }
	unsigned int indIntCount() const { return m_TotalIndiceInts; }

private:
	unsigned int m_QueueSize = 0;
	unsigned int m_TotalVerticeFloats = 0;
	unsigned int m_TotalIndiceInts = 0;
	std::shared_ptr<QueueNode<T>> m_FrontElement = nullptr;
};

#endif

