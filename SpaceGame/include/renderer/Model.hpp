#pragma once
#ifndef SG_MODEL_H
#define SG_MODEL_H

#include <vector>
#include <iterator>
#include <future>
#include <chrono>
#include <future>

#include "utility/SGUtil.h"

#include "renderer/GLClasses.h"
#include "renderer/Vertex.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Texture.h"

#include <assimp/Importer.hpp>  
#include <assimp/scene.h>   
#include <assimp/postprocess.h> 

namespace Model
{
    template<typename VertexType>
    bool LoadTextureVertexOBJ(const char* path, std::vector<VertexType>& verts, std::vector<unsigned int>& inds)
    {
        //Check is a valid vertex
        static_assert(std::is_base_of<Vertex, VertexType>::value, "Must be a vertex type!");

        auto start = EngineTimer::StartTimer();

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path,
            aiProcess_CalcTangentSpace |
            aiProcess_Triangulate |
            aiProcess_JoinIdenticalVertices |
            aiProcess_SortByPType);

        if (scene == nullptr) {
            EngineLog(importer.GetErrorString());
            return false;
        }

        //Temp data - only written to if attribute exists
        std::vector<glm::vec3> positionDataTemp;
        std::vector<glm::vec2> uvDataTemp;
        std::vector<glm::vec3> normalDataTemp;

        //Get vertices
        auto mesh = scene->mMeshes[0];
        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            glm::vec3 vert;
            vert.x = mesh->mVertices[i].x;
            vert.y = mesh->mVertices[i].y;
            vert.z = mesh->mVertices[i].z;
            positionDataTemp.push_back(vert);
        }

        //Get uvs
        bool hasUVS = false;
        //Check if both mesh and vertices have UV
        if (&mesh->mTextureCoords[0][0] != NULL && std::is_base_of<TextureVertex, VertexType>::value)
        {
            hasUVS = true;
        }
        if (hasUVS)
        {
            for (int i = 0; i < mesh->mNumVertices; i++)
            {
                glm::vec2 uv;
                uv.x = mesh->mTextureCoords[0][i].x;
                uv.y = mesh->mTextureCoords[0][i].y;
                uvDataTemp.push_back(uv);
            }
        }

        //Get normals
        bool hasNormals = false;
        //Check if both mesh and vertex type have normals
        if (&mesh->mNormals[0][0] != NULL && std::is_base_of<NormalTextureVertex, VertexType>::value)
        {
            hasNormals = true;
        }
        if (hasNormals)
        {
            for (int i = 0; i < mesh->mNumVertices; i++)
            {
                glm::vec3 normal;
                normal.x = mesh->mNormals[i].x;
                normal.y = mesh->mNormals[i].y;
                normal.z = mesh->mNormals[i].z;
                normalDataTemp.push_back(normal);
            }
        }

        //Indices
        for (int i = 0; i < mesh->mNumFaces; i++)
        {
            inds.push_back(mesh->mFaces[i].mIndices[0]);
            inds.push_back(mesh->mFaces[i].mIndices[1]);
            inds.push_back(mesh->mFaces[i].mIndices[2]);
        }

        //Write to vertex
        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            VertexType vert;
            vert.position = positionDataTemp[i];
            if (hasUVS)
            {
                vert.uvCoords = uvDataTemp[i];
            }
            if (hasNormals)
            {
                vert.normals = normalDataTemp[i];
                vert.normals = glm::normalize(vert.normals);
            }
            verts.push_back(vert);
        }
        auto time = EngineTimer::EndTimer(start);

        EngineLog("Model loaded: ", path);
        EngineLog("Time elapsed: ", time);
        if (time > 4)
        {
            EngineLog("Model took a LONG time to load");
        }

        return true;
    }

    //Only supports one material - cannot split between materials
    template <typename VertexType>
    class Model
    {
    public:
        Model(const char* path) { m_DataLoadedFtr = std::async(std::launch::async, LoadTextureVertexOBJ<VertexType>, path, std::ref(m_Vertices), std::ref(m_Indices)); }
        void setRen(Render::Renderer<VertexType>* ren) { m_Ren = ren; }
        void render()
        {
            if (m_DataLoadedFtr._Is_ready())
            {
                m_DataLoaded = m_DataLoadedFtr.get();
            }
            if (!m_DataLoaded)
            {
                return;
            }
            m_Ren->commit(&m_Vertices[0], m_Vertices.size() * (sizeof(VertexType) / sizeof(float)), &m_Indices[0], m_Indices.size());
        }

        VertexType* getVertices() const { return (VertexType*)&m_Vertices[0]; }
        unsigned int getVertCount() const { return m_Vertices.size(); }

    private:
        Render::Renderer<VertexType>* m_Ren = nullptr;
        std::future<bool> m_DataLoadedFtr;
        bool m_DataLoaded;

        //Data
        std::vector<VertexType> m_Vertices;
        std::vector<unsigned int> m_Indices;
    };

    template<typename VertexType>
    class MorphModel : public Model<VertexType>
    {

    };
}

#endif