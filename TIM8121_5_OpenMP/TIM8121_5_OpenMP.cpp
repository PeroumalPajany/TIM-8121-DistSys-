
#include <iostream>
#include <chrono>
#include "BMPUtility.h"
#include "BMPCreate.h"
#include <omp.h>
using namespace std;

/****************************************************************
I M A G E    P R O C E S S I N G    M A I N
*****************************************************************/
int main() {

	printf("START\n");
	auto beg = std::chrono::high_resolution_clock::now();
	int t_threads = 3; // Configrable threads to use for OpenMP

	/****************************************************************
	1. CREATE A BITMAP FILE OF 150KB 3 bytes per pixel (24 bits)
	*****************************************************************/
	const string baseFile_bmp = "T1M8121_BASE.bmp";
	const string baseFile_txt = "T1M8121_BASE.txt";
	int pixels = (150 * 1024) / 3;
	int width = static_cast<int>(sqrt(pixels));
	int height = pixels / width;
	// OPENMP parallel processing in the BMPCreate class file.
	BMPCreate writeObj;
	cout << "1. Bitmap image of 150KB created: " << baseFile_bmp << endl;
	writeObj.createBMPFile(baseFile_bmp, width, height, 0, 0, 150);  //R=0, B=0, G=150
	// Convert the base BMP file to text in 0/1 for comparsion using OPENMP 
	bmpToBinary(baseFile_bmp, baseFile_txt);


	/****************************************************************
	2. INVERT THE IMAGE (Uses ~ operator for image inversion
	*****************************************************************/
	const string invertedFile1_bmp = "T1M8121_INVERTED1.bmp";
	const string invertedFile1_txt = "T1M8121_INVERTED1.txt";
	// OPENMP file inversion using the ~ operator 
	if (invertColorsInFile(baseFile_bmp, invertedFile1_bmp, t_threads) == 0) {
		cout << "2. Base file image inverted" << endl;
	}
	bmpToBinary(invertedFile1_bmp, invertedFile1_txt);


	/****************************************************************
	3. INVERT THE IMAGE AGAIN TO MATCH THE STEP(1) IMAGE
	*****************************************************************/
	const string invertedFile2_bmp = "T1M8121_INVERTED2.bmp";
	const string invertedFile2_txt = "T1M8121_INVERTED2.txt";
	if (invertColorsInFile(invertedFile1_bmp, invertedFile2_bmp, t_threads) == 0) {
		cout << "3. Inverted to original image" << endl;
	}
	bmpToBinary(invertedFile2_bmp, invertedFile2_txt);


	/****************************************************************
	4. OPENMP file inversion using a colored file of 2701KB for secondary testing/processing
	*****************************************************************/
	if (invertColorsInFile("Image.bmp", "Image1.bmp", t_threads) == 0) {
		cout << "4a. 2701KB of a custom image file inverted" << endl;
	}
	if (invertColorsInFile("Image1.bmp", "Image2.bmp", t_threads) == 0) {
		cout << "4b Restore the 2701KB file to original state" << endl;
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg);
	cout << "Thread used:" << t_threads << " Duration in milliseconds:" << duration.count() << endl;
	
	printf("*********************************\n");
	printf("5. Execution Test Analysis\n");
	printf("*********************************\n");
	int myNumbers[10] = { 1,2,3,4,5,100,200,300,400,500 };
	for (int i : myNumbers) {
		auto beg = std::chrono::high_resolution_clock::now();
		invertColorsInFile("Image.bmp", "Image1.bmp", i);
		invertColorsInFile("Image1.bmp", "Image2.bmp", i);
		auto end = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg);
		cout << "Thread used:" << i << " Duration in milliseconds:" << duration.count() << endl;
	}

	printf("END\n");

	return 0;
}
