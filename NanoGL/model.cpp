#include "model.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

Model::Model(const char* filename)
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
			for (int i = 0; i < 3; i++) { iss >> v.Raw[i]; }
			m_Verts.push_back(v);
		}
		else if (prefix == "f")
		{
			std::vector<int> f;
			int itrash, idx;
			while (iss >> idx >> trash >> itrash >> trash >> itrash)
			{
				f.push_back(idx - 1);	// obj indexes start from 1
			}
			m_Faces.push_back(f);
		}
	}

	in.close();
	std::clog << "#" << filename << " v#" << m_Verts.size() << " f# " << m_Faces.size() << std::endl;
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

Vec3f Model::GetVert(int i)
{
	return m_Verts[i];
}

std::vector<int> Model::GetFace(int idx)
{
	return m_Faces[idx];
}
