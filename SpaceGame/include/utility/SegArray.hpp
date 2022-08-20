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
		m_Nodes.emplace_back(std::make_shared<T[]>(m_Stride), 0);
	};

	void pushBack(T obj) 
	{ 
		//Check if over running
		if (m_Size == m_Nodes.size() * m_Stride)
		{
			m_Nodes.emplace_back(std::make_shared<T[]>(m_Stride), 0);
		}

		T& ind = getIndex(m_Size);
		ind = obj;
		m_Size++;

		m_Nodes[m_Nodes.size()-1].size++;
	}

	void eraseAt(size_t index)
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
		m_Nodes.resize(nodeCount);
		m_Nodes[m_Nodes.size() - 1].size = size - m_Nodes.size() - 1;
		m_Size = size;
	}

	T& operator[](size_t index) 
	{ 
		assert(!(index >= m_Size));
		return getIndex(index);
	}

	size_t size() const { return m_Size; }

private:

	inline T& getIndex(size_t index)
	{
		//Get rounded down node number
		int node = getNode(index);

		//Get location within
		int loc = index - node * m_Stride;

		//Return index
		return m_Nodes[node].data[loc];
	}

	inline int getNode(size_t index)
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