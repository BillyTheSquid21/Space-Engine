#include "game/utility/ImageRead.h"

ImageDim GetImageDimension(const char* path) {
    SpaceImage::ImageMeta meta = SpaceImage::ReadImage(path);

    return { meta.width, meta.height };
}