#pragma once
#ifndef FLAGS_H
#define FLAGS_H

#include <array>
#include <stdint.h>

typedef std::array<bool, std::numeric_limits<uint16_t>::max()> FlagArray; //known size at runtime


#endif