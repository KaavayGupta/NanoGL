#include "tgaimage.h"

#include <iostream>

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);

int main()
{
	{
		TGAImage image(100, 100, 3);
		image.SetPixel(50, 50, red);
		image.WriteTGAImage("output.tga");
	}
	{
		TGAImage image;
		image.ReadTGAImage("output.tga");
		image.SetPixel(0, 0, white);
		image.WriteTGAImage("output_new.tga");
	}

	return 0;
}