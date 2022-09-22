#pragma once
#ifndef MIO_IMPLEMENTATION_H
#define MIO_IMPLEMENTATION_H

#define NOMINMAX

#include "mio/mio.hpp"
#include "system_error" 
#include "cstdio" 
#include "cassert"
#include "algorithm"
#include "fstream"

//Implementation of memmap to read large files quicker
int MemmapRead(const char* path, mio::mmap_source& readMap);
void Unmap(mio::mmap_source& map);

#endif