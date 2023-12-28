#pragma once

#include <vector>

#include "geometry.h"
#include "tgaimage.h"

class Model
{
public:
	Model(const char* filename, const char* textureFile);
	~Model();
	
	int nVerts();
	int nFaces();
	int nUVs();
	int nNorms();
	
	std::vector<int> GetFace(int idx);
	Vec3f GetVert(int i);
	Vec3f GetVert(int iFace, int nthVertex);
	Vec3f GetUV(int i);
	Vec3f GetUV(int iFace, int nthVertex);
	Vec3f GetNormal(int iFace, int nthVertex, bool normalize = true);

	TGAColor SampleDiffuseMap(Vec2f uvf);

	const TGAImage& GetDiffuseTexture() const { return m_DiffuseMap; }
private:
	TGAImage m_DiffuseMap;

	std::vector<std::vector<Vec3i>> m_Faces; // Vec3i --> vertex/uv/normal
	std::vector<Vec3f> m_Verts;
	std::vector<Vec3f> m_UVs;
	std::vector<Vec3f> m_Norms;
};