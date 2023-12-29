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

struct Shader : public IShader
{
	Mat<2, 3, float> varyingUV;
	Mat<3, 3, float> varyingTri;
	Mat<4, 4, float> uniformM;	// Projection * ModelView
	Mat<4, 4, float> uniformMIT; // (Projection * ModelView).InvertTranspose()
	Mat<4, 4, float> uniformMshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates

	Shader(Mat4x4 M, Mat4x4 MIT, Mat4x4 Mshadow) : uniformM(M), uniformMIT(MIT), uniformMshadow(Mshadow) {}

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
		for (int i = 0; i < 3; i++) color.Raw[i] = std::min<float>(20 + c.Raw[i] * shadow * (1.2 * diff + .6 * spec), 255);
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

int main(int argc, char** argv)
{
	if (argc > 2)
	{
		std::cerr << "Usage: " << argv[0] << "[obj/model.obj]" << std::endl;
	}

	const char* modelFileName = argc == 2 ? argv[1] : "obj/african_head.obj";
	std::clog << "Rendering default " << modelFileName << std::endl;

	float* zbuffer = new float[width * height];
	shadowbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max());

	model = new Model(modelFileName);
	lightDir.Normalize();
	
	{
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
	}

	Mat4x4 M = Viewport * Projection * ModelView;

	{
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
		frame.WriteTGAImage("resultsTGA/shadows.tga");
	}

	delete model;
	delete[] zbuffer;
	delete[] shadowbuffer;
	
	return 0;
}