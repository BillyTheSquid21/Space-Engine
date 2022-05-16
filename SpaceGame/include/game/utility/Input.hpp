#pragma once
#ifndef INPUT_H
#define INPUT_H

#include <GLFW/glfw3.h>

//Definitions for inputs that persist
#define STORED_INPUTS_COUNT 14
#define HELD_A m_PersistentInput[0]
#define HELD_D m_PersistentInput[1]
#define HELD_CTRL m_PersistentInput[2]
#define HELD_SHIFT m_PersistentInput[3]
#define HELD_Q m_PersistentInput[4]
#define HELD_E m_PersistentInput[5]
#define HELD_W m_PersistentInput[6]
#define HELD_S m_PersistentInput[7]
//For just pressed ones, reset per frame
#define PRESSED_X m_PersistentInput[8]
#define PRESSED_A m_PersistentInput[9]
#define PRESSED_D m_PersistentInput[10]
#define PRESSED_W m_PersistentInput[11]
#define PRESSED_S m_PersistentInput[12]
#define PRESSED_E m_PersistentInput[13]

class GameInput
{
public:
	//Input
	//Definitions for persistent input
	bool m_PersistentInput[STORED_INPUTS_COUNT];

	//Time to hold any input before held state
	double m_TimeHeld = 0.0;

	//Time before becomes held
	const double m_TimeToHold = 0.313;
	bool m_HoldTimerActive = false;

	void update(double deltaTime) 
	{
		if (m_HoldTimerActive)
		{
			m_TimeHeld += deltaTime;
		}
	}

	void handleInput(int key, int scancode, int action, int mods)
	{
        //Reset pressed
        PRESSED_A = false; PRESSED_D = false; PRESSED_S = false; PRESSED_W = false; PRESSED_E = false; PRESSED_X = false;

        if (key == GLFW_KEY_A) {
            if (action == GLFW_PRESS) {
                PRESSED_A = true;
                m_HoldTimerActive = true;
                if (HELD_SHIFT)
                {
                    m_TimeHeld = m_TimeToHold;
                }
            }
            else if (action == GLFW_RELEASE) {
                HELD_A = false;
                m_TimeHeld = 0.0;
            }
            if (m_TimeHeld >= m_TimeToHold)
            {
                HELD_A = true;
                m_HoldTimerActive = false;
            }
        }
        if (key == GLFW_KEY_D) {
            if (action == GLFW_PRESS) {
                PRESSED_D = true;
                m_HoldTimerActive = true;
                if (HELD_SHIFT)
                {
                    m_TimeHeld = m_TimeToHold;
                }
            }
            else if (action == GLFW_RELEASE) {
                HELD_D = false;
                m_TimeHeld = 0.0;
            }
            if (m_TimeHeld >= m_TimeToHold)
            {
                HELD_D = true;
                m_HoldTimerActive = false;
            }
        }
        if (key == GLFW_KEY_X)
        {
            if (action == GLFW_PRESS)
            {
                PRESSED_X = true;
            }
        }
        if (key == GLFW_KEY_LEFT_CONTROL) {
            if (action == GLFW_PRESS) {
                HELD_CTRL = true;
            }
            else if (action == GLFW_RELEASE) {
                HELD_CTRL = false;
            }
        }
        if (key == GLFW_KEY_LEFT_SHIFT) {
            if (action == GLFW_PRESS) {
                HELD_SHIFT = true;
            }
            else if (action == GLFW_RELEASE) {
                HELD_SHIFT = false;
            }
        }
        if (key == GLFW_KEY_Q) {
            if (action == GLFW_PRESS) {
                HELD_Q = true;
            }
            else if (action == GLFW_RELEASE) {
                HELD_Q = false;
            }
        }
        if (key == GLFW_KEY_E) {
            if (action == GLFW_PRESS) {
                HELD_E = true;
            }
            else if (action == GLFW_RELEASE) {
                HELD_E = false;
            }
        }
        if (key == GLFW_KEY_W) {
            if (action == GLFW_PRESS) {
                PRESSED_W = true;
                m_HoldTimerActive = true;
                if (HELD_SHIFT)
                {
                    m_TimeHeld = m_TimeToHold;
                }
            }
            else if (action == GLFW_RELEASE) {
                HELD_W = false;
                m_TimeHeld = 0.0;
            }
            if (m_TimeHeld >= m_TimeToHold)
            {
                HELD_W = true;
                m_HoldTimerActive = false;
            }
        }
        if (key == GLFW_KEY_S) {
            if (action == GLFW_PRESS) {
                PRESSED_S = true;
                m_HoldTimerActive = true;
                if (HELD_SHIFT)
                {
                    m_TimeHeld = m_TimeToHold;
                }
            }
            else if (action == GLFW_RELEASE) {
                HELD_S = false;
                m_TimeHeld = 0.0;
            }
            if (m_TimeHeld >= m_TimeToHold)
            {
                HELD_S = true;
                m_HoldTimerActive = false;
            }
        }
        if (key == GLFW_KEY_E)
        {
            if (action == GLFW_PRESS)
            {
                PRESSED_E = true;
            }
        }
	}
};

#endif