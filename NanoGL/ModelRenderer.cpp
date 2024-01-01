#include "ModelRenderer.h"

ModelRenderer::ModelRenderer(const char* filenamme, TGAImage* AOImage, TGAImage* depthImage, float* zbuffer, float* shadowbuffer)
	: m_AOImage(AOImage), m_DepthImage(depthImage), m_Zbuffer(zbuffer), m_ShadowBuffer(shadowbuffer)
{
	m_Model = new Model(filenamme);
	m_Width = m_AOImage->GetWidth();
	m_Height = m_AOImage->GetHeight();
}

ModelRenderer::~ModelRenderer()
{
	delete m_Model;
}

float MaxElevationAngle(float* zbuffer, Vec2f p, Vec2f dir, int width, int height)
{
	float maxangle = 0;
	for (float t = 0.; t < 1000.; t += 1.)
	{
		Vec2f cur = p + dir * t;
		if (cur.x >= width || cur.y >= height || cur.x < 0 || cur.y < 0) return maxangle;

		float distance = (p - cur).Magnitude();
		if (distance < 1.f) continue;
		float elevation = zbuffer[int(cur.x) + int(cur.y) * width] - zbuffer[int(p.x) + int(p.y) * width];
		maxangle = std::max(maxangle, atanf(elevation / distance));
	}
	return maxangle;
}

void ModelRenderer::Render(TGAImage& frame, const Vec3f& eye, const Vec3f& center, const Vec3f& up, const Vec3f& lightDir)
{
	m_Width = frame.GetWidth();
	m_Height = frame.GetHeight();

		std::clog << "Calculating Ambient Occlusion..." << std::endl;
		LookAt(eye, center, up);
		CreateViewportMatrix(m_Width / 8, m_Height / 8, m_Width * 3 / 4, m_Height * 3 / 4);
		CreateProjectionMatrix(-1.0f / (eye - center).Magnitude());

		ZShader zshader(*m_Model);
		Vec4f screenCoords[3];
		for (int i = 0; i < m_Model->nFaces(); i++) {
			for (int j = 0; j < 3; j++) {
				screenCoords[j] = zshader.Vertex(i, j);
			}
			Triangle(screenCoords, zshader, *m_AOImage, m_Zbuffer);
		}

		for (int x = 0; x < m_Width; x++) {
			for (int y = 0; y < m_Height; y++) {
				if (m_Zbuffer[x + y * m_Width] < -1e5) continue;
				float total = 0;
				for (float a = 0; a < M_PI * 2 - 1e-4; a += M_PI / 4) {
					total += M_PI / 2 - MaxElevationAngle(m_Zbuffer, Vec2f(x, y), Vec2f(cos(a), sin(a)), m_Width, m_Height);
				}
				total /= (M_PI / 2) * 8;
				total = pow(total, 100.f);
				m_AOImage->SetPixel(x, y, TGAColor(total * 255, total * 255, total * 255));
			}
		}

		std::clog << "DONE" << std::endl;

	{
		std::clog << "Calculate Depth Map..." << std::endl;

		LookAt(lightDir, center, up);
		CreateViewportMatrix(m_Width / 8, m_Height / 8, m_Width * 3 / 4, m_Height * 3 / 4);
		CreateProjectionMatrix(0);

		DepthShader depthShader(*m_Model);
		Vec4f screenCoords[3];
		for (int i = 0; i < m_Model->nFaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				screenCoords[j] = depthShader.Vertex(i, j);
			}
			Triangle(screenCoords, depthShader, *m_DepthImage, m_ShadowBuffer);
		}

		std::clog << "DONE" << std::endl;
	}

	Mat4x4 MShadow = Viewport * Projection * ModelView;

	{
		std::clog << "Rendering Final Image..." << std::endl;

		LookAt(eye, center, up);
		CreateViewportMatrix(m_Width / 8, m_Height / 8, m_Width * 3 / 4, m_Height * 3 / 4);
		CreateProjectionMatrix(-1.0f / (eye - center).Magnitude());

		Shader shader(Viewport*Projection*ModelView, (Viewport*Projection * ModelView).InvertTranspose(), MShadow * (Viewport * Projection * ModelView).Invert(), *m_Model, lightDir, m_ShadowBuffer, m_AOImage);
		Vec4f screenCoords[3];
		for (int i = 0; i < m_Model->nFaces(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
				screenCoords[j] = shader.Vertex(i, j);
			}
			Triangle(screenCoords, shader, frame, m_Zbuffer);
		}

		std::clog << "DONE" << std::endl;
	}
}
