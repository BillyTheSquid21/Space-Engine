#pragma once
#ifndef IMAGE_READ_H
#define IMAGE_READ_H
#pragma comment(lib, "Ws2_32.lib")

#include <fstream>
#include <iostream>
#include <winsock.h>

struct ImageDim
{
    unsigned int width;
    unsigned int height;
};

ImageDim GetImageDimension(const char* path);

#endif