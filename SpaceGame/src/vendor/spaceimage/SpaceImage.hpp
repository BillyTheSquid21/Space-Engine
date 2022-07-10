#pragma once
#ifndef META_READ_H
#define META_READ_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

namespace SpaceImage
{
	struct MetaElement
	{
		std::string keyword;
		std::string data;
	};

	struct ImageMeta
	{
		uint32_t width;
		uint32_t height;
		uint8_t bitdepth;
		uint8_t colorType;
		std::vector<MetaElement> metaElements;
		bool error = false; //Checks if file had an error - if not error is likely not from here
	};

	enum class PngChunkType
	{
		NULLTYPE = 0, IHDR = 0x49484452, tEXt = 0x74455874, IEND = 0x49454e44, IDAT = 0x49444154
	};
	
	struct PngChunk
	{
		uint32_t length;
		PngChunkType type = PngChunkType::NULLTYPE;
		uint8_t* data = nullptr;
	};

	ImageMeta ReadImage(const char* path);

	static inline bool ProcessFile(const char* path, ImageMeta& meta);

	//PNG
	static inline bool GetNextPngChunk(std::ifstream& infile, PngChunk& chunk);
	static inline bool IsPngChunkUseful(uint32_t type);
	static inline void GetPngIHDRMeta(PngChunk& chunk, ImageMeta& meta);
	static inline void GetPngTEXTMeta(PngChunk& chunk, ImageMeta& meta);
}

#endif