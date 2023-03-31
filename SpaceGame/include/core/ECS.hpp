#pragma once
#ifndef SG_ECS_HPP
#define SG_ECS_HPP

#include "stdint.h"
#include "memory"
#include "assert.h"

namespace SGECS
{

	typedef int32_t entityID;
	typedef int32_t componentID;
	typedef int32_t systemID;

	class Entity;
	class Component;
	class System;

	class ECS
	{
	public:
		virtual ~ECS() {}

		virtual entityID addEntity(Entity* entity) = 0;
		virtual void removeEntity(entityID entityID) = 0;
		virtual Entity* getEntity(entityID entityID) = 0;

		virtual componentID addComponent(entityID entityID, Component* component) = 0;
		virtual void removeComponent(entityID entityID, componentID compID) = 0;
		virtual Component* getComponent(entityID entityID, componentID compID) = 0;

		virtual systemID addSystem(System* system) = 0;
		virtual void removeSystem(systemID sysID) = 0;
		virtual System* getSystem(systemID sysID) = 0;

		virtual bool coupleSystem(entityID entityID, systemID sysID) = 0;
		virtual bool decoupleSystem(entityID entityID, systemID sysID) = 0;

		virtual void update(double deltaTime) = 0;
		virtual void render() = 0;

		virtual void clear() = 0;
		virtual void garbageCollect() = 0;
	};
}

#endif
