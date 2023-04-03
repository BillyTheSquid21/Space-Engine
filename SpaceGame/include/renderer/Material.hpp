#pragma once
#ifndef SG_MATERIAL
#define SG_MATERIAL

#include "glm/glm.hpp"

namespace Material
{
	//-1 signals material attribute not used
	struct Material
	{
		glm::vec3 ambient = glm::vec3(-1);
		glm::vec3 diffuse = glm::vec3(-1);
		glm::vec3 specular = glm::vec3(-1);
		float shininess = -1;
	};
}

#endif
