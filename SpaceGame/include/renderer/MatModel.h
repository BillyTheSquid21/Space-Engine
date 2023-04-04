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
    //Contains a mesh and a material
	struct MatMesh
	{
		Material::Material mat;
		SGRender::Mesh mesh;
	};

    //Contains a series of meshes split by material, with material data included
    struct MatModel
    {
        std::vector<MatMesh> meshes;
        SGRender::VertexType vertexType;
    };

    bool LoadModel(const char* path, MatModel& model, SGRender::VertexType vertexType);
    bool LoadModel(const char* path, MatModel& model, SGRender::VertexType vertexType, int modelFlags);
}

#endif
