#pragma once
#ifndef SG_OPTIONS_H
#define SG_OPTIONS_H

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/ostreamwrapper.h"
#include "filesystem"
#include "SGUtil.h"
#include "fstream"

namespace SGOptions
{
	//Here be the global data, initialised to defaults
	extern int WIDTH;
	extern int HEIGHT;
	extern bool VSYNC_ENABLED;
	extern bool WINDOWED;

	bool LoadOptions();
	bool SaveOptions();
}

#endif