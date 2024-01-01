#include "model.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

Model::Model(const char* filename, const char* diffuseMapFile, const char* normalMapFile, const char* specularMapFile)
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

	in.close();
	std::clog << "#" << filename << " v#" << m_Verts.size() << " f# " << m_Faces.size() << " vt#" << m_UVs.size() << " vn#" << m_Norms.size() << std::endl;

	// Load textures
	if (diffuseMapFile)
		LoadTexture(diffuseMapFile, m_DiffuseMap);
	else
		LoadTexture(filename, m_DiffuseMap, "_diffuse.tga");

	if (normalMapFile)
		LoadTexture(normalMapFile, m_NormalMap);
	else
		LoadTexture(filename, m_NormalMap, "_nm.tga");

	if (diffuseMapFile)
		LoadTexture(diffuseMapFile, m_SpecularMap);
	else
		LoadTexture(filename, m_SpecularMap, "_spec.tga");

	LoadTexture(filename, m_GlowMap, "_glow.tga");
	
}

Model::~Model()
{
}

int Model::nVerts() const
{
	return (int)m_Verts.size();
}

int Model::nFaces() const
{
	return (int)m_Faces.size();
}

int Model::nUVs() const
{
	return (int)m_UVs.size();
}
 
int Model::nNorms() const
{
	return (int)m_Norms.size();
}

Vec3f Model::GetVert(int i) const
{
	return m_Verts[i];
}

Vec3f Model::GetVert(int iFace, int nthVertex) const
{
	return m_Verts[m_Faces[iFace][nthVertex][0]];
}

std::vector<int> Model::GetFace(int idx) const
{
	std::vector<int> face;
	for (int i = 0; i < m_Faces[idx].size(); i++)
	{
		face.push_back(m_Faces[idx][i][0]);
	}
	return face;
}

Vec2f Model::GetUV(int i) const
{
	return m_UVs[i];
}

Vec2f Model::GetUV(int iFace, int nthVertex) const
{
	return m_UVs[m_Faces[iFace][nthVertex][1]];
}

Vec3f Model::GetNormal(int iFace, int nthVertex, bool normalize) const
{
	Vec3f n = m_Norms[m_Faces[iFace][nthVertex][2]];
	return normalize ? n.Normalize() : n;
}

TGAColor Model::SampleDiffuseMap(Vec2f uvf) const
{
	Vec2i uv(uvf[0] * m_DiffuseMap.GetWidth(), uvf[1] * m_DiffuseMap.GetHeight());
	return m_DiffuseMap.GetPixel(uv[0], uv[1]);
}

Vec3f Model::SampleNormalMap(Vec2f uvf) const
{
	Vec2i uv(uvf[0] * m_NormalMap.GetWidth(), uvf[1] * m_NormalMap.GetHeight());
	TGAColor c = m_NormalMap.GetPixel(uv[0], uv[1]);
	Vec3f res;
	for (int i = 0; i < 3; i++)
		res[2 - i] = (float)c.Raw[i] / 255.0f * 2.0f - 1.0f;
	return res;
}

float Model::SampleSpecularMap(Vec2f uvf) const
{
	Vec2i uv(uvf[0] * m_NormalMap.GetWidth(), uvf[1] * m_NormalMap.GetHeight());
	return (float)m_NormalMap.GetPixel(uv[0], uv[1]).Raw[0];
}

TGAColor Model::SampleGlowMap(Vec2f uvf) const
{
	Vec2i uv(uvf[0] * m_GlowMap.GetWidth(), uvf[1] * m_GlowMap.GetHeight());
	return m_GlowMap.GetPixel(uv[0], uv[1]);
}

void Model::LoadTexture(std::string filename, TGAImage& img, const char* suffix)
{
	if (!suffix)
	{
		std::clog << "Texture file " << filename << " loading " << (img.ReadTGAImage(filename.c_str()) ? "OK" : "FAILED") << std::endl;
		img.FlipVertical();
		return;
	}

	std::string texfile(filename);
	size_t dot = texfile.find_last_of(".");
	if (dot != std::string::npos)
	{
		texfile = texfile.substr(0, dot) + std::string(suffix);
		std::clog << "Texture file " << texfile << " loading " << (img.ReadTGAImage(texfile.c_str()) ? "OK" : "FAILED") << std::endl;
	}
	else
	{
		std::cerr << "Invalid suffix/filename name " << texfile << std::endl;
	}
}

