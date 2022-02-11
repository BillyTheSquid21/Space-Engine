#pragma once

#include "GLClasses.h"
#include "stb/stb_image.h"

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

    inline int width() const { return m_Width; }
    inline int height() const { return m_Height; }

private:
    unsigned int m_ID; unsigned int m_Slot = 0;
    std::string m_FilePath;
    unsigned char* m_LocalBuffer;
    int m_Width, m_Height, m_BPP;

};