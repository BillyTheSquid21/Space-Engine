#include "game/utility/ImageRead.h"

GifData GetImageData(const char* path) {
    SpaceImage::ImageMeta meta = SpaceImage::ReadImage(path);

    unsigned int frames = 0;
    for (int i = 0; i < meta.metaElements.size(); i++)
    {
        if (meta.metaElements[i].keyword == "frames")
        {
            frames = std::strtoul(meta.metaElements[i].data.c_str(), nullptr, 10);
        }
    }

    return { meta.width, meta.height, frames };
}