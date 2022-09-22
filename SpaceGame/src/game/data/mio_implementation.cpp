#include "game/data/mio_implementation.h"

int handle_error(const std::error_code& error);

int MemmapRead(const char* path, mio::mmap_source& readMap)
{
    std::error_code error;
    readMap.map(path, error);
    if (error) { return handle_error(error); }
}

void Unmap(mio::mmap_source& map)
{
    map.unmap();
}

int handle_error(const std::error_code& error)
{
    const auto& errmsg = error.message();
    std::printf("error mapping file: %s, exiting...\n", errmsg.c_str());
    return error.value();
}