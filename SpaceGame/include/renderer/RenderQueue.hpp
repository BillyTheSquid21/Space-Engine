#pragma once
#ifndef RENDER_QUEUE_HPP
#define RENDER_QUEUE_HPP

#include "SGUtil.h"
#include <vector>

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
class SimpleQueue : public Queue<T, QueueNode<T>>{};

template <typename T>
struct BulkNode
{
	BulkNode() : verts(MAX_IN_BATCH), vertFloats(MAX_IN_BATCH), inds(MAX_IN_BATCH), indCount(MAX_IN_BATCH) {}
	std::vector<T> verts;
	std::vector<unsigned int> vertFloats;
	std::vector<const unsigned int*> inds;
	std::vector<unsigned int> indCount;
	unsigned char currentElements = 0;
	static const unsigned char MAX_IN_BATCH = 96;
	std::shared_ptr<BulkNode> nextNode = nullptr;
};

template <typename T>
struct BulkRenderContainer
{
	std::vector<T> verts;
	std::vector<unsigned int> vertFloats;
	std::vector<const unsigned int*> inds;
	std::vector<unsigned int> indCount;
	unsigned char elementsCount = 0;
};

//Specific type of queue for batching requests together
//Each node has space for 64 objects to draw
//Once the node fills, then the next one is allocated
//Balances reallocation with size
template <typename T>
class BulkQueue
{
public:
	void pushBack(T object, unsigned int floatCount, const unsigned int* indices, unsigned int indicesSize)
	{
		//check if has been pushed yet
		if (m_FirstPush) {
			std::shared_ptr<BulkNode<T>> firstNode(new BulkNode<T>());
			m_FrontElement = firstNode;
			m_CurrentElement = firstNode;
			m_FirstPush = false;
			m_QueueSize++;
		}
		//Check if current element is full
		//If is full, make new element, point to old one, replace
		if (m_CurrentElement->currentElements >= BulkNode<T>::MAX_IN_BATCH) {
			//If is full, create new one
			std::shared_ptr<BulkNode<T>> newNode(new BulkNode<T>());
			//Assign new pointer to old node
			m_CurrentElement->nextNode = newNode;
			m_CurrentElement = newNode;
			m_QueueSize++;
		}
		//Get new current elements
		unsigned char currentElements = m_CurrentElement->currentElements;
		//Insert into new vectors
		m_CurrentElement->verts[currentElements] = object;
		m_CurrentElement->vertFloats[currentElements] = floatCount;
		m_CurrentElement->inds[currentElements] = indices;
		m_CurrentElement->indCount[currentElements] = indicesSize;
		m_CurrentElement->currentElements += 1;
		//Increment totals
		m_TotalIndiceInts += indicesSize;
		m_TotalVerticeFloats += floatCount;
	}

	BulkRenderContainer<T> nextInQueue()
	{
		std::vector<T> verts = m_FrontElement->verts;
		std::vector<unsigned int> vertFloats = m_FrontElement->vertFloats;
		std::vector<const unsigned int*> inds = m_FrontElement->inds;
		std::vector<unsigned int> indCount = m_FrontElement->indCount;
		unsigned char elementsCount = m_FrontElement->currentElements;

		//decrement total floats and ints
		//Sum all floats and inds
		unsigned int totalFloats = 0;
		for (int i = 0; i < vertFloats.size(); i++) {
			totalFloats += vertFloats[i];
		}
		unsigned int totalInds = 0;
		for (int i = 0; i < indCount.size(); i++) {
			totalInds += indCount[i];
		}
		m_TotalVerticeFloats -= totalFloats;
		m_TotalIndiceInts -= totalInds;

		m_FrontElement = m_FrontElement->nextNode;
		m_QueueSize--;

		//Return required data for drawing
		return { verts, vertFloats, inds, indCount, elementsCount };
	}
	//Renderer specific methods
	unsigned int vertFloatCount() const { return m_TotalVerticeFloats; }
	unsigned int indIntCount() const { return m_TotalIndiceInts; }
	bool itemsWaiting() { if (m_QueueSize == 0) { m_FirstPush = true; return false; } return true; }
	unsigned int size() const { return m_QueueSize; }
private:
	bool m_FirstPush = true;
	std::shared_ptr<BulkNode<T>> m_FrontElement = nullptr;
	std::shared_ptr<BulkNode<T>> m_CurrentElement = nullptr;
	unsigned int m_TotalVerticeFloats = 0;
	unsigned int m_TotalIndiceInts = 0;
	unsigned int m_QueueSize = 0;
};

#endif

