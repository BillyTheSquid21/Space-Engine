#pragma once
#ifndef IMAGE_READ_H
#define IMAGE_READ_H

#include <fstream>
#include <iostream>
#include "spaceimage/SpaceImage.hpp"

struct GifData
{
    unsigned int width;
    unsigned int height;
    unsigned int frames;
};

GifData GetImageData(const char* path);

#endif