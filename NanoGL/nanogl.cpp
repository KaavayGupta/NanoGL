#include "nanogl.h"

#include <cmath>
#include <limits>

Mat4x4 ModelView;
Mat4x4 Viewport;
Mat4x4 Projection;

IShader::~IShader() {}

void CreateViewportMatrix(int x, int y, int w, int h)
{
	Viewport = Mat4x4::Identity();
	Viewport[0][3] = x + w / 2.0f;
	Viewport[1][3] = y + h / 2.0f;
	Viewport[2][3] = 255.0f / 2.0f;
	Viewport[0][0] = w / 2.0f;
	Viewport[1][1] = h / 2.0f;
	Viewport[2][2] = 255.0f / 2.0f;
}

void CreateProjectionMatrix(float coeff)
{
	Projection = Mat4x4::Identity();
	Projection[3][2] = coeff;
}

void LookAt(Vec3f eye, Vec3f center, Vec3f up)
{
	Vec3f z = (eye - center).Normalize();
	Vec3f x = Cross(up, z).Normalize();
	Vec3f y = Cross(z, x).Normalize();
	ModelView = Mat4x4::Identity();
	for (int i = 0; i < 3; i++)
	{
		ModelView[0][i] = x[i];
		ModelView[1][i] = y[i];
		ModelView[2][i] = z[i];
		ModelView[i][3] = -center[i];
	}
}

Vec3f Barycentric(const Vec2f& A, const Vec2f& B, const Vec2f& C, const Vec2f& P)
{
	Vec3f s[2];
	for (int i = 2; i--;) {
		s[i][0] = C[i] - A[i];
		s[i][1] = B[i] - A[i];
		s[i][2] = A[i] - P[i];
	}

	Vec3f u = Cross(s[0], s[1]);
	if (std::abs(u[2]) > 1e-2)
		return Vec3f(1.0f - (u.x + u.y) / u.z, u.y / u.z, u.x / u.z);
	return Vec3f(-1, 1, 1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}
void Triangle(Vec4f* pts, IShader& shader, TGAImage& image, float* zbuffer)
{
	Mat<4, 3, float> clipc;
	Mat<3, 2, float> pts2;
	for (int i = 0; i < 3; i++)
	{
		clipc.SetCol(i, pts[i]);
		pts2[i] = Proj<2>(pts[i] / pts[i][3]);
	}
	clipc = Viewport.Invert() * clipc;

	Vec2f bboxmin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
	Vec2f clamp(image.GetWidth() - 1, image.GetHeight() - 1);
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			bboxmin[j] = std::max(0.0f, std::min(bboxmin[j], pts2[i][j]));
			bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts2[i][j]));
		}
	}

	Vec2i P;
	TGAColor color;
	for (P.x = bboxmin.x; P.x <= bboxmax.x; P.x++)
	{
		for (P.y = bboxmin.y; P.y <= bboxmax.y; P.y++)
		{
			Vec3f bcScreen = Barycentric(pts2[0], pts2[1], pts2[2], P);
			Vec3f bcClip = Vec3f(bcScreen.x / pts[0][3], bcScreen.y / pts[1][3], bcScreen.z / pts[2][3]);
			bcClip = bcClip / (bcClip.x + bcClip.y + bcClip.z);
			float fragDepth = clipc[2] * bcClip;
			if (bcScreen.x < 0 || bcScreen.y < 0 || bcScreen.z < 0 || zbuffer[P.x + P.y * image.GetWidth()] > fragDepth) continue;
			bool discard = shader.Fragment(bcClip, color);
			if (!discard)
			{
				zbuffer[P.x + P.y * image.GetWidth()] = fragDepth;
				image.SetPixel(P.x, P.y, color);
			}
		}
	}
}