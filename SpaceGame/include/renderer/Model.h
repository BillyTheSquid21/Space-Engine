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
#include <assimp/Importer.hpp>  
#include <assimp/scene.h>   
#include <assimp/postprocess.h> 

namespace Model
{
    bool LoadTextureVertexOBJ(const char* path, std::vector<TextureVertex>& verts, std::vector<unsigned int>& inds);

    class TextureVertexModel
    {
    public:
        TextureVertexModel(const char* path) { m_DataLoadedFtr = std::async(std::launch::async, LoadTextureVertexOBJ, path, std::ref(m_Vertices), std::ref(m_Indices)); }
        void setRen(Render::InstanceRenderer<TextureVertex>* ren) { m_Ren = ren; }
        void render();
    private:
        Render::InstanceRenderer<TextureVertex>* m_Ren = nullptr;
        std::future<bool> m_DataLoadedFtr;
        bool m_DataLoaded;
        std::vector<TextureVertex> m_Vertices;
        std::vector<unsigned int> m_Indices;
    };
}

#endif