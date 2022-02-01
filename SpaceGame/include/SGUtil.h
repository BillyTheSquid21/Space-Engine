#pragma once
#ifndef SGUTIL_H
#define SGUTIL_H

#include <iostream>

//pi - i define it because it seems easiest
#define SG_PI 3.141592f

static const char ENGINE_TAG[]{ "[Space Engine] " };

template<typename T>
void EngineLog(T value) {
	std::cout << ENGINE_TAG;
	std::cout << value << "\n";
}
template<typename T>
void EngineLog(std::string message,T value) {
	std::cout << ENGINE_TAG;
	std::cout << message;
	std::cout << value << "\n";
}

template<typename T>
void EngineLog(T value1, T value2) {
	std::cout << ENGINE_TAG;
	std::cout << value1 << " ";
	std::cout << value2 << "\n";
}

template<typename T>
void EngineLog(std::string message, T value1, T value2) {
	std::cout << ENGINE_TAG;
	std::cout << message << " ";
	std::cout << value1 << " ";
	std::cout << value2 << "\n";
}

float invSqrt(float number); //Supposedly a faster inv sqrt
int intFloor(float x); //supposedly faster floor

//return structs
struct Component2f
{
	float a, b;
};

struct Component3f
{
	float a, b, c;
};

struct Component4f
{
	float a, b, c, d;
};

//Render queue to minimise resizing arrays often
template<typename T>
struct QueueNode
{
	T object;
	std::shared_ptr<QueueNode> nextNode = nullptr;
};

template<typename T>
class RenderQueue
{
public:
	void pushBack(T object) {
		std::shared_ptr<QueueNode<T>> objectPointer(new QueueNode<T>());
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

private:
	unsigned int m_QueueSize = 0;
	std::shared_ptr<QueueNode<T>> m_FrontElement = nullptr;
};

#endif
