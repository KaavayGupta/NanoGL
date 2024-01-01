#pragma once

#include "nanogl.h"
#include "model.h"

struct ZShader : public IShader
{
	Mat<4, 3, float> varyingTri;
	const Model& uniformModel;

	ZShader(const Model& model) : uniformModel(model) {}

	virtual Vec4f Vertex(int iface, int nthvert)
	{
		Vec4f glVertex = Viewport * Projection * ModelView * Embed<4>(uniformModel.GetVert(iface, nthvert));
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
	Mat<3, 3, float> varyingNorm;
	Mat<4, 4, float> uniformM;	// Projection * ModelView
	Mat<4, 4, float> uniformMIT; // (Projection * ModelView).InvertTranspose()
	Mat<4, 4, float> uniformMshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates
	Vec3f uniformLight;
	const Model& uniformModel;
	const float* const uniformShadowBuffer;
	const TGAImage* const uniformAOImage;

	Shader(const Mat4x4& M, const Mat4x4& MIT, const Mat4x4& Mshadow, const Model& model, const Vec3f& light, const float* const shadowBuffer, const TGAImage* const AOImage)
		: uniformM(M), uniformMIT(MIT), uniformMshadow(Mshadow), uniformModel(model), uniformShadowBuffer(shadowBuffer), uniformAOImage(AOImage)
	{
		uniformLight = Proj<3>(uniformM * Embed<4>(light)).Normalize(); // light vector
	}

	virtual Vec4f Vertex(int iface, int nthvert)
	{
		varyingUV.SetCol(nthvert, uniformModel.GetUV(iface, nthvert));
		varyingNorm.SetCol(nthvert, Proj<3>((Viewport * Projection * ModelView).InvertTranspose() * Embed<4>(uniformModel.GetNormal(iface, nthvert), 0.0f)));
		Vec4f glVertex = Viewport * Projection * ModelView * Embed<4>(uniformModel.GetVert(iface, nthvert));
		varyingTri.SetCol(nthvert, Proj<3>(glVertex / glVertex[3]));
		return glVertex;
	}

	virtual bool Fragment(Vec3f bar, TGAColor& color)
	{
		Vec4f sbP = uniformMshadow * Embed<4>(varyingTri * bar); // corresponding point in the shadow buffer
		sbP = sbP / sbP[3];
		int idx = std::max(0,int(sbP[0])) + std::max(0,int(sbP[1])) * uniformAOImage->GetWidth(); // index in the shadowbuffer array
		float shadow = .3 + .7 * (uniformShadowBuffer[idx] < sbP[2] + 43.34);

		// Tangent Normal Calculations
		Vec3f bn = (varyingNorm * bar).Normalize();
		Mat<3, 3, float> A;
		A[0] = varyingTri.Col(1) - varyingTri.Col(0);
		A[1] = varyingTri.Col(2) - varyingTri.Col(0);
		A[2] = bn;
		Mat<3, 3, float> AI = A.Invert();

		Vec3f i = AI * Vec3f(varyingUV[0][1] - varyingUV[0][0], varyingUV[0][2] - varyingUV[0][0], 0);
		Vec3f j = AI * Vec3f(varyingUV[1][1] - varyingUV[1][0], varyingUV[1][2] - varyingUV[1][0], 0);

		Mat<3, 3, float> B;
		B.SetCol(0, i.Normalize());
		B.SetCol(1, j.Normalize());
		B.SetCol(2, bn);

		Vec2f uv = varyingUV * bar;                 // interpolate uv for the current pixel
		Vec3f n = (B * uniformModel.SampleNormalMap(uv)).Normalize(); // normal
		Vec3f r = (n * (n * uniformLight * 2.f) - uniformLight).Normalize();   // reflected light
		float spec = pow(std::max(r.z, 0.0f), uniformModel.SampleSpecularMap(uv));
		float diff = std::max(0.f, n * uniformLight);
		TGAColor c = uniformModel.SampleDiffuseMap(uv);
		TGAColor glowColor = uniformModel.SampleGlowMap(uv);
		TGAColor ao = uniformAOImage->GetPixel(uv[0] * uniformAOImage->GetWidth(), uv[1] * uniformAOImage->GetHeight()) * (1 / 255.0f);
		for (int i = 0; i < 3; i++) color.Raw[i] = std::min<float>((ao.Raw[i] + c.Raw[i] * shadow * (1.0f * diff + 1.1f * spec)) + glowColor.Raw[i] * 30.0f, 255);
		return false;
	}
};

struct DepthShader : public IShader
{
	Mat<3, 3, float> varyingTri;
	const Model& uniformModel;

	DepthShader(const Model& model) : uniformModel(model), varyingTri() {}

	virtual Vec4f Vertex(int iface, int nthvert)
	{
		Vec4f glVertex = Embed<4>(uniformModel.GetVert(iface, nthvert));
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

struct GouraudShader : public IShader 
{
	Vec3f varyingIntensity;
	const Vec3f& uniformLightDir;
	const Model& uniformModel;

	GouraudShader(const Model& model, const Vec3f& lightDir) : uniformModel(model), uniformLightDir(lightDir) {}

	virtual Vec4f Vertex(int iface, int nthvert)
	{
		Vec4f glVertex = Embed<4>(uniformModel.GetVert(iface, nthvert));
		glVertex = Viewport * Projection * ModelView * glVertex;
		varyingIntensity[nthvert] = std::max(0.f, uniformModel.GetNormal(iface, nthvert) * uniformLightDir);
		return glVertex;
	}

	virtual bool Fragment(Vec3f bar, TGAColor& color) 
	{
		float intensity = varyingIntensity * bar;
		color = TGAColor(255, 255, 255) * intensity;
		return false;
	}
};

struct ToonShader : public IShader 
{
	Vec3f varyingIntensity;
	const Vec3f& uniformLightDir;
	const Model& uniformModel;

	ToonShader(const Model& model, const Vec3f& lightDir) : uniformModel(model), uniformLightDir(lightDir) {}

	virtual Vec4f Vertex(int iface, int nthvert) 
	{
		Vec4f glVertex = Embed<4>(uniformModel.GetVert(iface, nthvert));
		glVertex = Viewport * Projection * ModelView * glVertex;
		varyingIntensity[nthvert] = std::max(0.f, uniformModel.GetNormal(iface, nthvert) * uniformLightDir);
		return glVertex;
	}

	virtual bool Fragment(Vec3f bar, TGAColor& color) 
	{
		float intensity = varyingIntensity * bar;
		if (intensity > .85) intensity = 1;
		else if (intensity > .60) intensity = .80;
		else if (intensity > .45) intensity = .60;
		else if (intensity > .30) intensity = .45;
		else if (intensity > .15) intensity = .30;
		color = TGAColor(155, 0, 100) * intensity;
		return false;
	}
};

