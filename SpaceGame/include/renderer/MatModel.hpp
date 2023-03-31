#pragma once
#ifndef SG_MATMODEL_H
#define SG_MATMODEL_H

#include "vector"
#include "utility/SGUtil.h"
#include "renderer/GLClasses.h"
#include "renderer/Vertex.h"
#include "renderer/Mesh.h"
#include "renderer/Texture.h"
#include "renderer/Material.hpp"
#include "renderer/Model.hpp"
#include "assimp/Importer.hpp"  
#include "assimp/scene.h"
#include "assimp/postprocess.h" 
#include "map"

#define _SHOW_MODEL_DEBUG 0 //Model info can be dense so allow easy switch off

namespace Model
{
	struct MatMesh
	{
		std::string matName;
		Material::Material mat;
		SGRender::Mesh mesh;
	};

    struct MatModel
    {
        std::vector<MatMesh> meshes;
        std::map<std::string, std::string> diffuseTextures; //Testing solution
        std::map<std::string, std::string> normalTextures;
        std::map<std::string, std::string> specularTextures;
    };

    bool LoadModel(const char* path, MatModel& model, SGRender::VertexType vertexType);
    bool LoadModel(const char* path, MatModel& model, SGRender::VertexType vertexType, int modelFlags);
}

#endif
