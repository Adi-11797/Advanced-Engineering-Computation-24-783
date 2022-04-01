#include <stdio.h>
#include <string>
#include <iostream>
#include <vector>

#include "yshash.h"
#include "simplebitmap.h"
#include "fssimplewindow.h"
#include "simplebitmaptemplate.h"

static int dim = 40;
static int h = 800;
static int w = 1200;

// Referencing from "test_hashtable.cpp"
template<>
struct ysHash<SimpleBitmap>
{
	// std::size_t is by default a 64-bit unsigned integer.
	std::size_t operator() (const SimpleBitmap& key) const
	{
		static unsigned int prime[7] = { 3,5,7,11,13,17,19};
		std::size_t sum = 0;
		int ctr = 0;
		int nElement = key.GetTotalNumComponent();
		auto itr = key.GetBitmapPointer();
		for (int p = 0; p < nElement; p++)
		{
			sum += (itr[p] * prime[ctr % 7]); // as mentioned in PS4, it cannot be cyclic multiples of 4, hence 7 was selected
			++ctr;
		}
		return sum;
	}
};

int main(int argc, char* argv[])
{
	SimpleBitmap bmp;
	ysHashTable<SimpleBitmap, int> hash_table;
	char* img = argv[1];
	bool read_status = bmp.LoadPng(argv[1]);
	int count = 0;

	if (argc < 2) // (argv[1] == NULL)
	{
		printf("\nUsage: ps4_1 <pngFileName.png>\n");
		return 0;
	}

	if (read_status == false)
	{
		printf("\nError: Failed to read a .PNG file.\n");
		return 0;
	}

	for (int i = 0; i < bmp.GetHeight(); i += 40)
	{
		for (int j = 0; j < bmp.GetWidth(); j += 40)
		{
			auto cut_img = bmp.CutOut(i, j, dim, dim);
			
			cut_img.Invert(); // alternatively bmp.Invert() can be done but outside nested loop

			auto bmp = cut_img.GetBitmapPointer();

			if (hash_table.find(cut_img) == hash_table.end())
			{
				hash_table.insert(cut_img, count);
				count++;
			}

			auto ID_hash = hash_table.find(cut_img);
			printf("\%c", ' ' + ID_hash->value);
		}
	}

	FsOpenWindow(0, 0, 1200, 800, 1);

	auto iterator = hash_table.begin();
	int x = 0, y = 0;
	int x_new = w / 40;
	int y_new = h / 40;

	while (true)
	{
		int y_fin = x / x_new;
		int x_fin = x % x_new;

		glRasterPos2i(x_fin * 40, y_fin * 40 + 20 - 1);
		glDrawPixels(dim, dim, GL_RGBA, GL_UNSIGNED_BYTE, iterator->key.GetBitmapPointer());

		FsSwapBuffers();

	
		++iterator;
		++x;

		if (iterator == hash_table.end() || y_fin >= y_new)
			break;
	}

	while (FSKEY_ESC != FsInkey())
	{
		FsPollDevice();
		FsSleep(1);
	}
	return 0;
}















//
//
//class BitMap
//{
//protected:
//
//	SimpleBitmap bmp;
//	ysHashTable <SimpleBitmap, int> table;
//
//	
//	int count;
//
//public:
//	BitMap();
//	virtual void Initialize(int argc, char* argv[]);
//	virtual void DrawWindow() const;
//	virtual void DrawBitMap(void);
//	virtual bool MustTerminate(void) const;
//};
//
//BitMap::BitMap()
//{
//}
//
//void BitMap::Initialize(int argc, char* argv[])
//{
// template<>
//struct ysHash<SimpleBitmap>
//{
//	// std::size_t is by default a 64-bit unsigned integer.
//	std::size_t operator() (const SimpleBitmap& key) const
//	{
//		static unsigned int prime[7] = { 3,5,7,11,13,17,19 };
//		std::size_t sum = 0;
//		int ctr = 0;
//		int nElement = key.GetTotalNumComponent();
//		auto itr = key.GetBitmapPointer();
//		for (int p = 0; p < nElement; p++)
//		{
//			sum += (itr[p] * prime[ctr % 7]); // as mentioned in PS4, it cannot be cyclic multiples of 4, hence 7 was selected
//			++ctr;
//		}
//		return sum;
//	}
//};

//	int x = bmp.GetWidth() / 40;
//	int y = bmp.GetHeight() / 40;
//	count = 0;
//
//	SimpleBitmap cut_bmp;
//
//	for (int j = 0; j < y; j++) 
//	{
//		std::vector<SimpleBitmap> tempVector;
//		for (int i = 0; i < x; i++)
//		{
//			cut_bmp = bmp.CutOut(i * 40, j * 40, 40, 40);
//			tempVector.push_back(cut_bmp);
//
//		}
//	}
//
//
//	for (int j = 0; j < y; j++)
//	{
//		std::vector<int> vector_Test;
//		for (int i = 0; i < x; i++)
//		{
//			cut_bmp = bmp.CutOut(i * 40, j * 40, 40, 40);
//			vector_Test.push_back(*f[cut_bmp]);
//		}
//		bmp_vector.push_back(vector_Test);
//	}
//
//	for (int i = 0; i < bmp_vector.size(); i++)
//	{
//		for (int j = 0; j < bmp_vector[i].size(); j++)
//		{
//
//			printf("%c", ' ' + (char)bmp_vector[i][j]);
//		}
//		printf("\n");
//	}
//}
//
//void BitMap::DrawWindow() const
//{
//}
//
//void BitMap::DrawBitMap(void)
//{
//}
//
//bool BitMap::MustTerminate(void) const
//{
//	return 0 != terminate;
//}
//
//
//int main(int argc,char *argv[])
//{
//	return 0;
//}
