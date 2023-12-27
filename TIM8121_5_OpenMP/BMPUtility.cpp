#include "BMPUtility.h"
#include <iostream>
#include <fstream>
#include <cstdint>
#include <bitset>
#include <omp.h>


int invertColorsInFile(std::string inFilePath, std::string outFilePath, int thrds) {
    // Read the BMP file into memory
    std::ifstream inputFile(inFilePath, std::ios::binary | std::ios::ate);
    if (!inputFile.is_open()) {
        std::cerr << "ERROR: Open input file." << std::endl;
        return 1;
    }

    std::streamsize fileSize = inputFile.tellg();
    inputFile.seekg(0, std::ios::beg);
    std::vector<char> imageData(fileSize);
    if (!inputFile.read(imageData.data(), fileSize)) {
        std::cerr << "ERROR: Read input file image." << std::endl;
        return 1;
    }
    inputFile.close();

    int width = *(reinterpret_cast<int*>(&imageData[18]));  // Pos: 18 width
    int height = *(reinterpret_cast<int*>(&imageData[22])); // Pos: 22 height

    // Invert the colors using OpenMP
    invertColors(imageData.data() + 54, width, height,thrds); // Pixel data starts from Pos: 54

    // Write the inverted image data to the output file
    std::ofstream outputFile(outFilePath, std::ios::binary);
    if (!outputFile.is_open()) {
        std::cerr << "ERROR: Open invert out file." << std::endl;
        return 1;
    }

    // Write the BMP header with the inverted color of the original image 
    outputFile.write(imageData.data(), fileSize);
    outputFile.close();

    return 0;

}


void invertColors(char* imageData, int width, int height, int thrds) {
    int tthrds = 0;
    #pragma omp parallel for num_threads(thrds) collapse(2)
    for (int i = 0; i < height; ++i) {
        tthrds = omp_get_num_threads();
        for (int j = 0; j < width; ++j) {
            int index = 3 * (i * width + j); // Each pixel has 3 bytes (RGB)
            // Invert each byte using the ~ operator
            #pragma omp critical
            {
                imageData[index] = ~imageData[index];         // RBG-Red Spectrum
                imageData[index + 1] = ~imageData[index + 1]; // RBG-Green Spectrum
                imageData[index + 2] = ~imageData[index + 2]; // RBG-Blue Spectrum
            }
        }
    }
    
    //std::cout << "TOTAL THREADS USED BY INVERSION=" << tthrds << std::endl;
}



void bmpToBinary(std::string filePath, std::string outputFileName) {
    
    // Open the BMP file in binary mode
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "ERROR: Open input file" << std::endl;
        return;
    }

    // Read the BMP header (54 bytes)
    char bmpHeader[54];
    file.read(bmpHeader, 54);

    // Calculate the size of the image data (assuming 24 bits per pixel)
    int dataSize = *(int*)&bmpHeader[18] * *(int*)&bmpHeader[22] * 3;

    // Read the content of the BMP file into a character array
    char* imageData = new char[dataSize];
    file.read(imageData, dataSize);
    file.close();

    // Create a vector to store the binary data
    std::vector<std::bitset<8>> binaryData;

    // Convert each byte to a binary array and append it to the vector
    for (int i = 0; i < dataSize; ++i) {
        binaryData.push_back(std::bitset<8>(imageData[i]));
    }
    delete[] imageData;

    // Create a text file to write the binary data (0s and 1s)
    std::ofstream outputFile(outputFileName, std::ios::out);
    if (!outputFile.is_open()) {
        std::cerr << "ERROR; Open output file!" << std::endl;
        return;
    }
    // Write the binary data to the text file
    for (int i = 0; i < binaryData.size(); ++i) {
        outputFile << binaryData[i] << " ";
    }

    outputFile.close();
}