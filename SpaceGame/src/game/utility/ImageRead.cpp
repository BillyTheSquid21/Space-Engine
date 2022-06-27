#include "game/utility/ImageRead.h"

ImageDim GetImageDimension(const char* path) {
    std::ifstream in(path);
    unsigned int width, height;

    in.seekg(16);
    in.read((char*)&width, 4);
    in.read((char*)&height, 4);

    width = ntohl(width);
    height = ntohl(height);

    return { width, height };
}