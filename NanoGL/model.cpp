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
			m_TexCoords.push_back(vt);
			iss >> trash;
		}
		else if (prefix == "f")
		{
			std::vector<int> f;
			std::vector<int> t;
			int itrash, itex, idx;
			while (iss >> idx >> trash >> itex >> trash >> itrash)
			{
				f.push_back(idx - 1);	// obj indexes start from 1
				t.push_back(itex - 1);
			}
			m_Faces.push_back(f);
			m_TexCoordIdxs.push_back(t);
		}
	}

	if (textureFile)
	{
		if (!m_DiffuseTexture.ReadTGAImage(textureFile))
		{
			in.close();
			std::cerr << "Could not read texture " << textureFile;
		}
	}

	in.close();
	std::clog << "#" << filename << " v#" << m_Verts.size() << " f# " << m_Faces.size() << " vt#" << m_TexCoords.size() << std::endl;
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

int Model::nTexCoords()
{
	return (int)m_TexCoords.size();
}

int Model::nTexCoordIdx()
{
	return (int)m_TexCoordIdxs.size();
}

Vec3f Model::GetVert(int i)
{
	return m_Verts[i];
}

std::vector<int> Model::GetFace(int idx)
{
	return m_Faces[idx];
}

Vec2f Model::GetTexCoord(int i)
{
	return m_TexCoords[i];
}

std::vector<int> Model::GetTexCoordIdx(int faceIdx)
{
	return m_TexCoordIdxs[faceIdx];
}
