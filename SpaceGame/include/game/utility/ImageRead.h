#pragma once
#ifndef IMAGE_READ_H
#define IMAGE_READ_H

#include <fstream>
#include <iostream>
#include "spaceimage/SpaceImage.hpp"

struct ImageDim
{
    unsigned int width;
    unsigned int height;
};

ImageDim GetImageDimension(const char* path);

#endif