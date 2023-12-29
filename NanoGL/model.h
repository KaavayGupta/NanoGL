#pragma once

#include <vector>

#include "geometry.h"
#include "tgaimage.h"

class Model
{
public:
	Model(const char* filename, const char* diffuseMapFile = nullptr, const char* normalMapFile = nullptr, const char* specularMapFile = nullptr);
	~Model();
	
	int nVerts();
	int nFaces();
	int nUVs();
	int nNorms();
	
	std::vector<int> GetFace(int idx);
	Vec3f GetVert(int i);
	Vec3f GetVert(int iFace, int nthVertex);
	Vec2f GetUV(int i);
	Vec2f GetUV(int iFace, int nthVertex);
	Vec3f GetNormal(int iFace, int nthVertex, bool normalize = true);

	TGAColor SampleDiffuseMap(Vec2f uvf);
	Vec3f SampleNormalMap(Vec2f uvf);
	float SampleSpecularMap(Vec2f uvf);

	const TGAImage& GetDiffuseMap() const { return m_DiffuseMap; }
	const TGAImage& GetNormalMap() const { return m_NormalMap; }
	const TGAImage& GetSpecularMap() const { return m_SpecularMap; }
private:
	void LoadTexture(std::string filename, TGAImage& img, const char* suffix = nullptr);
private:
	TGAImage m_DiffuseMap;
	TGAImage m_NormalMap;
	TGAImage m_SpecularMap;

	std::vector<std::vector<Vec3i>> m_Faces; // Vec3i --> vertex/uv/normal
	std::vector<Vec3f> m_Verts;
	std::vector<Vec2f> m_UVs;
	std::vector<Vec3f> m_Norms;
};