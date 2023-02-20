#include "renderer/Model.hpp"

void Model::ProcessMeshFaces(const aiMesh* aimesh, std::vector<float>& verts, Geometry::MeshData& mData, int flags)
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
            if (aimesh->HasTextureCoords(0) && (flags & SGRender::V_HAS_UVS)) {
                // The following line fixed the issue for me now:
                auto const& uv = aimesh->mTextureCoords[0][indice];
                verts.insert(verts.end(), { uv.x, uv.y });
            }
            else if ((flags & M_FILL_MISSING) && (flags & SGRender::V_HAS_UVS))
            {
                verts.insert(verts.end(), { 0.0f, 0.0f });
            }

            //Next Normal
            if (aimesh->HasNormals() && (flags & SGRender::V_HAS_NORMALS))
            {
                auto const& normal = aimesh->mNormals[indice];
                verts.insert(verts.end(), { normal.x, normal.y, normal.z });
            }
            else if ((flags & M_FILL_MISSING) && (flags & SGRender::V_HAS_NORMALS))
            {
                verts.insert(verts.end(), { 0.0f, 0.0f });
            }

            //Next Color
            if (aimesh->HasVertexColors(0) && (flags & SGRender::V_HAS_COLOR))
            {
                auto const& color = aimesh->mColors[indice];
                verts.insert(verts.end(), { color->r, color->g, color->b, color->a });
            }
            else if ((flags & M_FILL_MISSING) && (flags & SGRender::V_HAS_COLOR))
            {
                verts.insert(verts.end(), { 0.0f, 0.0f, 0.0f, 0.0f });
            }

            //Next Tangents
            if (aimesh->HasTangentsAndBitangents() && (flags & SGRender::V_HAS_TANGENTS))
            {
                auto const& tangent = aimesh->mTangents[indice];
                verts.insert(verts.end(), { tangent.x, tangent.y, tangent.z });
            }
            else if ((flags & M_FILL_MISSING) && (flags & SGRender::V_HAS_TANGENTS))
            {
                verts.insert(verts.end(), { 0.0f, 0.0f, 0.0f, 0.0f });
            }

            //Now push back indice
            mData.indices.push_back(mData.indices.size());
        }
    }
}