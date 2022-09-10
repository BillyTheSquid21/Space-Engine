#pragma once
#ifndef STATE_H
#define STATE_H

#include <GLFW/glfw3.h>
#include "ObjManagement.h"

namespace SG
{
	/**
	* Contains more local, independant logic than the Game class
	* This is where things such as the game state, menu state etc should be defined
	*/
	class State
	{
	public:
		//Universal functions
		virtual void update(double deltaTime, double time) {};
		virtual void render() {};
		virtual void handleInput(int key, int scancode, int action, int mods) {};
		virtual void handleScrolling(double xOffset, double yOffset) {};
		virtual void handleMouse(int button, int action, int mods) {};

		/**
		* Load data for the state that doesn't persist
		*/
		virtual void loadRequiredData() {};
		/**
		* Purge data for the state that doesn't persist
		*/
		virtual void purgeRequiredData() {};

		//Getters and setters
		bool active() const { return m_Active; }
		void setActive(bool set) { m_Active = set; }
		bool hasDataLoaded() const { return m_DataLoaded; }

	protected:
		bool m_Active = false;
		bool m_DataLoaded = false;
	};
}

#endif