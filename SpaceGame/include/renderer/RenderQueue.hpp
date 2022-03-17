#pragma once
#ifndef RENDER_QUEUE_HPP
#define RENDER_QUEUE_HPP

#include "utility/SGUtil.h"
#include <vector>

namespace Render
{
	template <typename T>
	struct BulkNode
	{
		BulkNode() : verts(MAX_IN_BATCH) {}
		std::vector<T> verts;
		unsigned char currentElements = 0;
		static const unsigned char MAX_IN_BATCH = 96;
		std::shared_ptr<BulkNode> nextNode = nullptr;
	};

	template <typename T>
	struct BulkRenderNode
	{
		BulkRenderNode() : verts(MAX_IN_BATCH), vertFloats(MAX_IN_BATCH), inds(MAX_IN_BATCH), indCount(MAX_IN_BATCH) {}
		std::vector<T> verts;
		std::vector<unsigned int> vertFloats;
		std::vector<const unsigned int*> inds;
		std::vector<unsigned int> indCount;
		unsigned char currentElements = 0;
		static const unsigned char MAX_IN_BATCH = 96;
		std::shared_ptr<BulkRenderNode> nextNode = nullptr;
	};

	template <typename T>
	struct RenderContainer
	{
		T verts;
		unsigned int vertFloats;
		const unsigned int* inds;
		unsigned int indCount;
	};

	template <typename T>
	struct BulkContainer
	{
		std::vector<T> verts;
		unsigned char elementsCount = 0;
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

	/**
	* Bulk Queue for batching groups of objects together in one preallocated queue node.
	* Works best for small objects.
	* Each node contains 96 slots for items.
	*/
	template <typename T>
	class BulkQueue
	{
	public:
		/**
		* Add an object to the queue. If the current queue node is not full, it adds to the array.
		* If the current node is full, it creates a new one to be filled.
		*/
		void pushBack(T object)
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
			m_CurrentElement->currentElements += 1;
		}

		/**
		* Returns the next node in the queue, in container form. Contains array of elements and element count.
		* Once the node data has been returned, it is discarded.
		* 
		* @return Bulk Container with object array and element count. 
		* 
		* Note: Getting the size of the array from the object itself will NOT return the number of elements.
		*/
		BulkContainer<T> nextInQueue()
		{
			std::vector<T> verts = m_FrontElement->verts;
			unsigned char elementsCount = m_FrontElement->currentElements;

			//decrement total floats and ints
			//Sum all floats and inds
			m_FrontElement = m_FrontElement->nextNode;
			m_QueueSize--;

			//Return required data for drawing
			return { verts, elementsCount };
		}
		/**
		* @return Whether any nodes are waiting in the queue. False when no nodes left.
		*/
		bool itemsWaiting() { if (m_QueueSize == 0) { m_FirstPush = true; return false; } return true; }
		
		/**
		* @return Size of queue
		*/
		unsigned int size() const { return m_QueueSize; }

	private:
		bool m_FirstPush = true;
		std::shared_ptr<BulkNode<T>> m_FrontElement = nullptr;
		std::shared_ptr<BulkNode<T>> m_CurrentElement = nullptr;
		unsigned int m_QueueSize = 0;
	};

	template <typename T>
	class BulkRenderQueue
	{
	public:
		/**
		* Add an object to the queue. If the current queue node is not full, it adds to the array.
		* If the current node is full, it creates a new one to be filled.
		* 
		* @param object Pointer to vertices to be rendered
		* @param floatCount Amount of total floats in vertices 
		* @param indices Pointer to indices for object
		* @param Amount of indices for drawing
		*/
		void pushBack(T object, unsigned int floatCount, const unsigned int* indices, unsigned int indicesSize)
		{
			//check if has been pushed yet
			if (m_FirstPush) {
				std::shared_ptr<BulkRenderNode<T>> firstNode(new BulkRenderNode<T>());
				m_FrontElement = firstNode;
				m_CurrentElement = firstNode;
				m_FirstPush = false;
				m_QueueSize++;
			}
			//Check if current element is full
			//If is full, make new element, point to old one, replace
			if (m_CurrentElement->currentElements >= BulkRenderNode<T>::MAX_IN_BATCH) {
				//If is full, create new one
				std::shared_ptr<BulkRenderNode<T>> newNode(new BulkRenderNode<T>());
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

		/**
		* Returns the next node in the queue, in container form. Contains array of elements and element count.
		* Also contains total vertice floats, indices and indices size.
		* Once the node data has been returned, it is discarded.
		*
		* @return Bulk Render Container with element array and amount of elements, as well as total vertice floats, indices and indices size.
		*
		* Note: Getting the size of the array from the object itself will NOT return the number of elements.
		*/
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

		/**
		* @return Total amount of vertices submitted to queue. Used for preallocating vertex buffer in rendering
		*/
		unsigned int vertFloatCount() const { return m_TotalVerticeFloats; }
		
		/**
		* @return Total amount of indices submitted to queue. Used for preallocating indice buffer in rendering
		*/
		unsigned int indIntCount() const { return m_TotalIndiceInts; }
		
		/**
		* @return Whether any nodes are waiting in the queue. False when no nodes left.
		*/
		bool itemsWaiting() { if (m_QueueSize == 0) { m_FirstPush = true; return false; } return true; }
		
		/**
		* @return Size of queue
		*/
		unsigned int size() const { return m_QueueSize; }
	private:
		bool m_FirstPush = true;
		std::shared_ptr<BulkRenderNode<T>> m_FrontElement = nullptr;
		std::shared_ptr<BulkRenderNode<T>> m_CurrentElement = nullptr;
		unsigned int m_TotalVerticeFloats = 0;
		unsigned int m_TotalIndiceInts = 0;
		unsigned int m_QueueSize = 0;
	};

}

#endif

