#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include "BMPUtility.h"
#include "BMPCreate.h"
#include <omp.h>
using namespace std;

const string baseFile_bmp = "T1M8121_BASE.bmp";
const string image_bmp = "Image.bmp";
const string DotBMP = ".bmp";
const string DotTXT = ".txt";
int t_threads = 2;


string getInvFileName(string fileName, string type) {
	return fileName.substr(0, fileName.find(".")) + "_INVERTED_" + type;
}

/*********************************************************************************
* CREATE BMP FILE
**********************************************************************************/

int createBasefile(string fileName, int sizeInKB, int r, int b, int g) {


	int pixels = (sizeInKB * 1024) / 3;
	int width = static_cast<int>(sqrt(pixels));
	int height = pixels / width;
	BMPCreate writeObj;
	writeObj.createBMPFile(fileName, width, height, r, b, g);  //R, B, G	
	bmpToBinary(fileName, fileName.substr(0, fileName.find(".")) + DotTXT);
	std::cout << "Bitmap image of 150KB created: " << fileName << endl;
	return 0;

}


/*********************************************************************************
* FAULT TOLERANT: RE-SUBMISSION WITH COMPARISON
**********************************************************************************/
int invertTaskResubmitFT(string imageFile) {

	auto beg = std::chrono::high_resolution_clock::now();
	string invertedFile = getInvFileName(imageFile, "TR");
	
	tuple <std::vector<char>, int> imageContent;
	imageContent = getBMPImageFromInFile(imageFile);
	vector<char> imageData = std::get<0>(imageContent);
	int fileSize = std::get<1>(imageContent);
	char* imageArray = imageData.data();

	bool areArraysEqual = false;
	while (!areArraysEqual) {

		// Make a copy of the original array for comparison
		std::vector<char> originalImageData(imageData);
		char* originalArray = originalImageData.data();

#pragma omp parallel sections
		{
#pragma omp section
			{
				invertColorsInImage(imageArray, t_threads);    //Fault Tolerant Main Thread-1 in one Core
			}

#pragma omp section
			{
				invertColorsInImage(originalArray, t_threads); //Fault Tolerant Main Thread-2 in next core
			}
		}
		areArraysEqual = isImageArraysEqual(imageArray, originalArray, fileSize);
		if (!areArraysEqual) {
			imageData = originalImageData;
			imageArray = imageData.data();
		}
	}

	writeOutputBMPFile(imageArray, fileSize, invertedFile + DotBMP);
	bmpToBinary(invertedFile + DotBMP, invertedFile + DotTXT);

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg);
	std::cout << invertedFile + DotBMP << " BMP inverted <Resubmission & Comparison> in: " << duration.count() << " ms" << endl;

	return 0;
}


/*********************************************************************************
* FAULT TOLERANT: REDUNDANCY VOTING
**********************************************************************************/
int  invertImageUsingRedundancyVotingFT(string imageFile) {

	auto beg = std::chrono::high_resolution_clock::now();
	string invertedFile = getInvFileName(imageFile, "RT");

	tuple <std::vector<char>, int> imageContent;
	imageContent = getBMPImageFromInFile(imageFile);
	vector<char> imageData = std::get<0>(imageContent);
	int fileSize = std::get<1>(imageContent);
	char* imageArray = imageData.data();
	std::vector<char> duplicatedImage(imageData);
	char* duplicatedImageArray = duplicatedImage.data(); // Duplicate the image for redundancy processing


#pragma omp parallel sections
	{
#pragma omp section
		{
			invertColorsInImage(imageArray, t_threads); //Fault Tolerant Main Thread-1 in 1 Core	
#pragma omp critical
			{
				//std::cout << "Thread " << omp_get_thread_num() << " inverted the original bitmap image.\n";
			}
		}

#pragma omp section
		{
			invertColorsInImage(duplicatedImageArray, t_threads); //Fault Tolerant Main Thread-1 in another Core		
#pragma omp critical
			{
				//std::cout << "Thread " << omp_get_thread_num() << " inverted the duplicated bitmap image.\n";
			}
		}
	}

	// Logic for Voting mechanism: Compare the results and choose the correct one from redundancy checking
	for (int i = 0; i < fileSize; ++i) {
		if (imageArray[i] != duplicatedImage[i]) {
			int originalCount = 0;
			int duplicatedCount = 0;

			for (int j = 0; j < fileSize; ++j) { // Count occurrences in the original image
				if (imageArray[j] == imageArray[i]) {
					originalCount++;
				}
			}
			for (int j = 0; j < fileSize; ++j) { // Count occurrences in the duplicated image
				if (duplicatedImage[j] == imageArray[i]) {
					duplicatedCount++;
				}
			}

			if (originalCount >= duplicatedCount) { // Choose the value that occurred more frequently
				imageArray[i] = duplicatedImage[i];
			}

#pragma omp critical
			{
				//std::cout << "Thread " << omp_get_thread_num() << " voted for the value at index " << i << ".\n";
			}
		}
	}

	writeOutputBMPFile(imageArray, fileSize, invertedFile + DotBMP);
	bmpToBinary(invertedFile + DotBMP, invertedFile + DotTXT);

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg);
	std::cout << invertedFile + DotBMP << " BMP inverted <Redundancy Voting> in: " << duration.count() << " ms" << endl;

	return 0;

}

