#pragma once

#include <cstdint>

#pragma pack(push, 1)
struct TGAHeader
{
	uint8_t IDLength;					// Specifies the length of the image ID field
	uint8_t ColorMapType;				// Indicates whether a color map is included in the file. (0 - no map, 1 - map present)
	uint8_t ImageType;					// 0-11: Specifies the type of image. (rle, color-mapped, grayscale, or true-color pixels)
	// COlOR MAP SPECIFICATION
	uint16_t ColorMapFirstEntryIndex;	// Index of the first color map entry.
	uint16_t ColorMapLength;			// Number of entries in the color map
	uint8_t ColorMapEntrySize;			// Number of bits per color map entry
	// IMAGE SPECIFICATION
	uint16_t XOrigin;					// X-coordinate of the lower-left corner where image data starts.
	uint16_t YOrigin;					// Y-coordinate of the lower-left corner where image data starts
	uint16_t Width;						// Width of image in pixels
	uint16_t Height;					// Height of image in pixels
	uint8_t BitsPerPixel;				// Number of bits per pixel in the image
	uint8_t ImageDescriptor;			// Contains various flags and information about the image layout, including the alpha channel and image origin
};
#pragma pack(pop)

struct TGAColor
{
	uint8_t R, G, B, A;
	uint8_t DataBGRA[4];

	TGAColor()
		: R(0), G(0), B(0), A(0), DataBGRA{ 0,0,0,0 }
	{}

	TGAColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
		: R(r), G(g), B(b), A(a), DataBGRA{ b,g,r,a }
	{}

	TGAColor(const uint8_t* bgraData, uint8_t bytesPerPixel)
	{
		for (uint8_t i = 0; i < bytesPerPixel; i++)
		{
			DataBGRA[i] = bgraData[i];
		}
		for (uint8_t i = bytesPerPixel; i < 4; i++)
		{
			DataBGRA[i] = 0;
		}

		B = DataBGRA[0];
		G = DataBGRA[1];
		R = DataBGRA[2];
		A = DataBGRA[3];
	}
};

class TGAImage
{
public:
	~TGAImage();
	TGAImage();
	TGAImage(uint16_t width, uint16_t height, uint8_t bytesPerPixel);

	bool ReadTGAImage(const char* filename);
	bool WriteTGAImage(const char* filename);

	bool SetPixel(int x, int y, const TGAColor& c);

	uint16_t GetWidth() const { return m_Width; }
	uint16_t GetHeight() const { return m_Height; }
	uint8_t GetBytesPerPixel() const { return m_BytesPerPixel; }
	uint8_t* GetBuffer() const { return m_Data; }
private:
	uint16_t m_Width = 0, m_Height = 0;
	uint8_t m_BytesPerPixel = 0;
	uint8_t* m_Data = nullptr;
};