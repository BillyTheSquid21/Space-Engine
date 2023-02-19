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
    //TODO - optimise to avoid vertice duplicates
    template<typename VertexType>
    bool LoadModel(const char* path, Geometry::Mesh& mesh)
    {
        using namespace SGRender;

        int properties = VertexType::properties();

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
            aiProcess_CalcTangentSpace * ((properties & SGRender::hasTangents) != 0) //Only bother if present
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
            //Iterate over all faces in this mesh
            for (int j = 0; j < aimesh->mNumFaces; j++) {

                auto const& face = aimesh->mFaces[j];
                //Iterate face vertices
                for (int k = 0; k < 3; ++k) {

                    //Get indice
                    unsigned int indice = face.mIndices[k];

                    //Assume always has position
                    auto const& vertex = aimesh->mVertices[indice];
                    rawDataTemp.push_back(vertex.x);
                    rawDataTemp.push_back(vertex.y);
                    rawDataTemp.push_back(vertex.z);

                    //Next do UV
                    if (aimesh->HasTextureCoords(0) && (properties & SGRender::hasUVs)) {
                        // The following line fixed the issue for me now:
                        auto const& uv = aimesh->mTextureCoords[0][indice];
                        rawDataTemp.push_back(uv.x);
                        rawDataTemp.push_back(uv.y);
                    }

                    //Next Normal
                    if (aimesh->HasNormals() && (properties & SGRender::hasNormals))
                    {
                        auto const& normal = aimesh->mNormals[indice];
                        rawDataTemp.push_back(normal.x);
                        rawDataTemp.push_back(normal.y);
                        rawDataTemp.push_back(normal.z);
                    }

                    //Next Color
                    if (aimesh->HasVertexColors(0) && (SGRender::hasColor))
                    {
                        auto const& color = aimesh->mColors[indice];
                        rawDataTemp.push_back(color->r);
                        rawDataTemp.push_back(color->g);
                        rawDataTemp.push_back(color->b);
                        rawDataTemp.push_back(color->a);
                    }

                    //Next Tangents
                    if (aimesh->HasTangentsAndBitangents() && (SGRender::hasTangents))
                    {
                        auto const& tangent = aimesh->mTangents[indice];
                        rawDataTemp.push_back(tangent.x);
                        rawDataTemp.push_back(tangent.y);
                        rawDataTemp.push_back(tangent.z);
                    }

                    //Now push back indice
                    mData.indices.push_back(mData.indices.size());
                }
            }
        }

        mData.vertices = rawDataTemp;
        mesh.setLoaded(true);
        mesh.setProperties(properties);

#if _DEBUG
        auto time = EngineTimer::EndTimer(start);

        EngineLog("Model loaded: ", path, " ", mData.vertices.size() * sizeof(float), " bytes - currently not optimising for duplicates");
        EngineLog("Time elapsed: ", time);
#endif

        return true;
    }
}

#endif