#include "stdafx.h"
#include "Widescreen.h"
#include "GlobalBitmap.h"
#include "MapManager.h"
#include "ZzzScene.h"
#if !defined(__ANDROID__)
#include "NewUIMainFrameWindow.h"
#include "NewUISystem.h"
#endif

namespace
{
	const GLuint kLegacyMainFrameImageCustomId = 120000;
}

CWideScreen GWidescreen;

void CWideScreen::Init()
{
	DisplayWinCDepthBox = (float)WindowWidth / g_fScreenRate_y - 640;

	FrameWinCDepthBox = ((float)WindowWidth / g_fScreenRate_y - 640);

	JCWinWidth = (float)WindowWidth / g_fScreenRate_y;

	JCWinWidthAdd = JCWinWidth / 2 - 320;

	// Janelas
	WidescreenPosX1 = JCWinWidth - (640 - 450);		// Apenas (1) UMA janela aberta
	WidescreenPosX2 = JCWinWidth - (640 - 260);		// Apenas (2) DUAS janelas abertas

	g_WideWindowWidth = (float)WindowWidth / g_fScreenRate_y;
	g_WideWindowWidthAdd = this->g_WideWindowWidth - 640;

	fScreen_Width = GetWindowsX;
}

void CWideScreen::RenderImages(GLuint Image, float x, float y, float width, float height, float su, float sv, float uw, float vh)
{
	BITMAP_t* pText = &Bitmaps[Image];
	RenderBitmap(Image, x, y, width, height, su / pText->Width, sv / pText->Height, uw / pText->Width, vh / pText->Height, 1, 1, 0.0);
}

void CWideScreen::RenderImages3F(GLuint Image, float x, float y, float width, float height, unsigned int output_width, unsigned int output_height)
{
	BITMAP_t* pText = &Bitmaps[Image];

	float uw, vh;

	uw = (float)output_width / (float)pText->Width;
	vh = (float)output_height / (float)pText->Height;

	RenderBitmap(Image, x, y, width, height, 0.0, 0.0, uw, vh, true, true, 0.0);
}

void CWideScreen::RenderImageF(GLuint Image, float x, float y, float width, float height, float su, float sv, float uw, float vh)
{
	BITMAP_t* pText = &Bitmaps[Image];
	RenderBitmap(Image, x, y, width, height, su / pText->Width, sv / pText->Height, uw / pText->Width, vh / pText->Height, 1, 1, 0.0);
}

double CWideScreen::RenderNumbers(float a1, float a2, int a3, float a4, float a5)
{
	float v10; // [sp+38h] [bp+8h]@1
	EnableAlphaTest(1);

	char strText[32];
	itoa(a3, strText, 10);
	int iLength = (int)strlen(strText);

	v10 = a1 - (double)iLength * a4 / 2.0;

	for (int i = 0; i < iLength; ++i)
	{
		float fU = (float)(strText[i] - 48) * 16.f / 256.f;
		RenderBitmap(kLegacyMainFrameImageCustomId - 1, v10, a2, a4, a5, fU, 0, 0.0625, 0.5, 1, 1, 0.0);
		v10 = a4 * 0.69999999 + v10;
	}

	return v10;
}

void CWideScreen::RenderButtons(int Image, float x, float y, float width, float heith)
{
	float sv = 0.0;

	if (SEASON3B::CheckMouseIn((int)x, (int)y, (int)width, (int)heith))
	{
		sv += (MouseLButton) ? 108.f : 54.f;
	}

	RenderImages(Image, x, y, width, heith, 0.0, sv, 53.5f, 53.5f);
}

void CWideScreen::RenderMenuButton(int This)
{
	float m_x, m_y, m_w, m_h;

	m_x = *(float*)(This + 44);
	m_y = *(float*)(This + 48);
	m_w = *(float*)(This + 52);
	m_h = *(float*)(This + 56);

	int m_EventState = *(int*)(This + 80);

	RenderImages(931324, m_x, m_y, m_w, m_h, 0.0, m_EventState * 54.f, 294.5f, 53.5f);
}

void CWideScreen::CreateNButton(int Image, float x, float y, float width, float heith, int windows)
{
	float sv = 0.0;

	if (SEASON3B::CheckMouseIn((int)x, (int)y, (int)width, (int)heith))
	{
		sv += (MouseLButton) ? 108.f : 54.f;
	}

#if !defined(__ANDROID__)
	if(g_pNewUISystem->IsVisible(windows))
	{
		sv = 108.f;
	}
#endif
	RenderImages(Image, x, y, width, heith, 0.0, sv, 53.5f, 54.f);
}

void CWideScreen::SceneLogin()
{
	if (gMapManager.WorldActive == 94 && SceneFlag == LOG_IN_SCENE)
	{
		CameraViewFar			= 33000.0;		//	CameraViewFar
		CameraAngle[0]			= -84.0;		//  CameraAngle[0]
		CameraAngle[1]			= 0.0;			//  CameraAngle[1]
		CameraAngle[2]			= -45.0;		//  CameraAngle[2]
		CameraPosition[0]		= 24475.79687;	//  CameraPosition[0]
		CameraPosition[1]		= 7581.581055;	//	CameraPosition[1]
		CameraPosition[2]		= 1834.539917;	//  CameraPosition[2]
		CameraFOV				= 35.0;			//  CameraFOV
	}

	if (SceneFlag == CHARACTER_SCENE)
	{
		CameraViewFar			= 3500.0;
		CameraAngle[0]			= -82.0; //meu Y
		CameraAngle[1]			= 0.0;
		CameraAngle[2]			= -90.0;
		CameraPosition[0]		= 9758.9297;
		CameraPosition[1]		= 18913.109;
		CameraPosition[2]		= 500.0;
		CameraFOV				= 35.0;
	}
}
