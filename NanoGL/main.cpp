#include "tgaimage.h"

#include <iostream>

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);

void Line(int x0, int y0, int x1, int y1, TGAImage& image, const TGAColor& color)
{
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) // If the line is steep, transpose the image
	{
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}
	if (x0 > x1)	// Ensure we all draw line from low to high
	{
		std::swap(x0, x1);
		std::swap(y0, y1);
	}
	int dx = x1 - x0;
	int dy = y1 - y0;
	int derror = std::abs(dy)*2;
	int error = 0;
	int y = y0;
	for (int x = x0; x <= x1; x++)
	{
		if (steep)
			image.SetPixel(y, x, color);	// Detranspose, if transposed
		else
			image.SetPixel(x, y, color);

		error += derror;
		if (error > dx)
		{
			y += (y1 > y0 ? 1 : -1);
			error -= dx * 2;
		}
	}
}

int main()
{
	TGAImage image(100, 100, 3);

	Line(13, 20, 80, 40, image, white);
	Line(20, 13, 40, 80, image, red);
	Line(80, 40, 13, 20, image, red);

	image.FlipVertical();
	image.WriteTGAImage("resultsTGA/out.tga");
	
	return 0;
}