/*********************************************************************************
* FAULT TOLERANT: CHECK POINT RESTART
**********************************************************************************/
int invertImageUsingCheckPointRestartFT(string imageFile) {

	auto beg = std::chrono::high_resolution_clock::now();
	string invertedFile = getInvFileName(imageFile, "CP");

	tuple <std::vector<char>, int> imageContent;
	imageContent = getBMPImageFromInFile(imageFile);
	vector<char> imageData = std::get<0>(imageContent);
	int fileSize = std::get<1>(imageContent);
	char* imageArray = imageData.data();
	std::vector<char> duplicatedImage(imageData);
	char* duplicatedImageArray = duplicatedImage.data(); //Duplicated image

	int width = *(reinterpret_cast<int*>(&imageArray[18]));  // Pos: 18 width


	bool errorDetected = true;
	while (errorDetected) { // Process until no error is detected
		errorDetected = false;
#pragma omp parallel
		{
			// Checkpointing at mid section
#pragma omp master
			{
				if (width % 2 == 0) {
					int halfWidth = width / 2;
					try {
						invertColorsInImage(imageArray, t_threads);
						writeOutputBMPFile(imageArray, fileSize, invertedFile + "_temp.bmp");
					}
					catch (exception& e) {
						errorDetected = true;
						std::cout << "Exception at checkpoint restart: " << e.what() << endl;
					}
				}
			}
#pragma omp barrier
			try {
				invertColorsInImage(imageArray, t_threads);
			}
			catch (exception& e) {
				errorDetected = true;
				std::cout << "Exception at checkpoint restart : " << e.what() << endl;
			}
		}
	}

	// Save the modified image
	writeOutputBMPFile(imageArray, fileSize, invertedFile + DotBMP);;
	bmpToBinary(invertedFile + DotBMP, invertedFile + DotTXT);

	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - beg);
	std::cout << invertedFile + DotBMP << " BMP inverted <CheckPoint and Restart> in: " << duration.count() << " ms" << endl;

	return 0;

}

int getFileOption() {
	int foption;
	while (true) {
		std::cout << "FILE: Enter 1 (New 150 kb) or 2. to provide a file name >>> ";
		std::cin >> foption;
		if (foption == 1 || foption == 2) {
			break;
		}
		else {
			std::cout << "Invalid input. Enter 1 or 2" << std::endl;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
	return foption;
}
int getProcessingOption() {
	int foption;
	while (true) {
		std::cout << "FAULT-TOLERANT: Enter 1 (Resubmission) 2. (Redundancy), 3. (Checkpoint), 4 for all  ";
		std::cin >> foption;
		if (foption >= 1 || foption <= 4) {
			break; // 
		}
		else {
			std::cout << "Invalid input. Enter 1,2,3,or 4" << std::endl;
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}
	return foption;
}

string getFileName() {

	string fileName;
	while (true) {
		std::cout << "Enter a bitmap file name located in the application directory";
		cin >> fileName;
		if (!fileName.empty()) {
			break;
		}
		else {
			std::cout << "Invalid input. Please enter a valid bit map file." << std::endl;
		}
	}
	return fileName;

}
bool isBitMapFile(string fileName) {

	std::ifstream file(fileName, std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Cannot open the file --> " << fileName << std::endl;
		return false;
	}
	char signature[2];
	file.read(signature, 2);
	if (signature[0] == 'B' && signature[1] == 'M') {
		return true;
	}
	else {
		std::cout << fileName << "<-- is not a valid bit map file." << std::endl;
		return false;
	}
}
/****************************************************************
					M A I N  (Image Inversion)
*****************************************************************/
int main() {

	auto beg = std::chrono::high_resolution_clock::now();

	std::cout << "**********************************************************" << std::endl;
	std::cout << "* OpenMP execution w/ different Fault Tolerant Approaches " << std::endl;
	std::cout << "**********************************************************" << std::endl;
	std::cout << "* 1. Resubmit w/compare " << "2. Redundancy w/voting" << "3. Checkpoint w/restart" << std::endl;
	std::cout << "**********************************************************" << std::endl;

	int foption = getFileOption(); // Get custom file or create bitmap file
	string fileName;
	if (foption == 1) {
		fileName = baseFile_bmp;
	}
	else {
		fileName = getFileName();
		if (!isBitMapFile(fileName)) {
			fileName = getFileName(); // Get the existing bmp file to convert
		}
	}
	int option = getProcessingOption(); // Fault tolerant processing approaches option


	try {
		if (foption == 1) {
			createBasefile(fileName, 150, 150, 0, 0); //Create a new 150KB bmp file with RBG {150,0,0}
		}
		switch (option) {
		case 1:
			invertTaskResubmitFT(fileName); // Compare & Resubmit 
			break;
		case 2:
			invertImageUsingRedundancyVotingFT(fileName); //Redundancy & Voting
			break;
		case 3:
			invertImageUsingCheckPointRestartFT(fileName); //Checkpoint & Restart
			break;
		default:
			invertTaskResubmitFT(fileName);
			invertImageUsingRedundancyVotingFT(fileName);
			invertImageUsingCheckPointRestartFT(fileName);
		}

	}
	catch (exception& e) {
		std::cout << "Exception: " << e.what() << endl;
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg);
	std::cout << std::endl << " Processed in ms:" << duration.count() << std::endl;
	std::cout << "**********************************************************" << std::endl;

	return 0;
}
