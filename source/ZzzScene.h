#pragma once
#include "ZzzInfomation.h"

extern int MenuStateCurrent;
extern int MenuStateNext;
extern int  SceneFlag;
extern int  MoveSceneFrame;
//extern bool EnableEdit;
extern int  ErrorMessage;
extern bool InitServerList;
extern bool InitLogIn;
extern bool InitLoading;
extern bool InitCharacterScene;
extern bool InitMainScene;
extern bool EnableMainRender;
extern char *szServerIpAddress;
extern unsigned short g_ServerPort;
extern int g_iLengthAuthorityCode;
extern int CreateAccount;

void SetTargetFps(double targetFps);
extern void WaitForNextActivity(bool usePreciseSleep);

extern void LogInScene(HDC hDC);
extern void LoadingScene(HDC hDC);
extern void Scene(HDC Hdc);
extern bool CheckName();
void    StartGame();
extern bool CheckRenderNextFrame();
extern void RenderScene(HDC Hdc);

BOOL	ShowCheckBox( int num, int index, int message=MESSAGE_TRADE_CHECK );

int	SeparateTextIntoLines( const char* lpszText, char *lpszSeparated, int iMaxLine, int iLineSize);

bool	GetTimeCheck(int DelayTime);
void	SetEffectVolumeLevel(int level);
void    SetViewPortLevel ( int level );

bool IsEnterPressed();
void SetEnterPressed( bool enterpressed );

