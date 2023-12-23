#include "tgaimage.h"
#include "model.h"

#include <iostream>
#include <algorithm>

const TGAColor white(255, 255, 255, 255);
const TGAColor red(255, 0, 0, 255);
const TGAColor green(0, 255, 0, 255);
const TGAColor blue(00, 0, 255, 255);

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

void Line(Vec2i v1, Vec2i v2, TGAImage& image, const TGAColor& color)
{
	Line(v1.x, v1.y, v2.x, v2.y, image, color);
}

void Triangle(Vec2i t0, Vec2i t1, Vec2i t2, TGAImage& image, const TGAColor& color)
{
	Vec2i t[3] = { t0, t1, t2 };
	std::sort(std::begin(t), std::end(t), [](const Vec2i& a, const Vec2i& b) {return a.y < b.y; });

	int totalHeight = t[2].y - t[0].y;
	for (int y = 0; y <= totalHeight; y++)
	{
		bool isLowerHalf = y >= t[1].y - t[0].y;

		Vec2i A = t[0] + (t[2] - t[0]) * (float)y / totalHeight; // Longest line
		Vec2i dh = isLowerHalf ? t[2] - t[1] : t[1] - t[0];
		Vec2i B = (isLowerHalf ? t[1] : t[0]) + dh * (float) (isLowerHalf ? y - (t[1].y - t[0].y) : y) / dh.y;
		
		// Line(A, B, image, color);
		if (A.x > B.x) std::swap(A, B);
		for (int x = A.x; x <= B.x; x++)
		{
			image.SetPixel(x, t[0].y + y, color);
		}
	}

	//Line(t[0], t[1], image, green);
	//Line(t[1], t[2], image, green);
	//Line(t[2], t[0], image, red);
}

void DrawModelWire()
{
	const int width = 800;
	const int height = 800;

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
}

int main()
{
	TGAImage image(200, 200, 3);

	Vec2i t0[3] = { Vec2i(10, 70),   Vec2i(50, 160),  Vec2i(70, 80) };
	Vec2i t1[3] = { Vec2i(180, 50),  Vec2i(150, 1),   Vec2i(70, 180) };
	Vec2i t2[3] = { Vec2i(180, 150), Vec2i(120, 160), Vec2i(130, 180) };

	Triangle(t0[0], t0[1], t0[2], image, red);
	Triangle(t1[0], t1[1], t1[2], image, white);
	Triangle(t2[0], t2[1], t2[2], image, green);
	
	image.FlipVertical();
	image.WriteTGAImage("resultsTGA/triangles.tga");

	return 0;
}