#include "stdafx.h"

#if defined(__ANDROID__)

#include "GlobalBitmap.h"
#include "UIControls.h"
#include "Widescreen.h"
#include "ZzzOpenglUtil.h"

#include "Platform/RenderBackend.h"

namespace
{
	const GLuint kLegacyMainFrameImageCustomId = 120000;

	void SetQuadVertex(platform::QuadVertex2D* vertex, float x, float y, float u, float v, float r, float g, float b, float a)
	{
		if (vertex == NULL)
		{
			return;
		}

		vertex->x = x;
		vertex->y = y;
		vertex->u = u;
		vertex->v = v;
		vertex->r = r;
		vertex->g = g;
		vertex->b = b;
		vertex->a = a;
	}

	float ConvertLegacyX(float x)
	{
		return x * g_fScreenRate_x;
	}

	float ConvertLegacyY(float y)
	{
		return y * g_fScreenRate_y;
	}

	void BindLegacyTexture(GLuint texture)
	{
		platform::GetRenderBackend().SetTextureEnabled(true);
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	GLuint ResolveLegacyTextureNumber(int texture_index)
	{
		if (texture_index <= 0)
		{
			return texture_index < 0 ? static_cast<GLuint>(-texture_index) : 0u;
		}

		BITMAP_t* bitmap = Bitmaps.FindTexture(static_cast<GLuint>(texture_index));
		if (bitmap == NULL)
		{
			bitmap = Bitmaps.GetTexture(static_cast<GLuint>(texture_index));
		}

		return bitmap != NULL ? bitmap->TextureNumber : 0u;
	}
}

#if !defined(MU_ANDROID_HAS_ZZZOPENGLUTIL_RUNTIME)
int MouseUpdateTime = 0;
int MouseUpdateTimeMax = 6;

CWideScreen GWidescreen;

void RenderBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth, float vHeight, bool Scale, bool StartScale, float Alpha)
{
	if (StartScale)
	{
		x = ConvertLegacyX(x);
		y = ConvertLegacyY(y);
	}
	if (Scale)
	{
		Width = ConvertLegacyX(Width);
		Height = ConvertLegacyY(Height);
	}

	BindLegacyTexture(ResolveLegacyTextureNumber(Texture));

	y = static_cast<float>(WindowHeight) - y;

	const float alpha = Alpha > 0.0f ? Alpha : 1.0f;
	platform::QuadVertex2D vertices[4];
	SetQuadVertex(&vertices[0], x, y, u, v, 1.0f, 1.0f, 1.0f, alpha);
	SetQuadVertex(&vertices[1], x, y - Height, u, v + vHeight, 1.0f, 1.0f, 1.0f, alpha);
	SetQuadVertex(&vertices[2], x + Width, y - Height, u + uWidth, v + vHeight, 1.0f, 1.0f, 1.0f, alpha);
	SetQuadVertex(&vertices[3], x + Width, y, u + uWidth, v, 1.0f, 1.0f, 1.0f, alpha);

	platform::GetRenderBackend().DrawQuad2D(vertices, true);
	platform::GetRenderBackend().SetCurrentColor(1.0f, 1.0f, 1.0f, 1.0f);
}
#else
int MouseUpdateTime = 0;
int MouseUpdateTimeMax = 6;

CWideScreen GWidescreen;
#endif

void CWideScreen::Init()
{
	DisplayWinCDepthBox = static_cast<int>(static_cast<float>(WindowWidth) / g_fScreenRate_y - 640.0f);
	FrameWinCDepthBox = static_cast<int>(static_cast<float>(WindowWidth) / g_fScreenRate_y - 640.0f);
	JCWinWidth = static_cast<int>(static_cast<float>(WindowWidth) / g_fScreenRate_y);
	JCWinWidthAdd = JCWinWidth / 2 - 320;
	WidescreenPosX1 = JCWinWidth - (640 - 450);
	WidescreenPosX2 = JCWinWidth - (640 - 260);
	g_WideWindowWidth = static_cast<float>(WindowWidth) / g_fScreenRate_y;
	g_WideWindowWidthAdd = g_WideWindowWidth - 640.0f;
	fScreen_Width = GetWindowsX;
}

