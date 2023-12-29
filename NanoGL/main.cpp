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

struct Shader : public IShader
{
	Mat<2, 3, float> varyingUV;		// triangle uv coordinates
	Mat<3, 3, float> varyingNorm;	// normal per vertex
	Mat<4, 3, float> varyingTri;	// triangle coordinates
	Mat<3, 3, float> ndcTri;		// triangle in normalised device coordinates

	virtual Vec4f Vertex(int iface, int nthvert)
	{
		varyingUV.SetCol(nthvert, model->GetUV(iface, nthvert));
		varyingNorm.SetCol(nthvert, Proj<3>((Projection * ModelView).InvertTranspose() * Embed<4>(model->GetNormal(iface, nthvert), 0.0f)));
		Vec4f gl_Vertex = Projection * ModelView * Embed<4>(model->GetVert(iface, nthvert));
		varyingTri.SetCol(nthvert, gl_Vertex);
		ndcTri.SetCol(nthvert, Proj<3>(gl_Vertex / gl_Vertex[3]));
		return gl_Vertex;
	}

	virtual bool Fragment(Vec3f bar, TGAColor& color)
	{
		Vec3f bn = (varyingNorm * bar).Normalize();
		Vec2f uv = varyingUV * bar;

		Mat<3, 3, float> A;
		A[0] = ndcTri.Col(1) - ndcTri.Col(0);
		A[1] = ndcTri.Col(2) - ndcTri.Col(0);
		A[2] = bn;

		Mat<3, 3, float> AI = A.Invert();

		Vec3f i = AI * Vec3f(varyingUV[0][1] - varyingUV[0][0], varyingUV[0][2] - varyingUV[0][0], 0);
		Vec3f j = AI * Vec3f(varyingUV[1][1] - varyingUV[1][0], varyingUV[1][2] - varyingUV[1][0], 0);

		Mat<3, 3, float> B;
		B.SetCol(0, i.Normalize());
		B.SetCol(1, j.Normalize());
		B.SetCol(2, bn);

		Vec3f n = (B * model->SampleNormalMap(uv)).Normalize();
		Vec3f r = (n * (n * lightDir * 2.0f) - lightDir).Normalize();	// Reflected ray
		float spec = pow(std::max(r.z, 0.0f), model->SampleSpecularMap(uv));
		float diff = std::max(0.0f, n*lightDir);
		TGAColor c = model->SampleDiffuseMap(uv) * diff;
		color = c;
		for (int i = 0; i < 3; i++) color.Raw[i] = std::min<float>(8 + c.Raw[i] * (diff + 0.8 * spec), 255);	// Phongs Approx: Weighted sum of ambient, diffuse and specular

		return false;
	}
};

int main()
{
	float* zbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

	TGAImage frame(width, height, 3);
	LookAt(eye, center, up);
	CreateViewportMatrix(width / 8, height / 8, width * 3 / 4, height * 3 / 4);
	CreateProjectionMatrix(-1.0f / (eye - center).Magnitude());
	lightDir = Proj<3>((Projection * ModelView * Embed<4>(lightDir, 0.f))).Normalize();

	model = new Model("obj/african_head.obj");
	Shader shader;
	for (int i = 0; i < model->nFaces(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			shader.Vertex(i, j);
		}
		Triangle(shader.varyingTri, shader, frame, zbuffer);
	}
	delete model;

	frame.FlipVertical();
	frame.WriteTGAImage("resultsTGA/simple_shader.tga");

	delete[] zbuffer;
	return 0;
}