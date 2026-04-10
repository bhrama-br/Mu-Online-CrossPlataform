#pragma once

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

#define GetWindowsX								(WindowWidth  / g_fScreenRate_x)
#define GetCenterX(x)							((GetWindowsX - x) / 2.f)
#define GetWindowsY								(WindowHeight / g_fScreenRate_y)
#define setPosMidRight(x)						((GetWindowsX - 640.f) / 2.f + x )
#define setPosCenterX(x)						((GetWindowsX - x) / 2.f)
#define GetCenterPosition(x)					(x + (GetWindowsX / 2.f))
#define SubWindowsX(x)							(GetWindowsX - x )
#define setPosRight(x)							(GetWindowsX - 640.f + x )
#define GetWindowsCX(x)							((float)(WindowWidth / g_fScreenRate_x /2.f) - (x / 2.f))
#define setPosDown(x)							(GetWindowsY - 480.f + x )

class CWideScreen
{
public:
	void Init();
	void RenderImages(GLuint Image, float x, float y, float width, float height, float su, float sv, float uw, float vh);
	double RenderNumbers(float a1, float a2, int a3, float a4, float a5);
	void RenderImages3F(GLuint Image, float x, float y, float width, float height, unsigned int output_width = 0, unsigned int output_height = 0);
	void RenderImageF(GLuint Image, float x, float y, float width, float height, float su, float sv, float uw, float vh);
	void RenderButtons(int Image, float x, float y, float width, float heith);
	void RenderMenuButton(int This);
	void CreateNButton(int Image, float x, float y, float width, float heith, int windows);
	void SceneLogin();

public:
	int JCWinWidth;
	int JCWinWidthAdd;
	int DisplayWinCDepthBox; // eax@4
	int FrameWinCDepthBox;
	
public:
	int WidescreenPosX1;
	int WidescreenPosX2;
	double fScreen_Width;

public:
	float g_WideWindowWidth;
	float g_WideWindowWidthAdd;
};

extern CWideScreen GWidescreen;