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
	Mat<4, 4, float> uniformM;	// Projection * ModelView
	Mat<4, 4, float> uniformMIT; // (Projection * ModelView).InvertTranspose()
	Mat<4, 4, float> uniformMshadow; // transform framebuffer screen coordinates to shadowbuffer screen coordinates
	Vec3f uniformLight;
	const Model& uniformModel;
	const float* const uniformShadowBuffer;
	const TGAImage* const uniformAOImage;

	Shader(const Mat4x4& M, const Mat4x4& MIT, const Mat4x4& Mshadow, const Model& model, const Vec3f& light, const float* const shadowBuffer, const TGAImage* const AOImage)
		: uniformM(M), uniformMIT(MIT), uniformMshadow(Mshadow), uniformModel(model), uniformLight(light), uniformShadowBuffer(shadowBuffer), uniformAOImage(AOImage)
	{
	}

	virtual Vec4f Vertex(int iface, int nthvert)
	{
		varyingUV.SetCol(nthvert, uniformModel.GetUV(iface, nthvert));
		Vec4f glVertex = Viewport * Projection * ModelView * Embed<4>(uniformModel.GetVert(iface, nthvert));
		varyingTri.SetCol(nthvert, Proj<3>(glVertex / glVertex[3]));
		return glVertex;
	}

	virtual bool Fragment(Vec3f bar, TGAColor& color)
	{
		Vec4f sbP = uniformMshadow * Embed<4>(varyingTri * bar); // corresponding point in the shadow buffer
		sbP = sbP / sbP[3];
		int idx = int(sbP[0]) + int(sbP[1]) * uniformAOImage->GetWidth(); // index in the shadowbuffer array
		float shadow = .3 + .7 * (uniformShadowBuffer[idx] < sbP[2] + 43.34);

		Vec2f uv = varyingUV * bar;                 // interpolate uv for the current pixel
		Vec3f n = Proj<3>(uniformMIT * Embed<4>(uniformModel.SampleNormalMap(uv))).Normalize(); // normal
		Vec3f l = Proj<3>(uniformM * Embed<4>(uniformLight)).Normalize(); // light vector
		Vec3f r = (n * (n * l * 2.f) - l).Normalize();   // reflected light
		float spec = pow(std::max(r.z, 0.0f), uniformModel.SampleSpecularMap(uv));
		float diff = std::max(0.f, n * l);
		TGAColor c = uniformModel.SampleDiffuseMap(uv);
		TGAColor glowColor = uniformModel.SampleGlowMap(uv);
		TGAColor ao = uniformAOImage->GetPixel(uv[0] * uniformAOImage->GetWidth(), uv[1] * uniformAOImage->GetHeight()) * (1 / 255.0f);
		for (int i = 0; i < 3; i++) color.Raw[i] = std::min<float>((ao.Raw[i] + c.Raw[i] * shadow * (1.2f * diff + 6.0f * spec)) + glowColor.Raw[i] * 30.0f, 255);
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
