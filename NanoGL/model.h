#pragma once

#include <vector>
#include "geometry.h"

class Model
{
public:
	Model(const char* filename);
	~Model();
	
	int nVerts();
	int nFaces();
	
	Vec3f GetVert(int i);
	std::vector<int> GetFace(int idx);
private:
	std::vector<Vec3f> m_Verts;
	std::vector<std::vector<int>> m_Faces;
};