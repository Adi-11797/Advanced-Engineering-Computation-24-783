
#include <string>
#include <fstream>
#include <iostream>
#include "simplebitmap.h"
#include "yspngenc.h"
using namespace std;


int main(int argc, char* argv[])
{
	int dim = 40;

	SimpleBitmap bmp;

	if (argv[1] == NULL) // (argc < 2)
	{
		printf("\nUsage: ps4_1 <pngFileName.png>\n");
		return 0;
	}

	bool read_status = bmp.LoadPng(argv[1]);
	if (read_status == false)
	{
		printf("\nError: Failed to read a .PNG file.\n");
		return 0;
	}

	int count = 0;

	for (int j = 0; j < bmp.GetHeight(); j += dim)
	{
		for (int i = 0; i < bmp.GetWidth(); i += dim)
		{
			FILE* f;

			// Create cropped instance of input image called "cut_img"
			auto cut_img = bmp.CutOut(i, j, dim, dim);

			// Convert counter value to string type and append ".png" to create the name of image to be saved
			std::string file_name = std::to_string(count) + ".png";

			// Pass a const char* that points to "file_name"
			char* name = const_cast<char*>(file_name.c_str());

			// Open for writing in binary mode. If the file exists, its contents are overwritten. 
			// If the file does not exist, it will be created
			f = fopen(name, "wb");

			// Save Image
			cut_img.SavePng(f);

			// Increment counter for next cut out
			count++;

			if (count == 200)
			{
				return 0;
			}
		}
	}
	return 0;
}