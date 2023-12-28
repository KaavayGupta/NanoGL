#include "model.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

Model::Model(const char* filename, const char* textureFile)
{
	std::ifstream in;
	in.open(filename, std::ifstream::in);
	if (!in.is_open())
	{
		in.close();
		std::cerr << "Error opening file " << filename;
		return;
	}

	std::string line;
	while (std::getline(in, line))
	{
		std::istringstream iss(line);
		std::string prefix;
		char trash;
		iss >> prefix;

		if (prefix == "v")
		{
			Vec3f v;
			for (int i = 0; i < 3; i++) { iss >> v[i]; }
			m_Verts.push_back(v);
		}
		else if (prefix == "vt")
		{
			Vec2f vt;
			for (int i = 0; i < 2; i++) { iss >> vt[i]; }
			m_UVs.push_back(vt);
			iss >> trash;
		}
		else if (prefix == "vn")
		{
			Vec3f n;
			for (int i = 0; i < 3; i++) { iss >> n[i]; }
			m_Norms.push_back(n);
		}
		else if (prefix == "f")
		{
			std::vector<Vec3i> f;
			Vec3f temp;
			int idx, iuv, inorm;

			while (iss >> idx >> trash >> iuv >> trash >> inorm)
			{
				// obj indexes start from 1
				temp[0] = idx - 1;
				temp[1] = iuv - 1;
				temp[2] = inorm - 1;
				f.push_back(temp);
			}

			m_Faces.push_back(f);
		}
	}

	if (textureFile)
	{
		if (!m_DiffuseMap.ReadTGAImage(textureFile))
		{
			in.close();
			std::cerr << "Could not read texture " << textureFile;
		}
	}

	in.close();
	std::clog << "#" << filename << " v#" << m_Verts.size() << " f# " << m_Faces.size() << " vt#" << m_UVs.size() << " vn#" << m_Norms.size() << std::endl;
}

Model::~Model()
{
}

int Model::nVerts()
{
	return (int)m_Verts.size();
}

int Model::nFaces()
{
	return (int)m_Faces.size();
}

int Model::nUVs()
{
	return (int)m_UVs.size();
}

int Model::nNorms()
{
	return (int)m_Norms.size();
}

Vec3f Model::GetVert(int i)
{
	return m_Verts[i];
}

Vec3f Model::GetVert(int iFace, int nthVertex)
{
	return m_Verts[m_Faces[iFace][nthVertex][0]];
}

std::vector<int> Model::GetFace(int idx)
{
	std::vector<int> face;
	for (int i = 0; i < m_Faces[idx].size(); i++)
	{
		face.push_back(m_Faces[idx][i][0]);
	}
	return face;
}

Vec2f Model::GetUV(int i)
{
	return m_UVs[i];
}

Vec2f Model::GetUV(int iFace, int nthVertex)
{
	return m_UVs[m_Faces[iFace][nthVertex][1]];
}

Vec3f Model::GetNormal(int iFace, int nthVertex, bool normalize)
{
	Vec3f n = m_Norms[m_Faces[iFace][nthVertex][2]];
	return normalize ? n.Normalize() : n;
}

TGAColor Model::SampleDiffuseMap(Vec2f uvf)
{
	Vec2i uv(uvf[0] * m_DiffuseMap.GetWidth(), uvf[1] * m_DiffuseMap.GetHeight());
	return m_DiffuseMap.GetPixel(uv[0], uv[1]);
}

