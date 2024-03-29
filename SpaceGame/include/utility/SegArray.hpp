#pragma once
#ifndef SEG_ARRAY_H
#define SEG_ARRAY_H

#include "memory"
#include "vector"
#include "algorithm"

template
<
	typename T,		//Type
	size_t stride	//Width of data
>
class SegArray
{
public:
	SegArray() 
	{ 
		m_Stride = stride;
	};

	~SegArray()
	{
		this->clear();
	}

	T* push_back(T obj) 
	{ 
		//Check if over running
		if (m_Size == m_Nodes.size() * m_Stride || m_Size == 0)
		{
			m_Nodes.emplace_back(std::make_shared<T[]>(m_Stride), 0);
		}

		T& ind = getIndex(m_Size);
		ind = obj;
		m_Size++;
		m_Nodes.back().size++;

		return &getIndex(m_Size - 1);
	}

	template<typename... Args>
	T* emplace_back(Args... args)
	{
		//Check if over running
		if (m_Size == m_Nodes.size() * m_Stride)
		{
			m_Nodes.emplace_back(std::make_shared<T[]>(m_Stride), 0);
		}

		T& ind = getIndex(m_Size);
		ind = T(args...);
		m_Size++;
		m_Nodes.back().size++;

		return &getIndex(m_Size - 1);
	}

	void pop_back()
	{
		erase(m_Size - 1);
	}

	void erase(size_t index)
	{
		if (index >= m_Size)
		{
			return;
		}

		m_Size--;

		//Get node and shift all past index over one
		int nodeInd = getNode(index);
		int nodeLoc = index - nodeInd * m_Stride;
		int nodeLocNext = nodeLoc + 1;
		Node& node = m_Nodes[nodeInd];
		memmove(&node.data[nodeLoc], &node.data[nodeLocNext], (m_Stride - nodeLocNext)*sizeof(T));
		nodeInd++;

		while (nodeInd < m_Nodes.size())
		{
			//Set last index in last node to first in this
			Node& currentNode = m_Nodes[nodeInd];
			Node& lastNode = m_Nodes[nodeInd - 1];
			lastNode.data[m_Stride - 1] = currentNode.data[0];

			//Shift all along one
			memmove(&currentNode.data[0], &currentNode.data[1], (m_Stride - 1)*sizeof(T));
			nodeInd++;
		}

		Node& endNode = m_Nodes[m_Nodes.size() - 1];
		endNode = m_Nodes[m_Nodes.size()-1];
		endNode.size--;

		//If current node is now empty, remove
		if (endNode.size <= 0)
		{
			m_Nodes.pop_back();
		}
	}

	void shrinkTo(size_t size)
	{
		int nodeCount = getNode(size - 1) + 1;
		if (!(size < m_Stride))
		{
			m_Nodes.resize(nodeCount);
		}
		m_Nodes[m_Nodes.size() - 1].size = size - m_Nodes.size() - 1;
		m_Size = size;
	}

	void resize(size_t size)
	{
		int nodeCount = getNode(size - 1) + 1;
		int currentNodeCount = m_Nodes.size();
		if (nodeCount > currentNodeCount)
		{
			m_Nodes.resize(nodeCount);
		}

		//Initialise all new nodes
		for (int i = currentNodeCount; i < m_Nodes.size() - 1; i++)
		{
			m_Nodes[i] = { std::make_shared<T[]>(m_Stride), m_Stride };
		}

		//For last node, init and set size correctly
		size_t lastNodeSize = size - ((m_Nodes.size() - 1) * m_Stride);
		m_Nodes[m_Nodes.size() - 1] = { std::make_shared<T[]>(m_Stride), lastNodeSize };
		m_Size = size;
	}

	T& operator[](size_t index) 
	{ 
		assert(!(index >= m_Size));
		return getIndex(index);
	}

	T& back()
	{
		assert(m_Size > 0);
		return getIndex(m_Size-1);
	}

	void clear()
	{
		m_Nodes.clear();
		m_Size = 0;
	}

	size_t size() const { return m_Size; }

private:

	T& getIndex(size_t index)
	{
		//Get rounded down node number
		int node = getNode(index);

		//Get location within
		int loc = index - node * m_Stride;

		//Return index
		return m_Nodes[node].data[loc];
	}

	size_t getNode(size_t index)
	{
		return index / m_Stride;
	}

	struct Node
	{
		std::shared_ptr<T[]> data;
		size_t size = 0;
	};

	std::vector<Node> m_Nodes;
	size_t m_Stride;
	size_t m_Size = 0;
};

#endif