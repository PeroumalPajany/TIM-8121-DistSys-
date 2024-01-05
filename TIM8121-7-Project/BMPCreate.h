#pragma once
#ifndef BMP_CREATE_H
#define BMP_CREATE_H

#include <cstdint>
#include <string>

class BMPCreate {
public:
#pragma pack(push, 1)
	struct BMPHeader {
		uint16_t fileType{ 0x4D42 };
		uint32_t fileSize;
		uint16_t reserved1{ 0 };
		uint16_t reserved2{ 0 };
		uint32_t dataOffset;
	};

	struct BMPInfo {
		uint32_t headerSize{ 40 };
		int32_t width;
		int32_t height;
		uint16_t planes{ 1 };
		uint16_t bitCount{ 24 };
		uint32_t compression{ 0 };
		uint32_t imageSize{ 0 };
		int32_t xPixelsPerMeter{ 0 };
		int32_t yPixelsPerMeter{ 0 };
		uint32_t colorsUsed{ 0 };
		uint32_t colorsImportant{ 0 };
	};

	void createBMPFile(const std::string& filename, int width, int height, int r, int g, int b);
};

#endif
