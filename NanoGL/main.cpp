#include <iostream>
#include <vector>

#include "tgaimage.h"
#include "model.h"
#include "geometry.h"
#include "nanogl.h"

Model* model = NULL;
const int width = 800;
const int height = 800;

Vec3f lightDir(1, 1, 1);
Vec3f eye(1, 1, 3);
Vec3f center(0, 0, 0);
Vec3f up(0, 1, 0);

struct GourardShader : public IShader
{
	Vec3f varyingIntensity; // written by vertex shader, read by fragment shader

	virtual Vec4f Vertex(int iface, int nthvert) 
	{
		varyingIntensity[nthvert] = std::max(0.f, model->GetNormal(iface, nthvert) * lightDir); // get diffuse lighting intensity
		Vec4f gl_Vertex = Embed<4>(model->GetVert(iface, nthvert)); // read the vertex from .obj file
		return Viewport * Projection * ModelView * gl_Vertex; // transform it to screen coordinates
	}

	virtual bool Fragment(Vec3f bar, TGAColor& color) 
	{
		float intensity = varyingIntensity * bar;   // interpolate intensity for the current pixel
		color = TGAColor(255, 255, 255);
		color.R *= intensity; color.G *= intensity; color.B *= intensity;
		return false;                              // no, we do not discard this pixel
	}
};

struct GourardShader2 : public IShader
{
	Vec3f varyingIntensity; // written by vertex shader, read by fragment shader

	virtual Vec4f Vertex(int iface, int nthvert) 
	{
		varyingIntensity[nthvert] = std::max(0.f, model->GetNormal(iface, nthvert) * lightDir); // get diffuse lighting intensity
		Vec4f gl_Vertex = Embed<4>(model->GetVert(iface, nthvert)); // read the vertex from .obj file
		return Viewport * Projection * ModelView * gl_Vertex; // transform it to screen coordinates
	}

	virtual bool Fragment(Vec3f bar, TGAColor& color) {
		float intensity = varyingIntensity * bar;   // interpolate intensity for the current pixel
		
		if (intensity > .85) intensity = 1;
		else if (intensity > .60) intensity = .80;
		else if (intensity > .45) intensity = .60;
		else if (intensity > .30) intensity = .45;
		else if (intensity > .15) intensity = .30;
		else intensity = 0;

		color = TGAColor(50, 0, 255);
		color.R *= intensity; color.G *= intensity; color.B *= intensity;
		return false;                              // no, we do not discard this pixel
	}
};

int main()
{
	model = new Model("obj/african_head.obj", "obj/african_head_diffuse.tga");

	LookAt(eye, center, up);
	CreateViewportMatrix(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	CreateProjectionMatrix(-1.0f / (eye - center).Magnitude());
	lightDir.Normalize();

	TGAImage image(width, height, 3);
	TGAImage zbuffer(width, height, 1);

	GourardShader2 shader;
	for (int i = 0; i < model->nFaces(); i++)
	{
		Vec4f screenCoords[3];
		for (int j = 0; j < 3; j++)
		{
			screenCoords[j] = shader.Vertex(i, j);
		}
		Triangle(screenCoords, shader, image, zbuffer);
	}

	image.FlipVertical();
	zbuffer.FlipVertical();
	image.WriteTGAImage("resultsTGA/gourard_shader2.tga");
	zbuffer.WriteTGAImage("resultsTGA/gourard_shader_zbuf.tga");

	delete model;
	return 0;
}