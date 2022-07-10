#include "SpaceImage.hpp"

SpaceImage::ImageMeta SpaceImage::ReadImage(const char* path)
{
	SpaceImage::ImageMeta meta;
	meta.error = SpaceImage::ProcessFile(path, meta);
	return meta;
}

bool SpaceImage::IsPngChunkUseful(uint32_t type)
{
	using SpaceImage::PngChunkType;
	switch (type)
	{
	case (uint32_t)PngChunkType::IHDR:
		return true;
	case (uint32_t)PngChunkType::tEXt:
		return true;
	case (uint32_t)PngChunkType::IEND:
		return true;
	default:
		return false;
	}
}

void SpaceImage::GetPngTEXTMeta(PngChunk& chunk, ImageMeta& meta)
{
	//Read keyword
	std::string keyword = "";
	int keywordLength = 0;
	for (int i = 0; i < chunk.length; i++)
	{
		if (chunk.data[i] == 0)
		{
			break;
		}
		keyword += (char)chunk.data[i];
		keywordLength++;
	}

	//Read text
	std::string data = "";
	for (int i = keywordLength+1; i < chunk.length; i++)
	{
		data += (char)chunk.data[i];
	}

	meta.metaElements.push_back({ keyword, data });
}

void SpaceImage::GetPngIHDRMeta(PngChunk& chunk, ImageMeta& meta)
{
	//Get width
	uint32_t width = 0;
	for (int i = 0; i < 4; i++)
	{
		width |= chunk.data[i] << (3 - i) * 8;
	}
	//Get height
	uint32_t height = 0;
	for (int i = 0; i < 4; i++)
	{
		height |= chunk.data[i + 4] << (3 - i) * 8;
	}
	//Get bit depth
	uint8_t bitdepth = chunk.data[8];
	//Get color type
	uint8_t colorType = chunk.data[9];

	meta.width = width; meta.height = height; meta.bitdepth = bitdepth; meta.colorType = colorType;
}

bool SpaceImage::GetNextPngChunk(std::ifstream& infile, SpaceImage::PngChunk& chunk)
{
	using namespace SpaceImage;
	//Length
	uint8_t lengthBuffer[4];
	infile.read((char*)lengthBuffer, 4);
	uint32_t length = 0;
	for (int i = 0; i < 4; i++)
	{
		length |= lengthBuffer[i] << (3 - i) * 8;
	}

	//Type - if not useful return false
	uint8_t typeBuffer[4];
	infile.read((char*)typeBuffer, 4);
	uint32_t type = 0;
	for (int i = 0; i < 4; i++)
	{
		type |= typeBuffer[i] << (3 - i) * 8;
	}
	if (!IsPngChunkUseful(type))
	{
		//Advance
		int seekLength = length + 4;
		infile.seekg(seekLength, std::ios::cur);
		chunk.type = PngChunkType::NULLTYPE;
		return false;
	}
	//Clear any existing data
	if (chunk.data)
	{
		delete[] chunk.data;
	}

	//Data
	uint8_t* dataBuffer = new uint8_t[length];
	infile.read((char*)dataBuffer, length);

	//Skip CRC
	infile.seekg(4, std::ios::cur);

	chunk = { length, (PngChunkType)type, dataBuffer };
	return true;
}

bool SpaceImage::ProcessFile(const char* path, SpaceImage::ImageMeta& meta)
{
	using namespace SpaceImage;
	//open file
	uint8_t buffer[8];
	std::ifstream infile(path, std::ios::binary);

	//reads the png file header
	infile.read((char*)buffer, 8);

	//Check all png header values are met
	uint8_t check = 0;
	{
		check |= (buffer[0] == 137) << 0;
		check |= (buffer[1] == 80) << 1;
		check |= (buffer[2] == 78) << 2;
		check |= (buffer[3] == 71) << 3;
		check |= (buffer[4] == 13) << 4;
		check |= (buffer[5] == 10) << 5;
		check |= (buffer[6] == 26) << 6;
		check |= (buffer[7] == 10) << 7;
	}
	if (check != 255)
	{
		std::cout << "File is not a PNG! Only PNG is currently supported." << std::endl;
		return 0;
	}

	PngChunk chunk;
	const uint32_t MAX_DATA = UINT32_MAX;
	uint32_t currentDataOffset = 0;
	while (chunk.type != PngChunkType::IEND)
	{
		GetNextPngChunk(infile, chunk);
		if (chunk.type == PngChunkType::IHDR)
		{
			GetPngIHDRMeta(chunk, meta);
		}
		if (chunk.type == PngChunkType::tEXt)
		{
			GetPngTEXTMeta(chunk, meta);
		}
		if (currentDataOffset > MAX_DATA)
		{
			std::cout << "File was too large!" << std::endl;
			delete[] chunk.data;
			return 0;
		}
		currentDataOffset++;
	}

	//Clear data
	infile.close();
	if (chunk.data)
	{
		delete[] chunk.data;
	}
	return 1;
}