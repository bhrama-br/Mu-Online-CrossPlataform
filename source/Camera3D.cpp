// Camera.cpp: implementation of the CCamera class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Camera3D.h"
#include "ZzzLodTerrain.h"
#include "NewUISystem.h"

CCamera gCamera;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCamera::CCamera() // OK
{
	this->LastPress = GetTickCount();
	this->m_CursorX = MouseX;
	this->m_CursorY = MouseY;
}

CCamera::~CCamera() // OK
{

}

void CCamera::Toggle() // OK
{
	m_CameraOnOff ^= 1;
}

void CCamera::Restore() // OK
{
	CameraZoom = 0;
	CameraAngle[2] = -45.f;
	AngleY3D = 0;
	AngleZ3D = 0;
}


void CCamera::Update() // OK
{
	if (SceneFlag != MAIN_SCENE) return;

	if (HIBYTE(GetAsyncKeyState(VK_F11)) == 128)
	{
		if ((GetTickCount() - this->LastPress) > 2000) {
			this->LastPress = GetTickCount();
			this->Toggle();
		}
		return;
	}

	if (!m_CameraOnOff) return;

	if (HIBYTE(GetAsyncKeyState(VK_F10)) == 128)
	{
		this->Restore();

		return;
	}

	if (HIBYTE(GetAsyncKeyState(VK_MBUTTON)) == 128)
	{
		if (this->m_CursorX < MouseX)
		{
			CameraAngle[2] -= 4;
		}

		if (this->m_CursorX > MouseX)
		{
			CameraAngle[2] += 4;
		}

		if (this->m_CursorY < MouseY)
		{
			if (AngleY3D < 10) {
				AngleY3D += 1.3f;
				AngleZ3D += 24.0f;
			}
		}

		if (this->m_CursorY > MouseY)
		{
			if (AngleY3D > -15) {
				AngleY3D -= 1.3f;
				AngleZ3D -= 24.0f;
			}
		}
	}

	if (HIBYTE(GetAsyncKeyState(VK_RIGHT)) == 128) // RIGHT ARROW
		CameraAngle[2] -= 4;

	if (HIBYTE(GetAsyncKeyState(VK_LEFT)) == 128) // LEFT ARROW
		CameraAngle[2] += 4;

	if (HIBYTE(GetAsyncKeyState(VK_DOWN)) == 128) // DOWN ARROW
		if (AngleY3D < 5) {
			AngleY3D += 1.3f;
			AngleZ3D += 24.0f;
		}

	if (HIBYTE(GetAsyncKeyState(VK_UP)) == 128) // UP ARROW
		if (AngleY3D > -15) {
			AngleY3D -= 1.3f;
			AngleZ3D -= 24.0f;
		}


	if (MouseWheel > 0)
		if (CameraZoom > -15) {
			CameraZoom -= 2;
			if (AngleY3D > -20) {
				AngleY3D -= 1.0f;
				AngleZ3D -= 24.0f;
			}
		}
	if (MouseWheel < 0)
		if (CameraZoom < 12) {
			CameraZoom += 2;
			if (AngleY3D < 10) {
				AngleY3D += 1.0f;
				AngleZ3D += 24.0f;
			}
		}
	if (MouseWheel != 0)
		MouseWheel = 0;

	this->m_CursorX = MouseX;

	this->m_CursorY = MouseY;

	return;
}