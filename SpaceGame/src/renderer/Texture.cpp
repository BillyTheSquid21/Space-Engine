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
}

void Texture::generateTexture(unsigned int slot) {

    //Generate texture
    glGenTextures(1, &m_ID);

    //Set active slot and bind
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_ID);

    //Set params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //Assign to slot
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_LocalBuffer);
    m_Slot = slot;
}

void Texture::clearBuffer() {
    if (m_LocalBuffer) {
        stbi_image_free(m_LocalBuffer);
        m_LocalBuffer = nullptr;
        return;
    }
    EngineLog("Error unloading texture buffer!");
}

Texture::~Texture()
{
    deleteTexture();
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

//Texture Atlas
void Tex::TextureAtlasRGBA::loadTexture(const std::string& path, const std::string& name)
{
    stbi_set_flip_vertically_on_load(1);
    //if texture has been previously generated don't generate until buffer cleared
    if (m_AtlasBuffer) {
        EngineLog("Atlas already generated!");
        return;
    }

    TexBuffer texBuff;
    texBuff.name = name;
    texBuff.buffer = (TexChannel_4*)stbi_load(path.c_str(), &texBuff.width, &texBuff.height, &texBuff.bpp, 4);
    m_LocalBuffers.push_back(texBuff);
}

void Tex::TextureAtlasRGBA::clearBuffers()
{
    clearTextureBuffers();
    clearAtlasBuffers();
    m_AtlasRequest.clear();
}

void Tex::TextureAtlasRGBA::clearAtlasBuffers()
{
    if (m_AtlasBuffer) {
        delete[] m_AtlasBuffer;
        m_AtlasBuffer = nullptr;
        return;
    }
}

void Tex::TextureAtlasRGBA::clearTextureBuffers()
{
    for (int i = 0; i < m_LocalBuffers.size(); i++)
    {
        if (m_LocalBuffers[i].buffer) {
            stbi_image_free(m_LocalBuffers[i].buffer);
            m_LocalBuffers[i].buffer = nullptr;
        }
    }
    m_LocalBuffers.clear();
}

void Tex::TextureAtlasRGBA::generateAtlas()
{
    //Find new height of atlas
    int height = 0;
    int largestWidth = 0; //width of the largest image imported
    for (int i = 0; i < m_LocalBuffers.size(); i++)
    {
        height += m_LocalBuffers[i].height;
        if (m_LocalBuffers[i].width > largestWidth)
        {
            largestWidth = m_LocalBuffers[i].width;
        }
    }

    //Allocate based on new size
    constexpr unsigned char BPP = 4;
    int bufferSize = height * largestWidth;
    m_AtlasBuffer = new TexChannel_4[bufferSize];

    //Copy each buffer into the new buffer
    int currentHeight = 0;
    for (int i = 0; i < m_LocalBuffers.size(); i++)
    {
        TexBuffer buffer = m_LocalBuffers[i];
        for (int y = 0; y < buffer.height; y++)
        {
            for (int x = 0; x < buffer.width; x++)
            {
                int atlasIndex = (y + currentHeight) * largestWidth + x;
                int bufferIndex = y * buffer.width + x;
                m_AtlasBuffer[atlasIndex] = buffer.buffer[bufferIndex];
            }
        }
        currentHeight += buffer.height;
        
        UVTransform trans;
        trans.deltaV = (float) (currentHeight - buffer.height) / (float)height; //How much the texture is vertically offset by
        trans.scaleU = (float)   buffer.width  / (float)largestWidth;
        trans.scaleV = (float)   buffer.height / (float)height;
        
        m_AtlasRequest[buffer.name] = trans;   
    }
    clearTextureBuffers();
    m_Width = largestWidth; m_Height = height;
}

void Tex::TextureAtlasRGBA::generateTexture(unsigned int slot)
{
    //Generate texture
    glGenTextures(1, &m_ID);

    //Set active slot and bind
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_ID);

    //Set params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //Assign to slot
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Width, m_Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_AtlasBuffer);
    m_Slot = slot;

    //Changes cycle number - if cycle is different to models they will remap
    m_AtlasGenCycle++;
    m_RequestNewTextures = false;
}
