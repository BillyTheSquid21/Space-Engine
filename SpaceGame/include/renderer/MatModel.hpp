#pragma once
#ifndef SG_MATMODEL_H
#define SG_MATMODEL_H

#include "vector"
#include "utility/SGUtil.h"
#include "renderer/GLClasses.h"
#include "renderer/Vertex.hpp"
#include "renderer/Renderer.hpp"
#include "renderer/Texture.h"
#include "renderer/Material.hpp"
#include "assimp/Importer.hpp"  
#include "assimp/scene.h"
#include "assimp/postprocess.h" 

#define _SHOW_MODEL_DEBUG 1

namespace Model
{
	struct MatMesh
	{
		std::string matName;
		Material::Material mat;
		Geometry::Mesh mesh;
	};

    struct MatModel
    {
        std::vector<MatMesh> meshes;
    };

    template<typename VertexType>
    bool LoadModel(const char* path, MatModel& model)
    {
        using namespace SGRender;
        
        EngineLog("Loading material mesh: ", path);

        int properties = VertexType::properties();

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

        //Set materials
        model.meshes.resize(tree->mNumMaterials);
        for (int i = 0; i < tree->mNumMaterials; i++)
        {
            model.meshes[i].mesh.unload();
            aiMaterial* mat = tree->mMaterials[i];
            
            aiString name;
            aiColor3D diff;
            aiColor3D amb;
            aiColor3D spec;
            float shin;

            mat->Get(AI_MATKEY_NAME, name);
            mat->Get(AI_MATKEY_COLOR_DIFFUSE, diff);
            mat->Get(AI_MATKEY_COLOR_AMBIENT, amb);
            mat->Get(AI_MATKEY_COLOR_SPECULAR, spec);
            mat->Get(AI_MATKEY_SHININESS, shin);
            
            //Set properties and convert where appropriate
            MatMesh& mmesh = model.meshes[i];
            mmesh.matName = name.C_Str(); 
            mmesh.mat.ambient = {amb.r, amb.g, amb.b};
            mmesh.mat.diffuse = {diff.r, diff.g, diff.b};
            mmesh.mat.specular = { spec.r, spec.g, spec.b };
            mmesh.mat.shininess = shin;
        }

#if _SHOW_MODEL_DEBUG

        //List all the important info about the model
        EngineLog("Material Count: ", tree->mNumMaterials);
        EngineLog("Mesh Count: ", tree->mNumMeshes);
        EngineLog("Texture Count: ", tree->mNumTextures);
        EngineLog("Lights Count: ", tree->mNumLights);

        //List each material and associated textures
        for (int i = 0; i < tree->mNumMaterials; i++)
        {
            const char* mname = tree->mMaterials[i]->GetName().C_Str();
            EngineLog("Material: ", mname);
            EngineLog(mname, " diffuse texture count: ", tree->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE));
            EngineLog(mname, " ambient texture count: ", tree->mMaterials[i]->GetTextureCount(aiTextureType_AMBIENT));
            EngineLog(mname, " normal texture count: ", tree->mMaterials[i]->GetTextureCount(aiTextureType_NORMALS));
        }

#endif

        //Read in as normal, but add to meshData that is associated with material index
        //keep each materials worth of rawData separate
        std::vector<std::vector<float>> rawDataTemp;
        rawDataTemp.resize(tree->mNumMaterials);

        //Iterate over all meshes in this scene
        for (int m = 0; m < tree->mNumMeshes; m++) {

            const aiMesh* aimesh = tree->mMeshes[m];
            int mIndex = aimesh->mMaterialIndex;
           
            //Iterate over all faces in this mesh
            for (int j = 0; j < aimesh->mNumFaces; j++) {

                auto const& face = aimesh->mFaces[j];
                //Iterate face vertices
                for (int k = 0; k < 3; ++k) {

                    //Get indice
                    unsigned int indice = face.mIndices[k];

                    //Assume always has position
                    auto const& vertex = aimesh->mVertices[indice];
                    rawDataTemp[mIndex].push_back(vertex.x);
                    rawDataTemp[mIndex].push_back(vertex.y);
                    rawDataTemp[mIndex].push_back(vertex.z);

                    //Next do UV
                    if (aimesh->HasTextureCoords(0) && (properties & SGRender::hasUVs)) {
                        // The following line fixed the issue for me now:
                        auto const& uv = aimesh->mTextureCoords[0][indice];
                        rawDataTemp[mIndex].push_back(uv.x);
                        rawDataTemp[mIndex].push_back(uv.y);
                    }

                    //Next Normal
                    if (aimesh->HasNormals() && (properties & SGRender::hasNormals))
                    {
                        auto const& normal = aimesh->mNormals[indice];
                        rawDataTemp[mIndex].push_back(normal.x);
                        rawDataTemp[mIndex].push_back(normal.y);
                        rawDataTemp[mIndex].push_back(normal.z);
                    }

                    //Next Color
                    if (aimesh->HasVertexColors(0) && (SGRender::hasColor))
                    {
                        auto const& color = aimesh->mColors[indice];
                        rawDataTemp[mIndex].push_back(color->r);
                        rawDataTemp[mIndex].push_back(color->g);
                        rawDataTemp[mIndex].push_back(color->b);
                        rawDataTemp[mIndex].push_back(color->a);
                    }

                    //Next Tangents
                    if (aimesh->HasTangentsAndBitangents() && (SGRender::hasTangents))
                    {
                        auto const& tangent = aimesh->mTangents[indice];
                        rawDataTemp[mIndex].push_back(tangent.x);
                        rawDataTemp[mIndex].push_back(tangent.y);
                        rawDataTemp[mIndex].push_back(tangent.z);
                    }

                    //Now push back indice
                    Geometry::MeshData& mData = model.meshes[mIndex].mesh.getMesh();
                    mData.indices.push_back(mData.indices.size());
                }
            }
        }

        //Set each mesh data to rawDataTemp index
        for (int i = 0; i < model.meshes.size(); i++)
        {
            Geometry::Mesh& m = model.meshes[i].mesh;
            Geometry::MeshData& mData = m.getMesh();
            mData.vertices = rawDataTemp[i];
            m.setLoaded(true);
            m.setProperties(properties);
        }

        //Check for any redundant material meshes
        for (int i = 0; i < model.meshes.size(); i++)
        {
            if (model.meshes[i].mesh.getVertSize() == 0)
            {
                model.meshes.erase(model.meshes.begin() + i);
                i--;
            }
        }

#if _DEBUG
        auto time = EngineTimer::EndTimer(start);
        size_t s = 0;
        for (auto& m : model.meshes)
        {
            s += m.mesh.getMesh().vertices.size();
        }
        EngineLog("Model loaded: ", path, " ", ((float)(s * sizeof(float)))/1000.f, " kB - currently not optimising for duplicates");
        EngineLog("Time elapsed: ", time);
#endif

        return true;
    }
}

#endif
