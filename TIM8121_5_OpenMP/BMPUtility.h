#pragma once

#ifndef BMP_UTILITY_H
#define BMP_UTILITY_H

#include <vector>
#include <bitset>

int invertColorsInFile(std::string inFilePath, std::string outFilePath,int thrds);

void invertColors(char* imageData, int width, int height, int thrds);

void bmpToBinary(std::string filePath, std::string outputFileName);

#endif 
