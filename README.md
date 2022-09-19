# Space Engine

A simple framework to easily build games utilizing 2D/3D rendering and object management, among other features.
Not intended for commercial use at this time, and is purely for educational purposes.

# Get Started

To get started, first follow the below instructions:

1. Fork and clone the repository using the "v0.7-Core" branch
2. Open the "SpaceEngine.sln" file in Visual Studio 2019
3. Create your "main.cpp" file in the root of the src directory

The "main.cpp" file should then be set up as such:

```
#include "core/Game.h"
#include "core/Run.h"

int main(void)
{   
	return SG::Run<Game>(640, 640);
}
```

This should return a blank GLFW screen of 640x640 resoltution.

To do something useful, create a class for your game, that inherits from the "Game" class in "Game.h":

```
#include "core/Game.h"

class DemoGame : public Game
{
public:
	using Game::Game;
	~DemoGame() {ImGui_ImplOpenGL3_Shutdown();ImGui_ImplGlfw_Shutdown();ImGui::DestroyContext();}
	bool init(const char name[], Key_Callback kCallback, Mouse_Callback mCallback, Scroll_Callback sCallback);
	void handleInput(int key, int scancode, int action, int mods);
	void handleMouse(int button, int action, int mods);
	void handleScrolling(double xOffset, double yOffset);
	void update(double deltaTime);
	void render();
	void clean();
};
```

The methods above must be implemented. In the cpp implementation, you must remember to run the base method (eg Game::render()).

