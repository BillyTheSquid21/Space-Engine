#pragma once
#ifndef STATE_H
#define STATE_H

#include <GLFW/glfw3.h>

//State class - primarily virtual functions
class State
{
public:
	//Universal functions
	virtual void update(double deltaTime, double time) {};
	virtual void render() {};
	virtual void handleInput(int key, int scancode, int action, int mods) {};
	virtual void handleScrolling(double xOffset, double yOffset) {};
	virtual void handleMouse(int button, int action, int mods) {};

	//Getters and setters
	bool active() const { return m_Active; }
	void setActive(bool set) { m_Active = set; }

private:
	bool m_Active = false;
};

#endif