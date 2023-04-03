#include "renderer/MatModel.h"

static bool LoadModelInternal(const char* path, Model::MatModel& model, SGRender::VertexType vertexType, int modelFlags)
{
    using namespace SGRender;
    using namespace Model;

    EngineLog("Loading material mesh: ", path);

#if _DEBUG
    auto start = EngineTimer::StartTimer();
#endif

    Assimp::Importer importer;
    const aiScene* tree = importer.ReadFile(path,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_SortByPType |
        aiProcess_CalcTangentSpace * ((vertexType & SGRender::V_HAS_TANGENTS) != 0) //Only bother if present
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

        //If name is blank, assign m_ + number in array
        if (name == aiString("") || name == aiString(" "))
        {
            name = "m_" + std::to_string(i);
        }

        //Set properties and convert where appropriate
        MatMesh& mmesh = model.meshes[i];
        mmesh.matName = name.C_Str();
        mmesh.mat.ambient = { amb.r, amb.g, amb.b };
        mmesh.mat.diffuse = { diff.r, diff.g, diff.b };
        mmesh.mat.specular = { spec.r, spec.g, spec.b };
        mmesh.mat.shininess = shin;

#if _SHOW_MODEL_DEBUG
        EngineLog("Material ", mmesh.matName, " loaded");
#endif

        //Add texture file names if exist
        //For now assume one texture per attribute
        if (mat->GetTextureCount(aiTextureType_DIFFUSE))
        {
            aiString texName;
            aiReturn ret;
            ret = mat->Get(AI_MATKEY_TEXTURE(aiTextureType_DIFFUSE, 0), texName);

            if (ret == 0)
            {
                model.diffuseTextures[mmesh.matName] = texName.C_Str();
#if _SHOW_MODEL_DEBUG
                EngineLogOk("Diffuse Texture: ", texName.C_Str());
#endif
            }
        }
        if (mat->GetTextureCount(aiTextureType_NORMALS))
        {
            aiString texName;
            aiReturn ret;
            ret = mat->Get(AI_MATKEY_TEXTURE(aiTextureType_NORMALS, 0), texName);

            if (ret == 0)
            {
                model.normalTextures[mmesh.matName] = texName.C_Str();
#if _SHOW_MODEL_DEBUG
                EngineLogOk("Normal Texture: ", texName.C_Str());
#endif
            }
        }
        if (mat->GetTextureCount(aiTextureType_SPECULAR))
        {
            aiString texName;
            aiReturn ret;
            ret = mat->Get(AI_MATKEY_TEXTURE(aiTextureType_SPECULAR, 0), texName);

            if (ret == 0)
            {
                model.specularTextures[mmesh.matName] = texName.C_Str();
#if _SHOW_MODEL_DEBUG
                EngineLogOk("Specular Texture: ", texName.C_Str());
#endif
            }
        }
    }

#if _SHOW_MODEL_DEBUG

    //List all the important info about the model
    EngineLog("Properties: ", flags);
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
        EngineLog(mname, " specular texture count: ", tree->mMaterials[i]->GetTextureCount(aiTextureType_SPECULAR));
        EngineLog(mname, " shininess texture count: ", tree->mMaterials[i]->GetTextureCount(aiTextureType_SHININESS));
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
        auto& mesh = model.meshes[mIndex].mesh.getMesh();
        ProcessMeshFaces(aimesh, rawDataTemp[mIndex], mesh, vertexType, modelFlags);
    }

    //Set each mesh data to rawDataTemp index
    for (int i = 0; i < model.meshes.size(); i++)
    {
        Mesh& m = model.meshes[i].mesh;
        MeshData& mData = m.getMesh();
        mData.vertices = rawDataTemp[i];
        m.setLoaded(true);
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
    EngineLog("Model loaded: ", path, " ", ((float)(s * sizeof(float))) / 1000.f, " kB - currently not optimising for duplicates");
    EngineLog("Time elapsed: ", time);
#endif
    model.vertexType = vertexType;
    return true;
}

bool Model::LoadModel(const char* path, MatModel& model, SGRender::VertexType vertexType)
{
    return LoadModelInternal(path, model, vertexType, 0);
}

bool Model::LoadModel(const char* path, MatModel& model, SGRender::VertexType vertexType, int modelFlags)
{
    return LoadModelInternal(path, model, vertexType, modelFlags);
}