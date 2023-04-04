#pragma once
#ifndef SG_MATERIAL
#define SG_MATERIAL

#include "glm/glm.hpp"
#include "string"
#include "map"

namespace Material
{
	//-1 signals material attribute not used
	struct Material
	{
		//Constants
		std::string name;
		glm::vec3 ambient = glm::vec3(-1);
		glm::vec3 diffuse = glm::vec3(-1);
		glm::vec3 specular = glm::vec3(-1);
		float shininess = -1;

		//Textures
		std::string diffuseTexture;
		std::string normalTexture;
		std::string specularTexture;
	};
}

#endif
