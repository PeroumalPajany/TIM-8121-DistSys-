#pragma once

#ifndef BMP_UTILITY_H
#define BMP_UTILITY_H

#include <vector>
#include <bitset>
#include <tuple>

int writeOutputBMPFile(char* imageBMPData, int fileSize, std::string outFilePath);
	
std::tuple <std::vector<char>, int> getBMPImageFromInFile(std::string inFilePath);

char* invertColorsInImage(char* imageBMPData, int thrds);

void invertColors(char* imageBMPData, int width, int height, int thrds);

void bmpToBinary(std::string filePath, std::string outputFileName);

bool isImageArraysEqual(const char* imageArray1, const char* imageArray2, size_t size);

#endif 
