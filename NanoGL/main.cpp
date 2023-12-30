#include <iostream>
#include <vector>
#include <limits>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "nanogl.h"

Model* model = NULL;
float* shadowbuffer = NULL;

const int width = 800;
const int height = 800;

Vec3f lightDir(1, 1, 0);
Vec3f eye(1, 1, 4);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

struct ZShader : public IShader
{
	Mat<4, 3, float> varyingTri;

	virtual Vec4f Vertex(int iface, int nthvert)
	{
		Vec4f glVertex = Viewport * Projection * ModelView * Embed<4>(model->GetVert(iface, nthvert));
		varyingTri.SetCol(nthvert, glVertex);
		return glVertex;
	}

	virtual bool Fragment(Vec3f bar, TGAColor& color)
	{
		color = TGAColor(0, 0, 0);
		return false;
	}
};

float MaxElevationAngle(float* zbuffer, Vec2f p, Vec2f dir) 
{
	float maxangle = 0;
	for (float t = 0.; t < 1000.; t += 1.) 
	{
		Vec2f cur = p + dir * t;
		if (cur.x >= width || cur.y >= height || cur.x < 0 || cur.y < 0) return maxangle;

		float distance = (p - cur).Magnitude();
		if (distance < 1.f) continue;
		float elevation = zbuffer[int(cur.x) + int(cur.y) * width] - zbuffer[int(p.x) + int(p.y) * width];
		maxangle = std::max(maxangle, atanf(elevation / distance));
	}
	return maxangle;
}

int main(int argc, char** argv)
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << "[obj/model.obj]" << std::endl;
	}

	const char* modelFileName = argc == 2 ? argv[1] : "obj/diablo3_pose/diablo3_pose.obj";
	std::clog << "Rendering default " << modelFileName << std::endl;

	float* zbuffer = new float[width * height];
	shadowbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max());

	model = new Model(modelFileName);

	{
		TGAImage AOImage(width, height, 3);
		LookAt(eye, center, up);
		CreateViewportMatrix(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		CreateProjectionMatrix(-1.0f / (eye - center).Magnitude());

		ZShader zshader;
		Vec4f screenCoords[3];
		for (int i = 0; i < model->nFaces(); i++) {
			for (int j = 0; j < 3; j++) {
				screenCoords[j] = zshader.Vertex(i, j);
			}
			Triangle(screenCoords, zshader, AOImage, zbuffer);
		}

		for (int x = 0; x < width; x++) {
			for (int y = 0; y < height; y++) {
				if (zbuffer[x + y * width] < -1e5) continue;
				float total = 0;
				for (float a = 0; a < M_PI * 2 - 1e-4; a += M_PI / 4) {
					total += M_PI / 2 - MaxElevationAngle(zbuffer, Vec2f(x, y), Vec2f(cos(a), sin(a)));
				}
				total /= (M_PI / 2) * 8;
				total = pow(total, 100.f);
				AOImage.SetPixel(x, y, TGAColor(total * 255, total * 255, total * 255));
			}
		}

		AOImage.FlipVertical();
		AOImage.WriteTGAImage("resultsTGA/ao.tga");
	}

	delete model;
	delete[] zbuffer;
	delete[] shadowbuffer;
	
	return 0;
}