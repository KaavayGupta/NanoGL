#include <iostream>
#include <vector>
#include <limits>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "nanogl.h"

Model* model = NULL;
TGAImage* AOImage = NULL;
float* shadowbuffer = NULL;

constexpr int width = 800;
constexpr int height = 800;

const Vec3f lightDir(1, 1, 0);
const Vec3f eye(1, 1, 4);
const Vec3f center(0, 0, 0);
const Vec3f up(0, 1, 0);

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

struct Shader : public IShader
{
	Mat<2, 3, float> varyingUV;
	Mat<3, 3, float> varyingTri;
	Mat<4, 4, float> uniformM;	// Projection * ModelView
	Mat<4, 4, float> uniformMIT; // (Projection * ModelView).InvertTranspose()
	Mat<4, 4, float> uniformMshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates

	Shader(const Mat4x4& M, const Mat4x4& MIT, const Mat4x4& Mshadow) 
		: uniformM(M), uniformMIT(MIT), uniformMshadow(Mshadow) {}

	virtual Vec4f Vertex(int iface, int nthvert)
	{
		varyingUV.SetCol(nthvert, model->GetUV(iface, nthvert));
		Vec4f glVertex = Viewport * Projection * ModelView * Embed<4>(model->GetVert(iface, nthvert));
		varyingTri.SetCol(nthvert, Proj<3>(glVertex / glVertex[3]));
		return glVertex;
	}

	virtual bool Fragment(Vec3f bar, TGAColor& color)
	{
		Vec4f sbP = uniformMshadow * Embed<4>(varyingTri * bar); // corresponding point in the shadow buffer
		sbP = sbP / sbP[3];
		int idx = int(sbP[0]) + int(sbP[1]) * width; // index in the shadowbuffer array
		float shadow = .3 + .7 * (shadowbuffer[idx] < sbP[2] + 43.34);

		Vec2f uv = varyingUV * bar;                 // interpolate uv for the current pixel
		Vec3f n = Proj<3>(uniformMIT * Embed<4>(model->SampleNormalMap(uv))).Normalize(); // normal
		Vec3f l = Proj<3>(uniformM * Embed<4>(lightDir)).Normalize(); // light vector
		Vec3f r = (n * (n * l * 2.f) - l).Normalize();   // reflected light
		float spec = pow(std::max(r.z, 0.0f), model->SampleSpecularMap(uv));
		float diff = std::max(0.f, n * l);
		TGAColor c = model->SampleDiffuseMap(uv);
		TGAColor glowColor = model->SampleGlowMap(uv);
		TGAColor ao = AOImage->GetPixel(uv[0] * AOImage->GetWidth(), uv[1] * AOImage->GetHeight()) * (1/255.0f);
		for (int i = 0; i < 3; i++) color.Raw[i] = std::min<float>((ao.Raw[i] + c.Raw[i] * shadow * (1.2f * diff + 6.0f * spec)) + glowColor.Raw[i] * 30.0f, 255);
		return false;
	}
};

struct DepthShader : public IShader
{
	Mat<3, 3, float> varyingTri;

	DepthShader() : varyingTri() {}

	virtual Vec4f Vertex(int iface, int nthvert)
	{
		Vec4f glVertex = Embed<4>(model->GetVert(iface, nthvert));
		glVertex = Viewport * Projection * ModelView * glVertex;
		varyingTri.SetCol(nthvert, Proj<3>(glVertex / glVertex[3]));
		return glVertex;
	}

	virtual bool Fragment(Vec3f bar, TGAColor& color)
	{
		Vec3f p = varyingTri * bar;
		color = TGAColor(255, 255, 255) * (p.z / depth);
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

	AOImage = new TGAImage(width, height, 3);
	model = new Model(modelFileName);

	{
		std::clog << "Calculating Ambient Occlusion..." << std::endl;
		LookAt(eye, center, up);
		CreateViewportMatrix(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		CreateProjectionMatrix(-1.0f / (eye - center).Magnitude());

		ZShader zshader;
		Vec4f screenCoords[3];
		for (int i = 0; i < model->nFaces(); i++) {
			for (int j = 0; j < 3; j++) {
				screenCoords[j] = zshader.Vertex(i, j);
			}
			Triangle(screenCoords, zshader, *AOImage, zbuffer);
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
				AOImage->SetPixel(x, y, TGAColor(total * 255, total * 255, total * 255));
			}
		}

		AOImage->FlipVertical();
		AOImage->WriteTGAImage("resultsTGA/ao.tga");

		std::clog << "DONE" << std::endl;
	}

	{
		std::clog << "Calculate Depth Map..." << std::endl;

		TGAImage depth(width, height, 3);
		LookAt(lightDir, center, up);
		CreateViewportMatrix(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		CreateProjectionMatrix(0);

		DepthShader depthShader;
		Vec4f screenCoords[3];
		for (int i = 0; i < model->nFaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				screenCoords[j] = depthShader.Vertex(i, j);
			}
			Triangle(screenCoords, depthShader, depth, shadowbuffer);
		}

		depth.FlipVertical();
		depth.WriteTGAImage("resultsTGA/depth.tga");

		std::clog << "DONE" << std::endl;
	}

	Mat4x4 M = Viewport * Projection * ModelView;

	{
		std::clog << "Rendering Final Image..." << std::endl;

		TGAImage frame(width, height, 3);
		LookAt(eye, center, up);
		CreateViewportMatrix(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
		CreateProjectionMatrix(-1.0f / (eye - center).Magnitude());

		Shader shader(ModelView, (Projection * ModelView).InvertTranspose(), M * (Viewport * Projection * ModelView).Invert());
		Vec4f screenCoords[3];
		for (int i = 0; i < model->nFaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				screenCoords[j] = shader.Vertex(i, j);
			}
			Triangle(screenCoords, shader, frame, zbuffer);
		}

		frame.FlipVertical();
		frame.WriteTGAImage("resultsTGA/framebuffer.tga");

		std::clog << "DONE" << std::endl;
	}

	delete model;
	delete[] zbuffer;
	delete[] shadowbuffer;
	
	return 0;
}