#pragma once

#include "stdint.h"
#include "GLClasses.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "renderer/Vertex.h"

namespace Tex
{
    //Has prefix T_
    enum TFlag
    {
        T_FILTER_LINEAR =       0b00000001,
        T_WRAP_TEXTURE =        0b00000010,
        T_GAMMA_CORRECT =       0b00000100,
        T_GENERATE_MIPMAPS =    0b00001000
    };

    class Texture : public SGRender::TextureBase
    {
    public:
        Texture() : m_Width(0), m_Height(0), m_BPP(0) {};

        /**
        * Load texture from path
        */
        void loadTexture(const std::string& path);
        /**
        * Load texture from path, specify if flipped on load
        */
        inline void loadTexture(const std::string& path, int bpp, bool flip);
        /**
        * Generate texture in a given slot
        */
        void generateTexture(int slot, int flag);
        /**
        * Generate texture in a given slot from an external buffer
        */
        void generateTexture(int slot, void* buffer, int flag);

        /**
        * Clear the texture on the cpu
        */
        void clearBuffer();

        /**
        * Deletes the texture on the GPU
        */
        void deleteTexture() { m_Resource = SGRender::TXResource::TXResource(0, 0); }

        GLuint id() const { return m_Resource.id(); }
        int32_t& getSlot() { return m_Slot; }
        void setWidth(int width) { m_Width = width; }
        void setHeight(int height) { m_Height = height; }
        void setBPP(int bpp) { m_BPP = bpp; }
        int width() const { return m_Width; }
        int height() const { return m_Height; }
        uint8_t* buffer() const { return m_LocalBuffer; }
        int bufferSize() const { return m_Width * m_Height * m_BPP; }

    private:
        uint8_t* m_LocalBuffer = nullptr;
        int32_t m_Width, m_Height, m_BPP;
    };
}