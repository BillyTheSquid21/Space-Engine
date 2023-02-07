#include "renderer/Texture.h"

void Tex::Texture::loadTexture(const std::string& path)
{
    loadTexture(path, 4, true);
}

inline void Tex::Texture::loadTexture(const std::string& path, int bpp, bool flip)
{
    stbi_set_flip_vertically_on_load(flip);
    //if texture has been previously generated don't generate until buffer cleared
    if (m_LocalBuffer) {
        EngineLog("Texture already generated!");
        return;
    }
    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, bpp);
    m_BPP = bpp;
}

void Tex::Texture::generateTexture(int slot, int flag) {

    generateTexture(slot, m_LocalBuffer, flag);
}

void Tex::Texture::generateTexture(int slot, void* buffer, int flag) {

    //Generate texture
    glGenTextures(1, &m_ID);

    //Set active slot and bind
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_ID);

    //Set params from flags

    //Filter params - linear is one more than nearest so add
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST + (flag & T_FILTER_LINEAR));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST + (flag & T_FILTER_LINEAR));

    //Wrap params
    if (flag & T_WRAP_TEXTURE)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_WRAP_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_WRAP_BORDER);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    //Gamma correct or not, and decide BPP format
    if (flag & T_GAMMA_CORRECT)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, 
            (GL_SRGB_ALPHA)*(m_BPP==4) + (GL_SRGB)*(m_BPP==3), 
            m_Width, m_Height, 0, 
            (GL_RGBA*(m_BPP == 4)) + (GL_RGB*(m_BPP == 3)),
            GL_UNSIGNED_BYTE, buffer
        );
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, 
            (GL_RGBA8)*(m_BPP==4) + (GL_RGB8)*(m_BPP==3) + (GL_LUMINANCE)*(m_BPP==1),
            m_Width, m_Height, 0, 
            (GL_RGBA * (m_BPP == 4)) + (GL_RGB * (m_BPP == 3)),
            GL_UNSIGNED_BYTE, buffer
        );
    }

    //Mipmaps
    if (flag & T_GENERATE_MIPMAPS)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    m_Slot = slot;
}

void Tex::Texture::clearBuffer() {
    if (m_LocalBuffer) {
        stbi_image_free(m_LocalBuffer);
        m_LocalBuffer = nullptr;
        return;
    }
}

Tex::Texture::~Texture()
{
    deleteTexture();
}
