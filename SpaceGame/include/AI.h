#ifndef AI_H
#define AI_H

#include "Ship.h"
#include "World.h"

class AI
{
public:
	AI() = default;
	AI(Ship* target, Ship* assignedShip);
	~AI() = default;
	void update(double deltaTime);

	//set world
	void setWorld(World* world);

protected:

	bool m_Brake = false;
	bool m_Orbit = false;

	//Target being sought
	Ship* m_Target;

	//Pointer to ship being controlled
	Ship* m_ShipControlling;

	//Pointer to world ship is in
	World* m_World = nullptr;
};

struct AIShip
{	
	Ship* ship;
	AI* ai;
};

#endif