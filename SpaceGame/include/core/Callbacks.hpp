#pragma once
#ifndef CALLBACKS_H
#define CALLBACKS_H

//callback pointer definition
typedef void (*Key_Callback)(GLFWwindow* window, int key, int scancode, int action, int mods);
typedef void (*Scroll_Callback)(GLFWwindow* window, double xoffset, double yoffset);
typedef void (*Mouse_Callback)(GLFWwindow* window, int button, int action, int mods);

//Callback declaration
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void MouseCallback(GLFWwindow* window, int button, int action, int mods);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

#endif