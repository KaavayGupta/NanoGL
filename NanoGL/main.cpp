#include "tgaimage.h"
#include "model.h"

#include <iostream>

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);
const int width = 800;
const int height = 800;

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
	Model model("obj/african_head.obj");
	TGAImage image(width, height, 3);

	for (int i = 0; i < model.nFaces(); i++)
	{
		std::vector<int> face = model.GetFace(i);
		for (int j = 0; j < 3; j++)
		{
			Vec3f v0 = model.GetVert(face[j]);
			Vec3f v1 = model.GetVert(face[(j + 1) % 3]);
			int x0 = (v0.x + 1.0f) * width / 2.0f;
			int y0 = (v0.y + 1.0f) * height / 2.0f;
			int x1 = (v1.x + 1.0f) * width / 2.0f;
			int y1 = (v1.y + 1.0f) * height / 2.0f;
			Line(x0, y0, x1, y1, image, white);
		}
	}

	image.FlipVertical();
	image.WriteTGAImage("resultsTGA/model_wire.tga");

	return 0;
}