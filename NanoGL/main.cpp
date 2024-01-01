#include <iostream>
#include <vector>
#include <limits>

#include "tgaimage.h"
#include "geometry.h"
#include "nanogl.h"
#include "ModelRenderer.h"

constexpr int width = 800;
constexpr int height = 800;

const Vec3f lightDir(1, 1, 2);
const Vec3f eye(1, 1, 4);
const Vec3f center(0, 0, 0);
const Vec3f up(0, 1, 0);

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << "obj/model.obj" << std::endl;
		//return 1;
	}

	float* zbuffer = new float[width * height];
	float* shadowbuffer = new float[width * height];
	for (int i = width * height; i--; zbuffer[i] = shadowbuffer[i] = -std::numeric_limits<float>::max());

	TGAImage* AOImage = new TGAImage(width, height, 3);
	TGAImage* depthImage = new TGAImage(width, height, 3);
	TGAImage frame(width, height, 3);
	
	// Set BG
	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			frame.SetPixel(x, y, TGAColor(20, 20, 20));
		}
	}
	
	for (int i = 0; i < argc - 1; i++)
	{
		ModelRenderer modelRenderer(argv[i+1], AOImage, depthImage, zbuffer, shadowbuffer);
		modelRenderer.Render(frame, eye, center, up, lightDir);
	}

	{
		ModelRenderer modelRenderer("obj/african_head/african_head_eye_inner.obj", AOImage, depthImage, zbuffer, shadowbuffer);
		modelRenderer.Render(frame, eye, center, up, lightDir);
	}
	{
		ModelRenderer modelRenderer("obj/african_head/african_head.obj", AOImage, depthImage, zbuffer, shadowbuffer);
		modelRenderer.Render(frame, eye, center, up, lightDir);
	}

	AOImage->FlipVertical();
	AOImage->WriteTGAImage("ao.tga");
	
	depthImage->FlipVertical();
	depthImage->WriteTGAImage("depth.tga");

	frame.FlipVertical();
	frame.WriteTGAImage("framebuffer.tga");

	delete AOImage;
	delete depthImage;
	delete[] zbuffer;
	delete[] shadowbuffer;
	
	return 0;
}