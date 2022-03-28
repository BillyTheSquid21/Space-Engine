#pragma once

#include "GLClasses.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "renderer/Vertex.hpp"

class Texture
{
public:
    Texture() : m_ID(0), m_FilePath("null"), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0) {};
    ~Texture();

    //Load texture into buffer from file path
    void loadTexture(const std::string& path);
    //Generate texture into slot
    void generateTexture(unsigned int slot);
    //Bind and Unbind texture
    void bind() const;
    void unbind() const;
    //Clear buffer image data is loaded into
    void clearBuffer() const;

    int width() const { return m_Width; }
    int height() const { return m_Height; }

private:
    unsigned int m_ID; unsigned int m_Slot = 0;
    std::string m_FilePath;
    unsigned char* m_LocalBuffer;
    int m_Width, m_Height, m_BPP;

};

namespace Tex
{
    struct alignas(1) TexChannel_4
    {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };

    struct alignas(1) TexChannel_3
    {
        unsigned char r;
        unsigned char g;
        unsigned char b;
    };

    struct TexBuffer
    {
        int width;
        int height;
        int bpp;
        TexChannel_4* buffer = nullptr;
        std::string name;
    };

    struct UVTransform
    {
        float deltaV;
        float scaleU; float scaleV;
    };

    class TextureAtlasRGBA
    {
    public:
        TextureAtlasRGBA() = default;
        ~TextureAtlasRGBA() { clearBuffers(); };
        void loadTexture(const std::string& path, const std::string& name);
        void generateAtlas();
        void generateTexture(unsigned int slot);
        void clearBuffers();
        void clearTextureBuffers();
        void clearAtlasBuffers();

        void bind() const { glActiveTexture(GL_TEXTURE0 + m_Slot); glBindTexture(GL_TEXTURE_2D, m_ID); }
        void unbind() const {glActiveTexture(GL_TEXTURE0 + m_Slot); glBindTexture(GL_TEXTURE_2D, 0);}

        template<typename T>
        void mapModelVerts(T* vertices, unsigned int vertCount, std::string texName)
        {
            static_assert(std::is_base_of<TextureVertex, T>::value, "Must be a texture vertex!");
            
            if (m_AtlasRequest.find(texName) == m_AtlasRequest.end())
            {
                EngineLog("Requested texture is not in atlas! ", texName);
                return;
            }
            UVTransform trans = m_AtlasRequest.at(texName);
            for (int i = 0; i < vertCount; i++)
            {
                vertices[i].uvCoords.a *= trans.scaleU;
                vertices[i].uvCoords.b *= trans.scaleV;
                vertices[i].uvCoords.b += trans.deltaV;
            }
        }

    private:
        std::vector<TexBuffer> m_LocalBuffers;
        TexChannel_4* m_AtlasBuffer = nullptr;
        std::unordered_map<std::string, UVTransform> m_AtlasRequest;
        unsigned int m_Slot; unsigned int m_ID;
        unsigned int m_Width; unsigned int m_Height;
    };
}