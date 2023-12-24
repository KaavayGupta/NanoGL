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
	int nTexCoords();
	int nTexCoordIdx();
	
	Vec3f GetVert(int i);
	std::vector<int> GetFace(int idx);
	Vec2f GetTexCoord(int i);
	std::vector<int> GetTexCoordIdx(int faceIdx);

	const TGAImage& GetDiffuseTexture() const { return m_DiffuseTexture; }
private:
	TGAImage m_DiffuseTexture;

	std::vector<Vec3f> m_Verts;
	std::vector<std::vector<int>> m_Faces;
	std::vector<Vec2f> m_TexCoords;
	std::vector<std::vector<int>> m_TexCoordIdxs;
};