#include "tgaimage.h"
#include "model.h"

#include <iostream>
#include <algorithm>
#include <limits>
#include <cmath>
#include <cstdlib>

const int width = 800;
const int height = 800;

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

Vec3f Barycentric(const Vec3f& A, const Vec3f& B, const Vec3f& C, const Vec3f& P)
{
	Vec3f s[2];
	for (int i = 2; i--;) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}

	Vec3f u = s[0] ^ s[1];
	if (std::abs(u[2]) > 1e-2)
		return Vec3f(1.f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void Triangle(Vec3f* pts, float *zbuffer, TGAImage& image, const TGAColor& color)
{
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.GetWidth() - 1, image.GetHeight() - 1);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bboxmin[j] = std::max(0.0f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}

	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bcScreen = Barycentric(pts[0], pts[1], pts[2], P);
			if (bcScreen.x < 0 || bcScreen.y < 0 || bcScreen.z < 0) continue;
			P.z = 0;
			for (int i = 0; i < 3; i++) P.z += pts[i][2] * bcScreen[i];
			if (zbuffer[int(P.x + P.y * width)] < P.z)
			{
				zbuffer[int(P.x + P.y * width)] = P.z;
				image.SetPixel(P.x, P.y, color);
			}
		}
	}
}

void Triangle(Vec3f* pts, float* zbuffer, TGAImage& image, Vec2f* texCoords, const TGAImage& texture, float intensity = 1.0f)
{
	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.GetWidth() - 1, image.GetHeight() - 1);

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bboxmin[j] = std::max(0.0f, std::min(bboxmin[j], pts[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
		}
	}

	Vec3f P;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bcScreen = Barycentric(pts[0], pts[1], pts[2], P);
			if (bcScreen.x < 0 || bcScreen.y < 0 || bcScreen.z < 0) continue;
			P.z = 0;
			for (int i = 0; i < 3; i++) P.z += pts[i][2] * bcScreen[i];
			if (zbuffer[int(P.x + P.y * width)] < P.z)
			{
				zbuffer[int(P.x + P.y * width)] = P.z;
				// Color calculations
				float u = bcScreen[0] * texCoords[0][0] + bcScreen[1] * texCoords[1][0] + bcScreen[2] * texCoords[2][0];
				float v = bcScreen[0] * texCoords[0][1] + bcScreen[1] * texCoords[1][1] + bcScreen[2] * texCoords[2][1];
				TGAColor color = texture.GetPixel(int(u * texture.GetWidth()), int(v * texture.GetHeight()));
				color.R *= intensity; color.G *= intensity; color.B *= intensity;
				image.SetPixel(P.x, P.y, color);
			}
		}
	}
}

Vec3f World2Screen(const Vec3f& v)
{
	return Vec3f(int((v.x + 1.0f) * width * 0.5f + 0.5f), int((v.y + 1.0f) * height * 0.5f + 0.5f), v.z);
}

void DrawModelWire()
{
	const int width = 800;
	const int height = 800;

	Model model("obj/african_head.obj", "obj/african_head_diffuse.tga");
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
	Model model("obj/african_head.obj", "obj/african_head_diffuse.tga");
	TGAImage image(width, height, 3);

	float* zbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	Vec3f lightDir(0, 0, -1);

	for (int i = 0; i < model.nFaces(); i++) 
	{
		std::vector<int> face = model.GetFace(i);
		Vec3f pts[3];
		Vec3f worldCoords[3];
		for (int j = 0; j < 3; j++) 
		{
			worldCoords[j] = model.GetVert(face[j]);
			pts[j] = World2Screen(model.GetVert(face[j]));
		}
		Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
		n.Normalize();
		float intensity = n * lightDir;
		if (intensity > 0)
			Triangle(pts, zbuffer, image, TGAColor(intensity * 255, intensity * 255, intensity * 255, 255));
	}

	image.FlipVertical();
	image.WriteTGAImage("resultsTGA/simple_shading.tga");
}

void DrawModelDiffuse()
{
	Model model("obj/african_head.obj", "obj/african_head_diffuse.tga");
	TGAImage image(width, height, 3);

	float* zbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	Vec3f lightDir(0, 0, -1);

	for (int i = 0; i < model.nFaces(); i++)
	{
		std::vector<int> face = model.GetFace(i);
		std::vector<int> texCoordIdxs = model.GetTexCoordIdx(i);
		Vec3f pts[3];
		Vec3f worldCoords[3];
		Vec2f texCoords[3];
		for (int j = 0; j < 3; j++)
		{
			worldCoords[j] = model.GetVert(face[j]);
			texCoords[j] = model.GetTexCoord(texCoordIdxs[j]);
			pts[j] = World2Screen(model.GetVert(face[j]));
		}

		Vec3f n = (worldCoords[2] - worldCoords[0]) ^ (worldCoords[1] - worldCoords[0]);
		n.Normalize();
		float intensity = n * lightDir;
		if (intensity > 0)
		{
			Triangle(pts, zbuffer, image, texCoords, model.GetDiffuseTexture(), intensity);
		}
	}

	image.FlipVertical();
	image.WriteTGAImage("resultsTGA/model_diffuse.tga");
}

int main()
{
	DrawModelDiffuse();

	return 0;
}