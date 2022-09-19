# Space Engine

A simple framework to easily build games utilizing 2D/3D rendering and object management, among other features.
Not intended for commercial use at this time, and is purely for educational purposes.

(This applies to the v0.7-Core version)

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
	return SG::Run<Game>(640, 640, true);
}
```

This should return a blank, windowed mode GLFW screen of 640x640 resoltution.

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

The methods above must be implemented. In the cpp implementation, you must remember to run the base method (eg Game::render() or Game::clean()).
Within this derived class you can define any game logic you want. To do something useful, you should use some of the classes provided by the framework. All these classes are documented in the docs folder.

# Object Manager

- Located in the SGObject namespace
- Should be used to store game objects and their components

When defining a game object, you should inherit from the game object class. Game objects should store only data stored amongst many components and does not perform its own logic. A component performs the logic, and is paired to a game object which provides the communication between components via messaging. 

A component can either be a render or update component, with update components being able to use delta time. Their logic is defined within the update or render methods respectively.

The object manager contains an update/render heap, and update/render groups. A group puts many of the same component next to each other in memory to increase execution speed, and a heap component has its own space in memory, and must be created as a new shared pointer. Groups must be added to the object manager as a shared pointer before adding a component to them. An example of usage is provided below:

```
//Code is intended to demonstrate usage, this is not a literal example
//Picture a "Sprite" class derived from GObject that contains some data, accessed by a "SpriteRender" class derived from RenderComponent
using namespace SGObject;

ObjectManager manager;

//Create the "Sprite" GObject
Sprite sprite;
	
//Declare a group for the "Sprite Render" render component
std::shared_ptr<RenderCompGroup<SpriteRender>> renderGroup(new RenderCompGroup<SpriteRender>());

//Add group to manager
manager.pushRenderGroup(renderGroup);

//Create a sprite render component - you would then add data from the sprite that sprite render would access
SpriteRender spriteRender;
spriteRender.spritePointer = &sprite.spriteData;

//Add to the sprite render group, linking to the object render components list
renderGroup->addExistingComponent(&sprite.m_RenderComps, spriteRender);

//Create a heap "Sprite Update" update component
std::shared_ptr<SpriteUpdate> spriteUpdate(new SpriteUpdate());

//Add to the update heap
manager.pushUpdateHeap(spriteUpdate, &sprite.m_UpdateComps);

...
manager.update(deltaTime);
...
manager.render();
```


