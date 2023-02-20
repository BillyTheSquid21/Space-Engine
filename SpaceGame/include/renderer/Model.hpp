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
#include "renderer/Vertex.hpp"
#include "renderer/Renderer.hpp"
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

    template<typename VertexType>
    bool LoadModel(const char* path, Geometry::Mesh& mesh)
    {
        return LoadModelInternal<VertexType>(path, mesh, VertexType::properties());
    }

    template<typename VertexType>
    bool LoadModel(const char* path, Geometry::Mesh& mesh, int flags)
    {
        return LoadModelInternal<VertexType>(path, mesh, flags | VertexType::properties());
    }

    void ProcessMeshFaces(const aiMesh* aimesh, std::vector<float>& verts, Geometry::MeshData& mData, int flags);

    template<typename VertexType>
    static bool LoadModelInternal(const char* path, Geometry::Mesh& mesh, int flags)
    {
        using namespace SGRender;

        //Access mesh
        mesh.unload();
        Geometry::MeshData& mData = mesh.getMesh();

#if _DEBUG
        auto start = EngineTimer::StartTimer();
#endif

        Assimp::Importer importer;
        const aiScene* tree = importer.ReadFile(path, 
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType |
            aiProcess_CalcTangentSpace * ((flags & SGRender::V_HAS_TANGENTS) != 0) //Only bother if present
        );

        if (tree == nullptr) {
            EngineLog(importer.GetErrorString());
            return false;
        }

        //Stores floats in order of normal reading (pos -> uv -> norm -> color - > tangent)
        std::vector<float> rawDataTemp;

        //Iterate over all meshes in this scene
        for (int m = 0; m < tree->mNumMeshes; m++) {
            
            const aiMesh* aimesh = tree->mMeshes[m];
            ProcessMeshFaces(aimesh, rawDataTemp, mData, flags);
        }

        mData.vertices = rawDataTemp;
        mesh.setLoaded(true);
        mesh.setProperties(flags);

#if _DEBUG
        auto time = EngineTimer::EndTimer(start);
        EngineLog("Model loaded: ", path, " ", mData.vertices.size() * sizeof(float), " bytes - currently not optimising for duplicates");
        EngineLog("Time elapsed: ", time);
#endif

        return true;
    }
}

#endif