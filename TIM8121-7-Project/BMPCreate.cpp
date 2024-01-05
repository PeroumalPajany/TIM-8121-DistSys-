#include "BMPCreate.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <omp.h>

#pragma pack(pop)
void BMPCreate::createBMPFile(const std::string& filename, int width, int height, int r, int g, int b) {

	BMPHeader writeHeader;
	BMPInfo writeBMPInfo;

	writeBMPInfo.width = width;
	writeBMPInfo.height = height;

	// Calculate image size 
	int rowSize = ((width * 3 + 3) / 4) * 4;  // Each row is padded to be a multiple of 4 bytes
	writeHeader.fileSize = sizeof(BMPHeader) + sizeof(BMPInfo) + rowSize * height;
	writeHeader.dataOffset = sizeof(BMPHeader) + sizeof(BMPInfo);

	// Create the BMP file and write headers
	std::ofstream outFile(filename, std::ios::binary);
	outFile.write(reinterpret_cast<char*>(&writeHeader), sizeof(BMPHeader));
	outFile.write(reinterpret_cast<char*>(&writeBMPInfo), sizeof(BMPInfo));

	// Generate  pixel data and write the BMP file
#pragma omp parallel for num_threads(1) collapse(2) schedule(static)
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			uint8_t pixel[3] = { r, b, g };  // RGB values supplied by the input parameters
#pragma omp critical
			outFile.write(reinterpret_cast<char*>(&pixel), 3);
		}
		// Add padding to the data as needed
		for (int i = 0; i < rowSize - width * 3; ++i) {
			uint8_t paddingByte = 0;
#pragma omp critical
			outFile.write(reinterpret_cast<char*>(&paddingByte), 1);
		}
	}

}
