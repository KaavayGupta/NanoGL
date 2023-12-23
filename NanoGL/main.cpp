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

Vec3f Barycentric(Vec2i* pts, Vec2i P) 
{
	Vec3f u = Vec3f(pts[2][0] - pts[0][0], pts[1][0] - pts[0][0], pts[0][0] - P[0]) ^ Vec3f(pts[2][1] - pts[0][1], pts[1][1] - pts[0][1], pts[0][1] - P[1]);
	/* `pts` and `P` has integer value as coordinates
	   so `abs(u[2])` < 1 means `u[2]` is 0, that means
	   triangle is degenerate, in this case return something with negative coordinates */
	if (std::abs(u.z) < 1) return Vec3f(-1, 1, 1);
	return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
}

void Triangle(Vec2i* pts, TGAImage& image, const TGAColor& color)
{
	Vec2i bboxmin(image.GetWidth() - 1, image.GetHeight() - 1);
	Vec2i bboxmax(0, 0);
	Vec2i clamp(image.GetWidth() - 1, image.GetHeight() - 1);

	for (int i = 0; i < 3; i++)
	{
		bboxmin.x = std::max(0, std::min(bboxmin.x, pts[i].x));
		bboxmin.y = std::max(0, std::min(bboxmin.y, pts[i].y));

		bboxmax.x = std::min(clamp.x, std::max(bboxmax.x, pts[i].x));
		bboxmax.y = std::min(clamp.y, std::max(bboxmax.y, pts[i].y));
	}

	Vec2i P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bcScreen = Barycentric(pts, P);
			if (bcScreen.x < 0 || bcScreen.y < 0 || bcScreen.z < 0) continue;
			image.SetPixel(P.x, P.y, color);
		}
	}
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

void DrawSimpleShading()
{
	const int width = 800;
	const int height = 800;

	Model model("obj/african_head.obj");
	TGAImage image(width, height, 3);

	Vec3f lightDir(0, 0, -1);

	for (int i = 0; i < model.nFaces(); i++)
	{
		std::vector<int> face = model.GetFace(i);
		Vec2i screenCoords[3];
		Vec3f worldCoords[3];
		for (int j = 0; j < 3; j++)
		{
			worldCoords[j] = model.GetVert(face[j]);
			screenCoords[j] = Vec2i((worldCoords[j].x + 1.0f) * width / 2.0f, (worldCoords[j].y + 1.0f) * height / 2.0f);
		}

		Vec3f normal = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
		normal.Normalize();
		float intensity = normal * lightDir;
		if (intensity > 0)
			Triangle(screenCoords, image, TGAColor(intensity * 255, intensity * 255, intensity * 255));
	}

	image.FlipVertical();
	image.WriteTGAImage("resultsTGA/simple_shading.tga");
}

int main()
{
	DrawSimpleShading();

	return 0;
}