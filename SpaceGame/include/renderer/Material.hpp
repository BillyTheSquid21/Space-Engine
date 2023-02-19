#pragma once
#ifndef SG_MATERIAL
#define SG_MATERIAL

#include "glm/glm.hpp"

namespace Material
{
	struct Material
	{
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;
		float shininess;
	};
}

#endif
