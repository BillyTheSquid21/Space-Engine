#include "renderer/Texture.h"

void Texture::loadTexture(const std::string& path)
{
    stbi_set_flip_vertically_on_load(1);
    //if texture has been previously generated don't generate until buffer cleared
    if (m_LocalBuffer) {
        EngineLog("Texture already generated!");
        return;
    }
    m_LocalBuffer = stbi_load(path.c_str(), &m_Width, &m_Height, &m_BPP, 4);
    EngineLog("Texture loaded from: ", path);
}

void Texture::generateTexture(unsigned int slot) {

    //Generate texture
    glGenTextures(1, &m_ID);

    //Set active slot and bind
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_ID);

    //Set params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //Assign to slot
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
    m_Slot = slot;
}

void Texture::clearBuffer() const {
    if (m_LocalBuffer) {
        stbi_image_free(m_LocalBuffer);
        return;
    }
    EngineLog("Error unloading texture buffer!");
}

Texture::~Texture()
{
    glDeleteTextures(1, &m_ID);
}

void Texture::bind() const
{
    glActiveTexture(GL_TEXTURE0 + m_Slot);
    glBindTexture(GL_TEXTURE_2D, m_ID);
}

void Texture::unbind() const
{   
    glActiveTexture(GL_TEXTURE0 + m_Slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}
