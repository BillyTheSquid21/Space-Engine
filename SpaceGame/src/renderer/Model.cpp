#include "renderer/Model.h"

static bool LoadMeshInternal(const char* path, SGRender::Mesh& mesh, SGRender::VertexType vertexType, int flags)
{
    using namespace SGRender;
    using namespace Model;

    //Access mesh
    mesh.unload();
    SGRender::MeshData& mData = mesh.getMesh();

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
        ProcessMeshFaces(aimesh, rawDataTemp, mData, vertexType, flags);
    }

    mData.vertices = rawDataTemp;
    mesh.setLoaded(true);

#if _DEBUG
    auto time = EngineTimer::EndTimer(start);
    EngineLog("Model loaded: ", path, " ", mData.vertices.size() * sizeof(float), " bytes - currently not optimising for duplicates");
    EngineLog("Time elapsed: ", time);
#endif
    return true;
}

//Also gets names of material textures
static bool LoadModelInternal(const char* path, Model::Model& model, SGRender::VertexType vertexType, int flags)
{
    using namespace SGRender;
    using namespace Model;

    //Access mesh
    model.mesh.unload();
    SGRender::MeshData& mData = model.mesh.getMesh();

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

    //Get materials
    if (tree->mNumMaterials)
    {
        aiMaterial* mat = tree->mMaterials[0];

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
                model.diffuseTexture = texName.C_Str();
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
                model.normalTexture = texName.C_Str();
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
                model.specularTexture = texName.C_Str();
#if _SHOW_MODEL_DEBUG
                EngineLogOk("Specular Texture: ", texName.C_Str());
#endif
            }
        }
    }

    //Stores floats in order of normal reading (pos -> uv -> norm -> color - > tangent)
    std::vector<float> rawDataTemp;

    //Iterate over all meshes in this scene
    for (int m = 0; m < tree->mNumMeshes; m++) {

        const aiMesh* aimesh = tree->mMeshes[m];
        ProcessMeshFaces(aimesh, rawDataTemp, mData, vertexType, flags);
    }

    mData.vertices = rawDataTemp;
    model.mesh.setLoaded(true);

#if _DEBUG
    auto time = EngineTimer::EndTimer(start);
    EngineLog("Model loaded: ", path, " ", mData.vertices.size() * sizeof(float), " bytes - currently not optimising for duplicates");
    EngineLog("Time elapsed: ", time);
#endif
    model.vertexType = vertexType;
    return true;
}

bool Model::LoadModel(const char* path, SGRender::Mesh& mesh, SGRender::VertexType vertexType)
{
    return LoadMeshInternal(path, mesh, vertexType, 0);
}

bool Model::LoadModel(const char* path, SGRender::Mesh& mesh, SGRender::VertexType vertexType, int flags)
{
    return LoadMeshInternal(path, mesh, vertexType, flags);
}

bool Model::LoadModel(const char* path, Model& model, SGRender::VertexType vertexType)
{
    return LoadModelInternal(path, model, vertexType, 0);
}

bool Model::LoadModel(const char* path, Model& model, SGRender::VertexType vertexType, int flags)
{
    return LoadModelInternal(path, model, vertexType, flags);
}

void Model::ProcessMeshFaces(const aiMesh* aimesh, std::vector<float>& verts, SGRender::MeshData& mData, SGRender::VertexType vertexType, int flags)
{
    //Iterate over all faces in this mesh
    for (int j = 0; j < aimesh->mNumFaces; j++) {

        auto const& face = aimesh->mFaces[j];

        //Keep faces correct for primitive type
        int indCount = face.mNumIndices;
        if ((indCount != 3 && (flags & M_TRIANGLES_ONLY))
            || (indCount != 2 && (flags & M_LINES_ONLY)))
        {
            continue;
        }

        //Iterate face vertices
        for (int k = 0; k < 3; ++k) {

            //Get indice
            unsigned int indice = face.mIndices[k];

            //Assume always has position
            auto const& vertex = aimesh->mVertices[indice];
            verts.insert(verts.end(), { vertex.x, vertex.y, vertex.z });

            //Next do UV
            if (aimesh->HasTextureCoords(0) && (vertexType & SGRender::V_HAS_UVS)) {
                // The following line fixed the issue for me now:
                auto const& uv = aimesh->mTextureCoords[0][indice];
                verts.insert(verts.end(), { uv.x, uv.y });
            }
            else if ((flags & M_FILL_MISSING) && (vertexType & SGRender::V_HAS_UVS))
            {
                verts.insert(verts.end(), { 0.0f, 0.0f });
            }

            //Next Normal
            if (aimesh->HasNormals() && (vertexType & SGRender::V_HAS_NORMALS))
            {
                auto const& normal = aimesh->mNormals[indice];
                verts.insert(verts.end(), { normal.x, normal.y, normal.z });
            }
            else if ((flags & M_FILL_MISSING) && (vertexType & SGRender::V_HAS_NORMALS))
            {
                verts.insert(verts.end(), { 0.0f, 0.0f });
            }

            //Next Color
            if (aimesh->HasVertexColors(0) && (vertexType & SGRender::V_HAS_COLOR))
            {
                auto const& color = aimesh->mColors[indice];
                verts.insert(verts.end(), { color->r, color->g, color->b, color->a });
            }
            else if ((flags & M_FILL_MISSING) && (vertexType & SGRender::V_HAS_COLOR))
            {
                verts.insert(verts.end(), { 0.0f, 0.0f, 0.0f, 0.0f });
            }

            //Next Tangents
            if (aimesh->HasTangentsAndBitangents() && (vertexType & SGRender::V_HAS_TANGENTS))
            {
                auto const& tangent = aimesh->mTangents[indice];
                verts.insert(verts.end(), { tangent.x, tangent.y, tangent.z });
            }
            else if ((flags & M_FILL_MISSING) && (vertexType & SGRender::V_HAS_TANGENTS))
            {
                verts.insert(verts.end(), { 0.0f, 0.0f, 0.0f, 0.0f });
            }

            //Now push back indice
            mData.indices.push_back(mData.indices.size());
        }
    }
}