#pragma once
#ifndef SG_MODEL_H
#define SG_MODEL_H

#include "vector"
#include "iterator"
#include "future"
#include "chrono"
#include "future"

#include "utility/SGUtil.h"

#include "renderer/GLClasses.h"
#include "renderer/Vertex.h"
#include "renderer/Mesh.h"
#include "renderer/Texture.h"

#include "assimp/Importer.hpp"  
#include "assimp/scene.h"
#include "assimp/postprocess.h" 

namespace Model
{
    enum MFlag
    {
        M_TRIANGLES_ONLY = 0x100,
        M_LINES_ONLY     = 0x200,
        M_FILL_MISSING   = 0x400,
        M_COMBINE_MESHES = 0x800,
    };
    //TODO FIX AFTER WAS TEMPLATED

    bool LoadModel(const char* path, SGRender::Mesh& mesh, SGRender::VertexType vertexType);
    bool LoadModel(const char* path, SGRender::Mesh& mesh, SGRender::VertexType vertexType, int flags);
    void ProcessMeshFaces(const aiMesh* aimesh, std::vector<float>& verts, SGRender::MeshData& mData, SGRender::VertexType vertexType, int flags);
}

#endif