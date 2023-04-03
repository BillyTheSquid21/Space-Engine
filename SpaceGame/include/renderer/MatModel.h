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
#include "renderer/Model.h"
#include "assimp/Importer.hpp"  
#include "assimp/scene.h"
#include "assimp/postprocess.h" 
#include "map"

namespace Model
{
	struct MatMesh
	{
		std::string matName;
		Material::Material mat;
		SGRender::Mesh mesh;
	};

    //TODO - make each material independant of mesh so can be shared
    //Implementation will be internal to renderer
    struct MatModel
    {
        std::vector<MatMesh> meshes;
        std::map<std::string, std::string> diffuseTextures;
        std::map<std::string, std::string> normalTextures;
        std::map<std::string, std::string> specularTextures;
        SGRender::VertexType vertexType;
    };

    bool LoadModel(const char* path, MatModel& model, SGRender::VertexType vertexType);
    bool LoadModel(const char* path, MatModel& model, SGRender::VertexType vertexType, int modelFlags);
}

#endif
