#pragma once
#ifndef FLAGS_H
#define FLAGS_H

#include "array"
#include "stdint.h"

//Define some flags that will be used in the code as well as outside
#define HAS_POKEMON 0x0

typedef std::array<bool, std::numeric_limits<uint16_t>::max()> FlagArray; //known size at runtime

#endif