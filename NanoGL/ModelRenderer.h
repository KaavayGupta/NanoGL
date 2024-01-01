#pragma once

#include "nanogl.h"
#include "model.h"
#include "tgaimage.h"
#include "shaders.h"

class ModelRenderer
{
public:
	ModelRenderer(const char* filenamme, TGAImage* AOImage, TGAImage* depthImage, float* zbuffer, float* shadowbuffer);
	~ModelRenderer();
	
	void Render(TGAImage& frame, const Vec3f& eye, const Vec3f& center, const Vec3f& up, const Vec3f& lightDir);
private:
	Model* m_Model;
	TGAImage* m_AOImage, *m_DepthImage;
	int m_Width, m_Height;
	float* m_Zbuffer;
	float* m_ShadowBuffer;
};