void CWideScreen::RenderImages(GLuint Image, float x, float y, float width, float height, float su, float sv, float uw, float vh)
{
	BITMAP_t* texture = Bitmaps.GetTexture(Image);
	if (texture == NULL || texture->Width <= 0.0f || texture->Height <= 0.0f)
	{
		return;
	}

	RenderBitmap(Image, x, y, width, height, su / texture->Width, sv / texture->Height, uw / texture->Width, vh / texture->Height, true, true, 0.0f);
}

void CWideScreen::RenderImages3F(GLuint Image, float x, float y, float width, float height, unsigned int output_width, unsigned int output_height)
{
	BITMAP_t* texture = Bitmaps.GetTexture(Image);
	if (texture == NULL || texture->Width <= 0.0f || texture->Height <= 0.0f)
	{
		return;
	}

	const float uw = output_width > 0 ? static_cast<float>(output_width) / texture->Width : 0.0f;
	const float vh = output_height > 0 ? static_cast<float>(output_height) / texture->Height : 0.0f;
	RenderBitmap(Image, x, y, width, height, 0.0f, 0.0f, uw, vh, true, true, 0.0f);
}

void CWideScreen::RenderImageF(GLuint Image, float x, float y, float width, float height, float su, float sv, float uw, float vh)
{
	BITMAP_t* texture = Bitmaps.GetTexture(Image);
	if (texture == NULL || texture->Width <= 0.0f || texture->Height <= 0.0f)
	{
		return;
	}

	RenderBitmap(Image, x, y, width, height, su / texture->Width, sv / texture->Height, uw / texture->Width, vh / texture->Height, true, true, 0.0f);
}

double CWideScreen::RenderNumbers(float a1, float a2, int a3, float a4, float a5)
{
	char text[32] = { 0 };
	itoa(a3, text, 10);
	const int length = static_cast<int>(strlen(text));
	float render_x = static_cast<float>(a1 - static_cast<double>(length) * a4 / 2.0);

	for (int index = 0; index < length; ++index)
	{
		const float u = static_cast<float>(text[index] - '0') * 16.0f / 256.0f;
		RenderBitmap(kLegacyMainFrameImageCustomId - 1, render_x, static_cast<float>(a2), a4, a5, u, 0.0f, 0.0625f, 0.5f, true, true, 0.0f);
		render_x += a4 * 0.7f;
	}

	return render_x;
}

void CWideScreen::RenderButtons(int Image, float x, float y, float width, float heith)
{
	float sv = 0.0f;
	if (::CheckMouseIn(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(heith)))
	{
		sv += MouseLButton ? 108.0f : 54.0f;
	}

	RenderImages(Image, x, y, width, heith, 0.0f, sv, 53.5f, 53.5f);
}

void CWideScreen::RenderMenuButton(int This)
{
	const float x = *reinterpret_cast<float*>(This + 44);
	const float y = *reinterpret_cast<float*>(This + 48);
	const float width = *reinterpret_cast<float*>(This + 52);
	const float height = *reinterpret_cast<float*>(This + 56);
	const int event_state = *reinterpret_cast<int*>(This + 80);
	RenderImages(931324, x, y, width, height, 0.0f, event_state * 54.0f, 294.5f, 53.5f);
}

void CWideScreen::CreateNButton(int Image, float x, float y, float width, float heith, int windows)
{
	(void)windows;

	float sv = 0.0f;
	if (::CheckMouseIn(static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(heith)))
	{
		sv += MouseLButton ? 108.0f : 54.0f;
	}

	RenderImages(Image, x, y, width, heith, 0.0f, sv, 53.5f, 54.0f);
}

void CWideScreen::SceneLogin()
{
}

#endif
