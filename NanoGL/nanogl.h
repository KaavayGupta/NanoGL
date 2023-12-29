#pragma once

#include "tgaimage.h"
#include "geometry.h"

extern Mat4x4 ModelView;
extern Mat4x4 Viewport;
extern Mat4x4 Projection;

void CreateViewportMatrix(int x, int y, int w, int h);
void CreateProjectionMatrix(float coeff = 0.0f);	// coeff = -1/c
void LookAt(Vec3f eye, Vec3f center, Vec3f up);

struct IShader
{
	virtual ~IShader();
	virtual Vec4f Vertex(int iface, int nthvert) = 0;
	virtual bool Fragment(Vec3f bar, TGAColor& color) = 0;
};

void Triangle(Mat<4, 3, float>& clipc, IShader& shader, TGAImage& image, float* zbuffer);