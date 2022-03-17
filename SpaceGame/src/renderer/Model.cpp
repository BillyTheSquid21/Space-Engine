#include "renderer/Model.h"

bool Model::LoadTextureVertexOBJ(const char* path, std::vector<TextureVertex>& verts, std::vector<unsigned int>& inds)
{
    auto start = std::chrono::system_clock::now();

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

    //Temp data
    std::vector<Component3f> positionDataTemp;
    std::vector<Component2f> uvDataTemp;

    auto mesh = scene->mMeshes[0];
    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        Component3f vert;
        vert.a = mesh->mVertices[i].x;
        vert.b = mesh->mVertices[i].y;
        vert.c = mesh->mVertices[i].z;
        positionDataTemp.push_back(vert);
    }

    bool hasUVS = false;
    if (&mesh->mTextureCoords[0][0] != NULL)
    {
        hasUVS = true;
    }
    if (hasUVS)
    {
        for (int i = 0; i < mesh->mNumVertices; i++)
        {
            Component2f uv;
            uv.a = mesh->mTextureCoords[0][i].x;
            uv.b = mesh->mTextureCoords[0][i].y;
            uvDataTemp.push_back(uv);
        }
    }
    for (int i = 0; i < mesh->mNumFaces; i++)
    {
        inds.push_back(mesh->mFaces[i].mIndices[0]);
        inds.push_back(mesh->mFaces[i].mIndices[1]);
        inds.push_back(mesh->mFaces[i].mIndices[2]);
    }
    for (int i = 0; i < mesh->mNumVertices; i++)
    {
        TextureVertex vert;
        vert.position = positionDataTemp[i];
        vert.uvCoords = { 0.0f, 0.0f };
        if (hasUVS)
        {
            vert.uvCoords = uvDataTemp[i];
        }
        verts.push_back(vert);
    }
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsedSeconds = end - start;

    EngineLog("Model loaded: ", path);
    EngineLog("Time elapsed: ", elapsedSeconds.count());
    if (elapsedSeconds.count() > 4)
    {
        EngineLog("Model took a LONG time to load");
    }

    return true;
}

void Model::TextureVertexModel::render()
{
   if (m_DataLoadedFtr._Is_ready())
   {
       m_DataLoaded = m_DataLoadedFtr.get();
   }
   if (!m_DataLoaded)
   {
       return;
   }
   m_Ren->commit(&m_Vertices[0], m_Vertices.size() * (sizeof(TextureVertex)/sizeof(float)), &m_Indices[0], m_Indices.size());
}