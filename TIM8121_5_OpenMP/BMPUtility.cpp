#include "BMPUtility.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <bitset>
#include <omp.h>


int invertColorsInFile(std::string inFilePath, std::string outFilePath, int thrds) {
	// Read the BMP file into memory (RAM) as per the assignment requirement 
	std::ifstream inputFile(inFilePath, std::ios::binary | std::ios::ate);
	if (!inputFile.is_open()) {
		std::cerr << "ERROR: Open input file." << std::endl;
		return 1;
	}

	std::streamsize fileSize = inputFile.tellg();
	inputFile.seekg(0, std::ios::beg);
	std::vector<char> imageBMPData(fileSize);
	if (!inputFile.read(imageBMPData.data(), fileSize)) {
		std::cerr << "ERROR: Read input file image." << std::endl;
		return 1;
	}
	inputFile.close();

	int width = *(reinterpret_cast<int*>(&imageBMPData[18]));  // Pos: 18 width
	int height = *(reinterpret_cast<int*>(&imageBMPData[22])); // Pos: 22 height
	// Invert the colors using OpenMP
	invertColors(imageBMPData.data() + 54, width, height, thrds); // Pixel data starts from Pos: 54

	// Write the inverted image to the output file
	std::ofstream outputFile(outFilePath, std::ios::binary);
	if (!outputFile.is_open()) {
		std::cerr << "ERROR: Open invert out file." << std::endl;
		return 1;
	}

	// Write the BMP header with the inverted color image of the original image 
	outputFile.write(imageBMPData.data(), fileSize);
	outputFile.close();

	return 0;

}


void invertColors(char* imageBMPData, int width, int height, int thrds) {

#pragma omp parallel for num_threads(thrds) collapse(2)
	for (int i = 0; i < height; ++i) {
		for (int j = 0; j < width; ++j) {
			int index = 3 * (i * width + j); // Here the each pixel has 3 bytes (RGB)
#pragma omp critical
			{  //Use !~ comparsion as per the assignment requirement
				imageBMPData[index]		= ~imageBMPData[index];     // RBG-Red Spectrum 
				imageBMPData[index + 1] = ~imageBMPData[index + 1]; // RBG-Blue Spectrum
				imageBMPData[index + 2] = ~imageBMPData[index + 2]; // RBG-Green Spectrum
			}
		}
	}
}



void bmpToBinary(std::string filePath, std::string outputFileName) {

	// Open the BMP file in binary mode
	std::ifstream file(filePath, std::ios::binary);

	if (!file.is_open()) {
		std::cerr << "ERROR: Open input file" << std::endl;
		return;
	}

	// Read the BMP header which is the 54 bytes
	char bmpHeader[54];
	file.read(bmpHeader, 54);

	// Calculate the size of the image data using 24 bits per pixel
	int dataSize = *(int*)&bmpHeader[18] * *(int*)&bmpHeader[22] * 3;

	// Read the content of the BMP file into a character array
	char* imageBMPData = new char[dataSize];
	file.read(imageBMPData, dataSize);
	file.close();

	// Use vector processing to store the binary bitmap data
	std::vector<std::bitset<8>> binaryBMPData;

	// Convert each byte to a binary array and append it to the vector
	for (int i = 0; i < dataSize; ++i) {
		binaryBMPData.push_back(std::bitset<8>(imageBMPData[i]));
	}
	delete[] imageBMPData;

	// Create a text file to write the binary bitmap data (0s and 1s)
	std::ofstream outputFile(outputFileName, std::ios::out);
	if (!outputFile.is_open()) {
		std::cerr << "ERROR: Open output file!" << std::endl;
		return;
	}
	// Write the binary bitmap data (0s and 1s) to the text file
	for (int i = 0; i < binaryBMPData.size(); ++i) {
		outputFile << binaryBMPData[i] << " ";
	}

	outputFile.close();
}