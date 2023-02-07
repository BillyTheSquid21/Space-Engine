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
        Geometry::MeshData& meshData = mesh.getMesh();

        auto start = EngineTimer::StartTimer();

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
            
            const aiMesh* mesh = tree->mMeshes[m];
            //Iterate over all faces in this mesh
            for (int j = 0; j < mesh->mNumFaces; j++) {

                auto const& face = mesh->mFaces[j];
                //Iterate face vertices
                for (int k = 0; k < 3; ++k) {

                    //Get indice
                    unsigned int indice = face.mIndices[k];

                    //Assume always has position
                    auto const& vertex = mesh->mVertices[indice];
                    rawDataTemp.push_back(vertex.x);
                    rawDataTemp.push_back(vertex.y);
                    rawDataTemp.push_back(vertex.z);

                    //Next do UV
                    if (mesh->HasTextureCoords(0) && (properties & SGRender::hasUVs)) {
                        // The following line fixed the issue for me now:
                        auto const& uv = mesh->mTextureCoords[0][indice];
                        rawDataTemp.push_back(uv.x);
                        rawDataTemp.push_back(uv.y);
                    }

                    //Next Normal
                    if (mesh->HasNormals() && (properties & SGRender::hasNormals))
                    {
                        auto const& normal = mesh->mNormals[indice];
                        rawDataTemp.push_back(normal.x);
                        rawDataTemp.push_back(normal.y);
                        rawDataTemp.push_back(normal.z);
                    }

                    //Next Color
                    if (mesh->HasVertexColors(0) && (SGRender::hasColor))
                    {
                        auto const& color = mesh->mColors[indice];
                        rawDataTemp.push_back(color->r);
                        rawDataTemp.push_back(color->g);
                        rawDataTemp.push_back(color->b);
                        rawDataTemp.push_back(color->a);
                    }

                    //Next Tangents
                    if (mesh->HasTangentsAndBitangents() && (SGRender::hasTangents))
                    {
                        auto const& tangent = mesh->mTangents[indice];
                        rawDataTemp.push_back(tangent.x);
                        rawDataTemp.push_back(tangent.y);
                        rawDataTemp.push_back(tangent.z);
                    }

                    //Now push back indice
                    meshData.indices.push_back(meshData.indices.size());
                }
            }
        }

        meshData.vertices = rawDataTemp;
        mesh.setLoaded(true);
        mesh.setProperties(properties);

        auto time = EngineTimer::EndTimer(start);

        EngineLog("Model loaded: ", path, " ", meshData.vertices.size() * sizeof(float), " bytes - currently not optimising for duplicates");
        EngineLog("Time elapsed: ", time);
        if (time > 4)
        {
            EngineLog("Model took a LONG time to load");
        }

        return true;
    }
}

#endif