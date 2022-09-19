#pragma once

#include "GLClasses.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "renderer/Vertex.hpp"

namespace Tex
{
    class Texture
    {
    public:
        Texture() : m_ID(0), m_FilePath("res/debug.png"), m_LocalBuffer(nullptr), m_Width(0), m_Height(0), m_BPP(0) {};
        ~Texture();

        /**
        * Load texture from path
        */
        void loadTexture(const std::string& path);
        /**
        * Load texture from path, specify if flipped on load
        */
        inline void loadTexture(const std::string& path, bool flip);
        /**
        * Generate texture in a given slot
        */
        void generateTexture(unsigned int slot);
        /**
        * Generate texture in a given slot from an external buffer
        */
        void generateTexture(unsigned int slot, void* buffer);
        /**
        * Generate texture in a given slot from an external buffer that is wrapped instead of clamped to edge
        */
        void generateTextureWrap(unsigned int slot, void* buffer); //TODO have 1 func
        void bind() const;
        void unbind() const;
        /**
        * Clear the local buffer - should do after has been generated as a glTexture
        */
        void clearBuffer();

        /**
        * Deletes the texture on the GPU
        */
        void deleteTexture() const { glDeleteTextures(1, &m_ID); }

        unsigned int getID() const { return m_ID; }

        void setWidth(int width) { m_Width = width; }
        void setHeight(int height) { m_Height = height; }
        void setBPP(int bpp) { m_BPP = bpp; }
        int width() const { return m_Width; }
        int height() const { return m_Height; }

    private:
        unsigned int m_ID; unsigned int m_Slot = 0;
        std::string m_FilePath;
        unsigned char* m_LocalBuffer = nullptr;
        int m_Width, m_Height, m_BPP;

    };

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
        ~TextureAtlasRGBA() { clearBuffers(); deleteTextures(); };
        void loadTexture(const std::string& path, const std::string& name);
        void generateAtlas();
        void generateTexture(unsigned int slot);
        void clearBuffers();
        void clearTextureBuffers();
        void clearAtlasBuffers();
        void deleteTextures() const { glDeleteTextures(1, &m_ID); }
        void requestNewTextures() { m_RequestNewTextures = true; clearBuffers(); }
        bool shouldRequestNewTextures() const { return m_RequestNewTextures; }

        void bind() const { glActiveTexture(GL_TEXTURE0 + m_Slot); glBindTexture(GL_TEXTURE_2D, m_ID); }
        void unbind() const {glActiveTexture(GL_TEXTURE0 + m_Slot); glBindTexture(GL_TEXTURE_2D, 0);}

        unsigned char currentTextureCycle() { return m_AtlasGenCycle; }

        template<typename T>
        UVTransform mapModelVerts(T* vertices, unsigned int vertCount, std::string texName)
        {
            static_assert(std::is_base_of<SGRender::TVertex, T>::value, "Must be a texture vertex!");
            
            if (m_AtlasRequest.find(texName) == m_AtlasRequest.end())
            {
                EngineLog("Requested texture is not in atlas! ", texName);
                return { 0.0f, 0.0f, 0.0f };
            }
            UVTransform trans = m_AtlasRequest.at(texName);
            for (int i = 0; i < vertCount; i++)
            {
                vertices[i].uvCoords.x *= trans.scaleU;
                vertices[i].uvCoords.y *= trans.scaleV;
                vertices[i].uvCoords.y += trans.deltaV;
            }
            return trans;
        }

        template<typename T>
        static void unmapModelVerts(T* vertices, unsigned int vertCount, std::string texName, UVTransform trans)
        {
            static_assert(std::is_base_of<SGRender::TVertex, T>::value, "Must be a texture vertex!");
            for (int i = 0; i < vertCount; i++)
            {
                vertices[i].uvCoords.y -= trans.deltaV;
                vertices[i].uvCoords.x /= trans.scaleU;
                vertices[i].uvCoords.y /= trans.scaleV;
            }
        }

    private:
        std::vector<TexBuffer> m_LocalBuffers;
        TexChannel_4* m_AtlasBuffer = nullptr;
        std::unordered_map<std::string, UVTransform> m_AtlasRequest;
        unsigned int m_Slot; unsigned int m_ID;
        unsigned int m_Width; unsigned int m_Height;
        unsigned char m_AtlasGenCycle = 0; //Goes up each time a new texture is generated - should let models know to remap
        bool m_RequestNewTextures = false; //Checks whether objects linked to atlas should load their textures again - means only textures in buffer should be textures needed
    };
}