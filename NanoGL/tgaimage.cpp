#include "tgaimage.h"

#include <iostream>
#include <fstream>
#include <cstring>

TGAImage::~TGAImage()
{
	delete[] m_Data;
}

TGAImage::TGAImage()
	: m_Width(0), m_Height(0), m_BytesPerPixel(0), m_Data(nullptr)
{
}

TGAImage::TGAImage(uint16_t width, uint16_t height, uint8_t bytesPerPixel)
	: m_Width(width), m_Height(height), m_BytesPerPixel(bytesPerPixel)
{
	unsigned long nbytes = width * height * bytesPerPixel;
	m_Data = new uint8_t[nbytes];
	memset(m_Data, 0, nbytes);
}

bool TGAImage::ReadTGAImage(const char* filename)
{
	// Open the file
	std::ifstream in;
	in.open(filename, std::ios::binary);
	if (!in.is_open())
	{
		std::cerr << "Error opening file " << filename << "\n";
		in.close();
		return false;
	}

	// Read the TGA Header
	TGAHeader header;
	in.read((char*)&header, sizeof(header));
	if (!in.good())
	{
		std::cerr << "Unable to read TGA Header\n";
		in.close();
		return false;
	}

	// Validate width/height/Bpp
	m_Width = header.Width;
	m_Height = header.Height;
	m_BytesPerPixel = header.BitsPerPixel >> 3;
	if (m_Width <= 0 || m_Height <= 0 || (m_BytesPerPixel != 1 && m_BytesPerPixel != 3 && m_BytesPerPixel != 4))
	{
		std::cerr << "Invalid Width or Height or BytesPerPixel\n";
		in.close();
		return false;
	}

	// Read the data
	unsigned long nbytes = m_Width * m_Height * m_BytesPerPixel;
	m_Data = new uint8_t[nbytes];

	if (header.ImageType == 2 || header.ImageType == 3) // True Color or Gray Scale
	{
		in.read((char*)m_Data, nbytes);
		if (!in.good())
		{
			std::cerr << "Unable to read TGA Data\n";
			in.close();
			return false;
		}
	}
	else
	{
		std::cerr << "Unsupported/Invalid Image Type" << (int)header.ImageType << "\n";
		in.close();
		return false;
	}

	std::clog << "Read tga image " << filename << " : " << m_Width << "x" << m_Height << "/" << m_BytesPerPixel * 8 << "\n";
	in.close();
	return true;
}

bool TGAImage::WriteTGAImage(const char* filename)
{
	// Open the file
	std::ofstream out;
	out.open(filename, std::ios::binary);
	if (!out.is_open())
	{
		std::cerr << "Error opening file " << filename << "\n";
		out.close();
		return false;
	}

	// Create & Write the header
	TGAHeader header;
	memset((void*)&header, 0, sizeof(header));
	header.Width = m_Width;
	header.Height = m_Height;
	header.BitsPerPixel = m_BytesPerPixel << 3;
	header.ImageType = (m_BytesPerPixel == 1) ? 3 : 2;
	header.ImageDescriptor = 0x20;	// Top Left origin
	out.write((char*)&header, sizeof(header));
	if (!out.good())
	{
		std::cerr << "Unable to write TGA Header\n";
		out.close();
		return false;
	}

	// Write the data
	out.write((char*)m_Data, m_Width * m_Height * m_BytesPerPixel);
	if (!out.good())
	{
		std::cerr << "Unable to write TGA Data\n";
		out.close();
		return false;
	}

	out.close();
	return true;
}

bool TGAImage::SetPixel(int x, int y, const TGAColor& c)
{
	if (!m_Data)
		return false;

	if (x < 0 || y < 0 || x >= m_Width || y >= m_Height)
		return false;
	
	memcpy(m_Data + (x + y * m_Width) * m_BytesPerPixel, c.DataBGRA, m_BytesPerPixel);
	return true;
}

