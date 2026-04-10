///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Platform/RenderColorCompat.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "zzzObject.h"
#include "zzzcharacter.h"
#include "Zzzinfomation.h"
#include "NewUISystem.h"
#include "Platform/RenderBackend.h"

int     OpenglWindowX;     
int     OpenglWindowY;     
int     OpenglWindowWidth; 
int     OpenglWindowHeight;
bool    CameraTopViewEnable = false;
float   CameraViewNear      = 20.f;
float   CameraViewFar       = 2000.f;
float   CameraFOV           = 55.f;
float	CameraZoom = 0.f;
float	AngleX3D = 0.f;
float	AngleY3D = 0.f;
float	AngleZ3D = 0.f;
float	AngleRL = 0.f;
vec3_t  CameraPosition;
vec3_t  CameraAngle;
float   CameraMatrix[3][4];
vec3_t  MousePosition;
vec3_t  MouseTarget;
float   g_fCameraCustomDistance = 0.f;
bool    FogEnable   = false;
GLfloat FogDensity  = 0.0004f;
GLfloat FogColor[4] = {30/256.f,20/256.f,10/256.f,};

unsigned int WindowWidth  = 1024;
unsigned int WindowHeight = 768;
int          MouseX       = WindowWidth/2;
int          MouseY       = WindowHeight/2;
int          BackMouseX   = MouseX;
int          BackMouseY   = MouseY;
bool         MouseLButton;
bool 		 MouseLButtonPop;
bool 		 MouseLButtonPush;
bool         MouseRButton;
bool 		 MouseRButtonPop;
bool 		 MouseRButtonPush;
bool 	   	 MouseLButtonDBClick;
bool         MouseMButton;
bool         MouseMButtonPop;
bool         MouseMButtonPush;
int          MouseWheel;
DWORD		 MouseRButtonPress = 0;
bool		 BlockClick = false;
bool		 LockPlayerWalk = false;

namespace
{
	void SetMultisampleCapability(bool enabled)
	{
#if defined(GL_MULTISAMPLE_ARB)
		if (enabled)
		{
			glEnable(GL_MULTISAMPLE_ARB);
		}
		else
		{
			glDisable(GL_MULTISAMPLE_ARB);
		}
#elif defined(GL_MULTISAMPLE)
		if (enabled)
		{
			glEnable(GL_MULTISAMPLE);
		}
		else
		{
			glDisable(GL_MULTISAMPLE);
		}
#else
		(void)enabled;
#endif
	}

	void SetQuadVertex(platform::QuadVertex2D* vertex, float x, float y, float u, float v, float r, float g, float b, float a)
	{
		vertex->x = x;
		vertex->y = y;
		vertex->u = u;
		vertex->v = v;
		vertex->r = r;
		vertex->g = g;
		vertex->b = b;
		vertex->a = a;
	}

	void SetVertex3D(platform::Vertex3D* vertex, const vec3_t position, float u, float v, float r, float g, float b, float a)
	{
		vertex->x = position[0];
		vertex->y = position[1];
		vertex->z = position[2];
		vertex->u = u;
		vertex->v = v;
		vertex->r = r;
		vertex->g = g;
		vertex->b = b;
		vertex->a = a;
	}
}

//bool    showShoppingMall = false;

void OpenExploper(char *Name,char *para)
{
	ShellExecute( NULL, "open", Name, para, "", SW_SHOW);
}

bool CheckID_HistoryDay ( char* Name, WORD day )
{
    typedef struct  __day_history__
    {
        char ID[MAX_ID_SIZE+1];
        WORD date;
    }dayHistory;

    FILE* fp;
    dayHistory days[100];
    int   count = 0;
    WORD  num = 0;
    bool  sameName = false;
    bool  update = true;

    if ( ( fp=fopen( "dconfig.ini", "rb" ) )!=NULL )
    {
        fread ( &num, sizeof( WORD ), 1, fp );
        
        if ( num>100 )
        {
            num = 0;
        }
        else
        {
            for ( int i=0; i<num; ++i )
            {
                fread ( days[i].ID, sizeof( char ), MAX_ID_SIZE+1, fp );
                fread ( &days[i].date, sizeof( WORD ), 1, fp );

                if ( !strcmp( days[i].ID, Name ) )
                {
                    sameName = true;
                    if ( days[i].date==day )
                    {
                        update = false;
                        break;
                    }
                    days[i].date = day;
                }
                count++;
            }
        }
        fclose ( fp );
    }

    if ( update )
    {
        if ( !sameName )
        {
            memcpy ( days[num].ID, Name, (MAX_ID_SIZE+1)*sizeof( char ) );
            days[num].date = day;

            num++;
        }

        fp = fopen ( "dconfig.ini", "wb" );

        fwrite ( &num, sizeof( WORD ), 1, fp );
        for ( int i=0; i<num; ++i )
        {
            fwrite ( days[i].ID, sizeof( char ), MAX_ID_SIZE+1, fp );
            fwrite ( &days[i].date, sizeof( WORD ), 1, fp );
        }

        fclose ( fp );
    }

//    showShoppingMall = update;

    return  update;
}

bool GrabEnable = false;
char GrabFileName[MAX_PATH];
int  GrabScreen = 0;
bool GrabFirst = false;

void SaveScreen()
{
	GrabFirst = true;

	/*if(!GrabFirst)
	{
		GrabFirst = true;
		for(int i=0;i<10000;i++)
		{
			GrabScreen = i;
			if(GrabScreen<10)
				sprintf(GrabFileName,"Screen000%d",GrabScreen);
			else if(GrabScreen<100)
				sprintf(GrabFileName,"Screen00%d",GrabScreen);
			else if(GrabScreen<1000)
				sprintf(GrabFileName,"Screen0%d",GrabScreen);
			else
				sprintf(GrabFileName,"Screen%d",GrabScreen);

			strcat( GrabFileName, lpszFileName);
			FILE *fp = fopen(GrabFileName,"rb");
			if(fp==NULL)
				break;
			else
				fclose(fp);
		}
	}
	else
	{
		if(GrabScreen<10)
			sprintf(GrabFileName,"Screen000%d",GrabScreen);
		else if(GrabScreen<100)
			sprintf(GrabFileName,"Screen00%d",GrabScreen);
		else if(GrabScreen<1000)
			sprintf(GrabFileName,"Screen0%d",GrabScreen);
		else
			sprintf(GrabFileName,"Screen%d",GrabScreen);

		strcat( GrabFileName, lpszFileName);
	}*/

	unsigned char *Buffer = new unsigned char [(int)WindowWidth*(int)WindowHeight*3];
	glReadPixels(0,0,(int)WindowWidth,(int)WindowHeight,GL_RGB,GL_UNSIGNED_BYTE,Buffer);
	WriteJpeg(GrabFileName,(int)WindowWidth,(int)WindowHeight,Buffer,100);

	SAFE_DELETE_ARRAY(Buffer);
	
	GrabScreen++;
	GrabScreen %= 10000;
}

float PerspectiveX;
float PerspectiveY;
int   ScreenCenterX;
int   ScreenCenterY;
int   ScreenCenterYFlip;

void GetOpenGLMatrix(float Matrix[3][4])
{
	float OpenGLMatrix[16];
	platform::GetRenderBackend().CopyModelViewMatrix(OpenGLMatrix);
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<4;j++)
		{
			Matrix[i][j] = OpenGLMatrix[j*4+i];
		}
	}
}

void gluPerspective2(float Fov,float Aspect,float ZNear,float ZFar)
{
	(void)ZNear;
	(void)ZFar;

	ScreenCenterX      = OpenglWindowX + OpenglWindowWidth/2;
	ScreenCenterY      = OpenglWindowY + OpenglWindowHeight/2;
	ScreenCenterYFlip  = WindowWidth - ScreenCenterY;

	float AspectY = (float)(WindowHeight)/(float)(OpenglWindowHeight);
	PerspectiveX = tanf(Fov*0.5f*3.141592f/180.f)/(float)(OpenglWindowWidth /2)*Aspect;
	PerspectiveY = tanf(Fov*0.5f*3.141592f/180.f)/(float)(OpenglWindowHeight/2)*AspectY;
	//PerspectiveX = (float)ScreenCenterX/tanf(Fov*0.5f*3.141592f/180.f)*Aspect;
	//PerspectiveY = (float)ScreenCenterY/tanf(Fov*0.5f*3.141592f/180.f);
}

extern float g_fScreenRate_x;	// ¡Ø
extern float g_fScreenRate_y;

DWORD ConvertWX(float a1)	// ConvertModelX (1.0
{
	//return a1 * 1.25;
	return (DWORD)(g_fScreenRate_y <= 1.25) ? a1 * g_fScreenRate_x : a1 * g_fScreenRate_y;
}

void CreateScreenVector ( int sx, int sy, vec3_t Target, bool bFixView )
{
	sx = ConvertWX(sx);
	sy = sy * WindowHeight / GetWindowsY;
	vec3_t p1,p2;
    if ( bFixView )
    {
	    p1[0] =  (float)(sx-ScreenCenterX) * CameraViewFar * PerspectiveX;
	    p1[1] = -(float)(sy-ScreenCenterY) * CameraViewFar * PerspectiveY;
	    p1[2] = -CameraViewFar;
    }
    else
    {
	    p1[0] =  (float)(sx-ScreenCenterX) * RENDER_ITEMVIEW_FAR * PerspectiveX;
	    p1[1] = -(float)(sy-ScreenCenterY) * RENDER_ITEMVIEW_FAR * PerspectiveY;
	    p1[2] = -RENDER_ITEMVIEW_FAR;
    }

	p2[0] = -CameraMatrix[0][3];
	p2[1] = -CameraMatrix[1][3];
	p2[2] = -CameraMatrix[2][3];
	VectorIRotate(p2,CameraMatrix,MousePosition);
	VectorIRotate(p1,CameraMatrix,p2);
	VectorAdd(MousePosition,p2,Target);
}

void Projection(vec3_t Position,int *sx,int *sy)
{
	vec3_t TrasformPosition;
	VectorTransform(Position,CameraMatrix,TrasformPosition);
	*sx = -(int)(TrasformPosition[0] / PerspectiveX / TrasformPosition[2]) + ScreenCenterX;
	*sy =  (int)(TrasformPosition[1] / PerspectiveY / TrasformPosition[2]) + ScreenCenterY;
	*sx = GetWindowsX * *sx / WindowWidth;
	*sy = *sy * GetWindowsY / (int)WindowHeight;
}

void TransformPosition(vec3_t Position,vec3_t WorldPosition,int *x,int *y)
{
	vec3_t Temp;
	VectorSubtract(Position,CameraPosition,Temp);
	VectorRotate(Temp,CameraMatrix,WorldPosition);

	*x = (int)(WorldPosition[0]/PerspectiveX/-WorldPosition[2]) + (ScreenCenterX);
	*y = (int)(WorldPosition[1]/PerspectiveY/-WorldPosition[2]) + (ScreenCenterYFlip);
	//*y = (int)(WorldPosition[1]/PerspectiveY/-WorldPosition[2]) + (WindowHeight/2);
}

bool TestDepthBuffer(vec3_t Position)
{
	vec3_t WorldPosition;
	int x,y;
    TransformPosition(Position,WorldPosition,&x,&y);
	if(x<OpenglWindowX ||
		y<OpenglWindowY ||
		x>=(int)OpenglWindowX+OpenglWindowWidth ||
		y>=(int)OpenglWindowY+OpenglWindowHeight) return false;

	GLfloat key[3];
    glReadPixels(x,y,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,key);

	float z = 1.f - CameraViewNear/-WorldPosition[2] + CameraViewNear/CameraViewFar;
	if(key[0] >= z) return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// opengl render util
///////////////////////////////////////////////////////////////////////////////

int  CachTexture = -1;
bool TextureEnable;
bool DepthTestEnable;
bool CullFaceEnable;
bool DepthMaskEnable;
bool AlphaTestEnable;
int  AlphaBlendType;

void BindTexture(int tex)
{
	if(CachTexture != tex)
	{
      	CachTexture = tex;
		if (tex >= 0)
		{
			BITMAP_t *b = &Bitmaps[tex];
			glBindTexture(GL_TEXTURE_2D,b->TextureNumber);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, -1 * tex);
		}
	}
}

bool TextureStream = false;

extern  int test;
void BindTextureStream(int tex)
{
	BindTexture(tex);
	TextureStream = true;
}

void EndTextureStream()
{
	TextureStream = false;
}

void EnableDepthTest()
{
    if(!DepthTestEnable) 
	{
		DepthTestEnable = true;
		platform::GetRenderBackend().SetDepthTestEnabled(true);
	}
}

void DisableDepthTest()
{
    if(DepthTestEnable) 
	{
		DepthTestEnable = false;
		platform::GetRenderBackend().SetDepthTestEnabled(false);
	}
}

void EnableDepthMask()
{
    if(!DepthMaskEnable) 
	{
		DepthMaskEnable = true;
		platform::GetRenderBackend().SetDepthMaskEnabled(true);
	}
}

void DisableDepthMask()
{
    if(DepthMaskEnable) 
	{
		DepthMaskEnable = false;
		platform::GetRenderBackend().SetDepthMaskEnabled(false);
	}
}

void EnableCullFace()
{
    if(!CullFaceEnable) 
	{
		CullFaceEnable = true;
		platform::GetRenderBackend().SetCullFaceEnabled(true);
	}
}

void DisableCullFace()
{
    if(CullFaceEnable) 
	{
		CullFaceEnable = false;
		platform::GetRenderBackend().SetCullFaceEnabled(false);
	}
}

void DisableTexture( bool AlphaTest )
{
    EnableDepthMask();
    if ( AlphaTest==true )
    {
        if(!AlphaTestEnable) 
	    {
		    AlphaTestEnable = true;
			platform::GetRenderBackend().SetAlphaTestEnabled(true);
	    }
    }
    else
    {
        if(AlphaTestEnable) 
	    {
		    AlphaTestEnable = false;
			platform::GetRenderBackend().SetAlphaTestEnabled(false);
	    }
    }
    if(TextureEnable) 
	{
		TextureEnable = false;
		platform::GetRenderBackend().SetTextureEnabled(false);
	}
}

void DisableAlphaBlend()
{
    if(AlphaBlendType != 0) 
	{
		AlphaBlendType = 0;
		platform::GetRenderBackend().SetBlendState(false, GL_ONE, GL_ZERO);
	}
    EnableCullFace();
    EnableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
		platform::GetRenderBackend().SetAlphaTestEnabled(false);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		platform::GetRenderBackend().SetTextureEnabled(true);
	}
	if(FogEnable)
		platform::GetRenderBackend().SetFogEnabled(true);
}

void EnableAlphaTest(bool DepthMask)
{
    if(AlphaBlendType != 2)
	{
		AlphaBlendType = 2;
		platform::GetRenderBackend().SetBlendState(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
    DisableCullFace();
	if(DepthMask)
        EnableDepthMask();
    if(!AlphaTestEnable) 
	{
		AlphaTestEnable = true;
		platform::GetRenderBackend().SetAlphaTestEnabled(true);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		platform::GetRenderBackend().SetTextureEnabled(true);
	}
	if(FogEnable)
		platform::GetRenderBackend().SetFogEnabled(true);
}

void EnableAlphaBlend()
{
    if(AlphaBlendType != 3)
	{
		AlphaBlendType = 3;
		platform::GetRenderBackend().SetBlendState(true, GL_ONE, GL_ONE);
	}
    DisableCullFace();
    DisableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
		platform::GetRenderBackend().SetAlphaTestEnabled(false);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		platform::GetRenderBackend().SetTextureEnabled(true);
	}
	if(FogEnable)
		platform::GetRenderBackend().SetFogEnabled(false);
}

void EnableAlphaBlendMinus()
{
    if(AlphaBlendType != 4)
	{
		AlphaBlendType = 4;
		platform::GetRenderBackend().SetBlendState(true, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);
	}
    DisableCullFace();
    DisableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
		platform::GetRenderBackend().SetAlphaTestEnabled(false);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		platform::GetRenderBackend().SetTextureEnabled(true);
	}
	if(FogEnable)
		platform::GetRenderBackend().SetFogEnabled(true);
}

void EnableAlphaBlend2()
{
    if(AlphaBlendType != 5)
	{
		AlphaBlendType = 5;
		platform::GetRenderBackend().SetBlendState(true, GL_ONE_MINUS_SRC_COLOR, GL_ONE);
	}
    DisableCullFace();
    DisableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
		platform::GetRenderBackend().SetAlphaTestEnabled(false);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		platform::GetRenderBackend().SetTextureEnabled(true);
	}
	if(FogEnable)
		platform::GetRenderBackend().SetFogEnabled(true);
}

void EnableAlphaBlend3()
{
    if(AlphaBlendType != 6)
	{
		AlphaBlendType = 6;
		platform::GetRenderBackend().SetBlendState(true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
    DisableCullFace();
    DisableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
		platform::GetRenderBackend().SetAlphaTestEnabled(false);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		platform::GetRenderBackend().SetTextureEnabled(true);
	}
	if(FogEnable)
		platform::GetRenderBackend().SetFogEnabled(true);
}

void EnableAlphaBlend4()
{
    if(AlphaBlendType != 7)
	{
		AlphaBlendType = 7;
		platform::GetRenderBackend().SetBlendState(true, GL_ONE, GL_ONE_MINUS_SRC_COLOR);
	}
    DisableCullFace();
    DisableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
		platform::GetRenderBackend().SetAlphaTestEnabled(false);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		platform::GetRenderBackend().SetTextureEnabled(true);
	}
	if(FogEnable)
		platform::GetRenderBackend().SetFogEnabled(true);
}

void EnableLightMap()
{
    if(AlphaBlendType != 1)
	{
		AlphaBlendType = 1;
		platform::GetRenderBackend().SetBlendState(true, GL_ZERO, GL_SRC_COLOR);
	}
    EnableCullFace();
    EnableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
		platform::GetRenderBackend().SetAlphaTestEnabled(false);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		platform::GetRenderBackend().SetTextureEnabled(true);
	}
	if(FogEnable)
		platform::GetRenderBackend().SetFogEnabled(true);
}

void glViewport2(int x,int y,int Width,int Height)
{
   	OpenglWindowX      = x;
   	OpenglWindowY      = y;
   	OpenglWindowWidth  = Width;
   	OpenglWindowHeight = Height;
	platform::GetRenderBackend().SetViewport(x, y, Width, Height, WindowHeight);
}

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

float ConvertX(float x) {

	return (float)(x * g_fScreenRate_x);
}

float ConvertY(float y)
{
	return (float)(y * g_fScreenRate_y);
}

void BeginOpengl(int x,int y,int Width,int Height )
{
	x = x * WindowWidth / GetWindowsX;
	y = y * WindowHeight / GetWindowsY;
	Width = Width * WindowWidth / GetWindowsX;
	Height = Height * WindowHeight / GetWindowsY;

	OpenglWindowX = x;
	OpenglWindowY = y;
	OpenglWindowWidth = Width;
	OpenglWindowHeight = Height;
	platform::GetRenderBackend().PushPerspectiveScene(
		x,
		y,
		Width,
		Height,
		WindowHeight,
		CameraFOV,
		CameraViewNear,
		CameraViewFar * 1.4f,
		CameraPosition,
		CameraAngle,
		CameraTopViewEnable,
		FogEnable,
		FogDensity,
		FogColor);

    AlphaTestEnable = false;
	TextureEnable   = true;
	DepthTestEnable = true;
	CullFaceEnable  = true;
	DepthMaskEnable = true;

    GetOpenGLMatrix(CameraMatrix);
}

void EndOpengl()
{
	platform::GetRenderBackend().PopScene();
}

void BeginModelPreviewViewport(int x, int y, int width, int height, float fov, float near_plane, float far_plane, bool clear_depth_buffer)
{
	static const float kZeroCameraPosition[3] = { 0.f, 0.f, 0.f };
	static const float kZeroCameraAngle[3] = { 0.f, 0.f, 0.f };

	platform::GetRenderBackend().PushPerspectiveScene(
		x,
		y,
		width,
		height,
		WindowHeight,
		fov,
		near_plane,
		far_plane,
		kZeroCameraPosition,
		kZeroCameraAngle,
		false,
		false,
		FogDensity,
		FogColor);

	AlphaTestEnable = false;
	TextureEnable = true;
	DepthTestEnable = true;
	CullFaceEnable = true;
	DepthMaskEnable = true;

	GetOpenGLMatrix(CameraMatrix);
	EnableDepthTest();
	EnableDepthMask();

	if (clear_depth_buffer)
	{
		platform::GetRenderBackend().Clear(GL_DEPTH_BUFFER_BIT);
	}
}

void EndModelPreviewViewport()
{
	platform::GetRenderBackend().PopScene();
}

void UpdateMousePositionn()
{
	vec3_t vPos;

	platform::GetRenderBackend().LoadIdentityModelView();
	platform::GetRenderBackend().TranslateModelView(-CameraPosition[0], -CameraPosition[1], -CameraPosition[2]);
	GetOpenGLMatrix(CameraMatrix);

	Vector(-CameraMatrix[0][3], -CameraMatrix[1][3], -CameraMatrix[2][3], vPos);
	VectorIRotate(vPos, CameraMatrix, MousePosition);
}

#ifdef LDS_ADD_MULTISAMPLEANTIALIASING
BOOL IsGLExtensionSupported(const char *extension)
{
	const size_t extlen = strlen(extension);
	const char *supported = NULL;
	
	// Try To Use wglGetExtensionStringARB On Current DC, If Possible
	PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");
	
	if (wglGetExtString)
		supported = ((char*(__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());
	
	// If That Failed, Try Standard Opengl Extensions String
	if (supported == NULL)
		supported = (char*)glGetString(GL_EXTENSIONS);
	
	// If That Failed Too, Must Be No Extensions Supported
	if (supported == NULL)
		return FALSE;
	
	// Begin Examination At Start Of String, Increment By 1 On False Match
	for (const char* p = supported; ; p++)
	{
		// Advance p Up To The Next Possible Match
		p = strstr(p, extension);
		
		if (p == NULL)
			return FALSE;															// No Match
		
		if ((p==supported || p[-1]==' ') && (p[extlen]=='\0' || p[extlen]==' '))
			return TRUE;															// Match
	}
}

int GetFPSLimit()
{
    return GetDeviceCaps(g_hDC, VREFRESH);
}
BOOL InitGLMultisample(HINSTANCE hInstance,HWND hWnd,PIXELFORMATDESCRIPTOR pfd, int iRequestMSAAValue,int& OutiPixelFormat )
{
	BOOL bIsGLMultisampleSupported = FALSE;

#if defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(_DEBUG)

	// See If The String Exists In WGL!
	if (!IsGLExtensionSupported("WGL_ARB_multisample"))
	{
		bIsGLMultisampleSupported=FALSE;
		return FALSE;
	}
	
#if defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(_DEBUG)
	// Get Our Pixel Format
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");	
	if (!wglChoosePixelFormatARB) 
	{
		bIsGLMultisampleSupported=FALSE;
		return FALSE;
	}

#if defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(_DEBUG)
	
	// Get Our Current Device Context
	HDC hDC = GetDC(hWnd);
	
	int		valid;
	UINT	numFormats;
	float	fAttributes[] = {0,0};
	
	// These Attributes Are The Bits We Want To Test For In Our Sample
	// Everything Is Pretty Standard, The Only One We Want To 
	// Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
	// These Two Are Going To Do The Main Testing For Whether Or Not
	// We Support Multisampling On This Hardware.
	int iAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
			WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB,24,
			WGL_ALPHA_BITS_ARB,8,
			WGL_DEPTH_BITS_ARB,16,
			WGL_STENCIL_BITS_ARB,0,
			WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
			WGL_SAMPLES_ARB, iRequestMSAAValue,					// xN MultiSampling (N=4,2,1)
			0,0
	};
	
#if defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(_DEBUG)

	// First We Check To See If We Can Get A Pixel Format For 4 Samples
	valid = wglChoosePixelFormatARB(hDC,iAttributes,fAttributes,1,&OutiPixelFormat,&numFormats);

#if defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(_DEBUG)
	
	// If We Returned True, And Our Format Count Is Greater Than 1
	if (valid && numFormats >= 1)
	{
		bIsGLMultisampleSupported = TRUE;
		return bIsGLMultisampleSupported;
	}
	
#if defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(_DEBUG)

	// Our Pixel Format With 4 Samples Failed, Test For 2 Samples
	iAttributes[19] = 2;
	valid = wglChoosePixelFormatARB(hDC,iAttributes,fAttributes,1,&OutiPixelFormat,&numFormats);
	if (valid && numFormats >= 1)
	{
		bIsGLMultisampleSupported = TRUE;
		return bIsGLMultisampleSupported;
	}

#if defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(_DEBUG)
	
	// Return The Valid Format
	return  bIsGLMultisampleSupported;
}

void SetEnableMultisample()
{
	if( TRUE == g_bSupportedMSAA )
	{
		SetMultisampleCapability(true);
	}

#if defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(_DEBUG)
}

void SetDisableMultisample()
{
	if( TRUE == g_bSupportedMSAA )
	{
		SetMultisampleCapability(false);
	}

#if defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // defined(_DEBUG)
}

#endif // LDS_ADD_MULTISAMPLEANTIALIASING

///////////////////////////////////////////////////////////////////////////////
// render util
///////////////////////////////////////////////////////////////////////////////

inline void TEXCOORD(float *c,float u,float v)
{
	c[0] = u;
	c[1] = v;
}

void RenderBox(float Matrix[3][4])
{
	vec3_t BoundingBoxMin;
	vec3_t BoundingBoxMax;
	Vector(-10.f,-30.f,-10.f,BoundingBoxMin);
	Vector(10.f,0.f,10.f,BoundingBoxMax);

	vec3_t BoundingVertices[8];
	Vector(BoundingBoxMax[0],BoundingBoxMax[1],BoundingBoxMax[2],BoundingVertices[0]);
	Vector(BoundingBoxMax[0],BoundingBoxMax[1],BoundingBoxMin[2],BoundingVertices[1]);
	Vector(BoundingBoxMax[0],BoundingBoxMin[1],BoundingBoxMax[2],BoundingVertices[2]);
	Vector(BoundingBoxMax[0],BoundingBoxMin[1],BoundingBoxMin[2],BoundingVertices[3]);
	Vector(BoundingBoxMin[0],BoundingBoxMax[1],BoundingBoxMax[2],BoundingVertices[4]);
	Vector(BoundingBoxMin[0],BoundingBoxMax[1],BoundingBoxMin[2],BoundingVertices[5]);
	Vector(BoundingBoxMin[0],BoundingBoxMin[1],BoundingBoxMax[2],BoundingVertices[6]);
	Vector(BoundingBoxMin[0],BoundingBoxMin[1],BoundingBoxMin[2],BoundingVertices[7]);
	
	vec3_t TransformVertices[8];
	for(int j=0;j<8;j++)
	{
   		VectorTransform(BoundingVertices[j],Matrix,TransformVertices[j]);
	}

	static const int kFaceIndices[6][4] =
	{
		{7, 6, 4, 5},
		{0, 2, 3, 1},
		{7, 3, 2, 6},
		{0, 1, 5, 4},
		{7, 5, 1, 3},
		{0, 4, 6, 2},
	};
	static const float kFaceColors[6][4] =
	{
		{0.2f, 0.2f, 0.2f, 1.f},
		{0.2f, 0.2f, 0.2f, 1.f},
		{0.6f, 0.6f, 0.6f, 1.f},
		{0.6f, 0.6f, 0.6f, 1.f},
		{0.4f, 0.4f, 0.4f, 1.f},
		{0.4f, 0.4f, 0.4f, 1.f},
	};
	static const float kFaceUVs[4][2] =
	{
		{1.f, 1.f},
		{1.f, 0.f},
		{0.f, 0.f},
		{0.f, 1.f},
	};

	platform::Vertex3D quad[4];
	for (int face = 0; face < 6; ++face)
	{
		for (int vertex_index = 0; vertex_index < 4; ++vertex_index)
		{
			const int transform_index = kFaceIndices[face][vertex_index];
			SetVertex3D(
				&quad[vertex_index],
				TransformVertices[transform_index],
				kFaceUVs[vertex_index][0],
				kFaceUVs[vertex_index][1],
				kFaceColors[face][0],
				kFaceColors[face][1],
				kFaceColors[face][2],
				kFaceColors[face][3]);
		}

		platform::GetRenderBackend().DrawQuad3D(quad, true, true);
	}
}

void RenderPlane3D(float Width,float Height,float Matrix[3][4])
{
	vec3_t BoundingVertices[4];
	Vector(-Width,-Width, Height,BoundingVertices[3]);
	Vector( Width, Width, Height,BoundingVertices[2]);
	Vector( Width, Width,-Height,BoundingVertices[1]);
	Vector(-Width,-Width,-Height,BoundingVertices[0]);
	
	vec3_t TransformVertices[4];
	for(int j=0;j<4;j++)
	{
   		VectorTransform(BoundingVertices[j],Matrix,TransformVertices[j]);
	}

	platform::Vertex3D quad[4];
	SetVertex3D(&quad[0], TransformVertices[0], 0.f, 1.f, 0.f, 0.f, 0.f, 0.f);
	SetVertex3D(&quad[1], TransformVertices[1], 1.f, 1.f, 0.f, 0.f, 0.f, 0.f);
	SetVertex3D(&quad[2], TransformVertices[2], 1.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	SetVertex3D(&quad[3], TransformVertices[3], 0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
	platform::GetRenderBackend().DrawQuad3D(quad, true, false);
}

void BeginSprite()
{
	platform::GetRenderBackend().PushIdentityModelView();
}

void EndSprite()
{
	platform::GetRenderBackend().PopModelView();
}

void RenderSprite(int Texture,vec3_t Position,float Width,float Height,vec3_t Light,float Rotation,float u,float v,float uWidth,float vHeight)
{
    BindTexture(Texture);

	vec3_t p2;
	VectorTransform(Position,CameraMatrix,p2);
	//VectorCopy(Position,p2);
	float x = p2[0];
	float y = p2[1];
	float z = p2[2];

	Width  *= 0.5f;
	Height *= 0.5f;

	vec3_t p[4];
	if(Rotation==0)
	{
		Vector(x-Width, y-Height, z, p[0]);
		Vector(x+Width, y-Height, z, p[1]);
		Vector(x+Width, y+Height, z, p[2]);
		Vector(x-Width, y+Height, z, p[3]);
	}
	else
	{
		vec3_t p2[4];
		Vector(-Width,-Height, z, p2[0]);
		Vector( Width,-Height, z, p2[1]);
		Vector( Width, Height, z, p2[2]);
		Vector(-Width, Height, z, p2[3]);
		vec3_t Angle;
		Vector(0.f,0.f,Rotation,Angle);
		float Matrix[3][4];
		AngleMatrix(Angle,Matrix);
		for(int i=0;i<4;i++) 
		{
			VectorRotate(p2[i],Matrix,p[i]);
			p[i][0] += x;
			p[i][1] += y;
		}
	}
	
	float c[4][2];
	TEXCOORD(c[3],u       ,v        );
	TEXCOORD(c[2],u+uWidth,v        );
	TEXCOORD(c[1],u+uWidth,v+vHeight);
	TEXCOORD(c[0],u       ,v+vHeight);

	float alpha = 1.f;
	if(Bitmaps[Texture].Components!=3)
	{
		if(Texture != BITMAP_BLOOD+1 && Texture != BITMAP_FONT_HIT)
		{
			alpha = Light[0];
		}
	}

	platform::Vertex3D quad[4];
	for(int i=0;i<4;i++)
	{
		SetVertex3D(&quad[i], p[i], c[i][0], c[i][1], Light[0], Light[1], Light[2], alpha);
	}
	platform::GetRenderBackend().DrawQuad3D(quad, true, true);
}

void RenderSpriteUV(int Texture,vec3_t Position,float Width,float Height,float (*UV)[2],vec3_t Light[4],float Alpha)
{
    BindTexture(Texture);

	vec3_t p2;
	VectorTransform(Position,CameraMatrix,p2);
	float x = p2[0];
	float y = p2[1];
	float z = p2[2];
	
	Width  *= 0.5f;
	Height *= 0.5f;
	vec3_t p[4];
	Vector(x-Width, y-Height, z, p[0]);
	Vector(x+Width, y-Height, z, p[1]);
	Vector(x+Width, y+Height, z, p[2]);
	Vector(x-Width, y+Height, z, p[3]);

	platform::Vertex3D quad[4];
	for(int i=0;i<4;i++)
	{
		SetVertex3D(&quad[i], p[i], UV[i][0], UV[i][1], Light[i][0], Light[i][1], Light[i][2], Alpha);
	}
	platform::GetRenderBackend().DrawQuad3D(quad, true, true);
}

void RenderNumber(vec3_t Position,int Num,vec3_t Color,float Alpha,float Scale)
{
	vec3_t p;
	VectorCopy(Position,p);
	vec3_t Light[4];
	VectorCopy(Color,Light[0]);
	VectorCopy(Color,Light[1]);
	VectorCopy(Color,Light[2]);
	VectorCopy(Color,Light[3]);
    if(Num == -1)
	{
		float UV[4][2];
		TEXCOORD(UV[0],0.f       ,32.f/32.f);
		TEXCOORD(UV[1],32.f/256.f,32.f/32.f);
		TEXCOORD(UV[2],32.f/256.f,17.f/32.f);
		TEXCOORD(UV[3],0.f       ,17.f/32.f);
		RenderSpriteUV(BITMAP_FONT+1,p,45,20,UV,Light,Alpha);
	}
	else if(Num == -2)
	{
		RenderSprite(BITMAP_FONT_HIT,p,32*Scale,20*Scale, Light[0], 0.f, 0.f, 0.f, 27.f/32.f, 15.f/16.f);	
	}
	else
	{
		char Text[32];
		itoa(Num,Text,10);
		p[0] -= strlen(Text)*5.f;
		unsigned int Length = strlen(Text);
		p[0] -= Length*Scale*0.125f;
		p[1] -= Length*Scale*0.125f;
		for(unsigned int i=0;i<Length;i++)
		{
			float UV[4][2];
			float u = (float)(Text[i]-48)*16.f/256.f;
			TEXCOORD(UV[0],u           ,16.f/32.f);
			TEXCOORD(UV[1],u+16.f/256.f,16.f/32.f);
			TEXCOORD(UV[2],u+16.f/256.f,0.f);
			TEXCOORD(UV[3],u           ,0.f);
			RenderSpriteUV(BITMAP_FONT+1,p,Scale,Scale,UV,Light,Alpha);
			p[0] += Scale*0.5f;
			p[1] += Scale*0.5f;
		}
	}
}

float RenderNumber2D(float x,float y,int Num,float Width,float Height)
{
	char Text[32];
	itoa(Num,Text,10);
	int Length = (int)strlen(Text);
	x -= Width*Length/2;
	for(int i=0;i<Length;i++)
	{
		float u = (float)(Text[i]-48)*16.f/256.f;
      	//glColor3fv(Color);
		RenderBitmap(BITMAP_FONT+1,x,y,Width,Height,u,0.f,16.f/256.f,16.f/32.f);
		x += Width*0.7f;
	}
	return x;
}

void BeginBitmap()
{
	platform::GetRenderBackend().PushOrthoScene(WindowWidth, WindowHeight);
	DisableDepthTest();
}

void EndBitmap()
{
	platform::GetRenderBackend().PopScene();
}

void RenderColorRadius(float x, float y, float Width, float Height, float Alpha, int Flag, float radius)
{
	DisableTexture();

	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);
	y = WindowHeight - y;

	float innerWidth = Width - 2 * radius;
	float innerHeight = Height - 2 * radius;
	float red = 1.f;
	float green = 1.f;
	float blue = 1.f;
	float alpha = Alpha > 0.f ? Alpha : 1.f;

	if (Flag == 1)
	{
		red = 0.f;
		green = 0.f;
		blue = 0.f;
	}

	platform::QuadVertex2D quad_vertices[4];
	SetQuadVertex(&quad_vertices[0], x + radius, y - radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[1], x + innerWidth + radius, y - radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[2], x + innerWidth + radius, y - innerHeight - radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[3], x + radius, y - innerHeight - radius, 0.f, 0.f, red, green, blue, alpha);
	platform::GetRenderBackend().DrawQuad2D(quad_vertices, false);

	SetQuadVertex(&quad_vertices[0], x + radius, y, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[1], x + innerWidth + radius, y, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[2], x + innerWidth + radius, y - radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[3], x + radius, y - radius, 0.f, 0.f, red, green, blue, alpha);
	platform::GetRenderBackend().DrawQuad2D(quad_vertices, false);

	SetQuadVertex(&quad_vertices[0], x + radius, y - Height, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[1], x + innerWidth + radius, y - Height, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[2], x + innerWidth + radius, y - Height + radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[3], x + radius, y - Height + radius, 0.f, 0.f, red, green, blue, alpha);
	platform::GetRenderBackend().DrawQuad2D(quad_vertices, false);

	SetQuadVertex(&quad_vertices[0], x, y - radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[1], x + radius, y - radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[2], x + radius, y - Height + radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[3], x, y - Height + radius, 0.f, 0.f, red, green, blue, alpha);
	platform::GetRenderBackend().DrawQuad2D(quad_vertices, false);

	SetQuadVertex(&quad_vertices[0], x + Width, y - radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[1], x + Width - radius, y - radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[2], x + Width - radius, y - Height + radius, 0.f, 0.f, red, green, blue, alpha);
	SetQuadVertex(&quad_vertices[3], x + Width, y - Height + radius, 0.f, 0.f, red, green, blue, alpha);
	platform::GetRenderBackend().DrawQuad2D(quad_vertices, false);

	int num_segments = 20;
	platform::QuadVertex2D fan_vertices[32];

	SetQuadVertex(&fan_vertices[0], x + radius, y - Height + radius, 0.f, 0.f, red, green, blue, alpha);
	for (int i = 0; i <= num_segments; i++) {
		float theta = (M_PI / 2.0f) * (i / (float)num_segments); 
		float cx = radius * cosf(theta); 
		float cy = radius * sinf(theta); 
		SetQuadVertex(&fan_vertices[i + 1], x + radius - cx, y - Height + radius - cy, 0.f, 0.f, red, green, blue, alpha);
	}
	platform::GetRenderBackend().DrawTriangleFan2D(fan_vertices, num_segments + 2, false);

	SetQuadVertex(&fan_vertices[0], x + Width - radius, y - Height + radius, 0.f, 0.f, red, green, blue, alpha);
	for (int i = 0; i <= num_segments; i++) 
	{
		float theta = (M_PI / 2.0f) * (i / (float)num_segments); 
		float cx = radius * cosf(theta); 
		float cy = radius * sinf(theta); 
		SetQuadVertex(&fan_vertices[i + 1], x + Width - radius + cx, y - Height + radius - cy, 0.f, 0.f, red, green, blue, alpha);
	}
	platform::GetRenderBackend().DrawTriangleFan2D(fan_vertices, num_segments + 2, false);

	SetQuadVertex(&fan_vertices[0], x + Width - radius, y - radius, 0.f, 0.f, red, green, blue, alpha);
	for (int i = 0; i <= num_segments; i++) 
	{		
		float theta = (M_PI / 2.0f) * (i / (float)num_segments); 
		float cx = radius * cosf(theta); 
		float cy = radius * sinf(theta); 
		SetQuadVertex(&fan_vertices[i + 1], x + Width - radius + cx, y - radius + cy, 0.f, 0.f, red, green, blue, alpha);
	}
	platform::GetRenderBackend().DrawTriangleFan2D(fan_vertices, num_segments + 2, false);

	SetQuadVertex(&fan_vertices[0], x + radius, y - radius, 0.f, 0.f, red, green, blue, alpha);
	for (int i = 0; i <= num_segments; i++) 
	{
		float theta = (M_PI / 2.0f) * (i / (float)num_segments);  
		float cx = radius * cosf(theta); 
		float cy = radius * sinf(theta); 
		SetQuadVertex(&fan_vertices[i + 1], x + radius - cx, y - radius + cy, 0.f, 0.f, red, green, blue, alpha);
	}
	platform::GetRenderBackend().DrawTriangleFan2D(fan_vertices, num_segments + 2, false);
}

void RenderColor(float x,float y,float Width,float Height,float Alpha,int Flag)
{
    DisableTexture();

	x = ConvertX(x);
	y = ConvertY(y);
	Width  = ConvertX(Width);
	Height = ConvertY(Height);

	float p[4][2];
	y = WindowHeight - y;

	p[0][0] = x      ;p[0][1] = y;
	p[1][0] = x      ;p[1][1] = y-Height;
	p[2][0] = x+Width;p[2][1] = y-Height;
	p[3][0] = x+Width;p[3][1] = y;

	float red = 1.f;
	float green = 1.f;
	float blue = 1.f;
	float alpha = Alpha > 0.f ? Alpha : 1.f;
	if (Flag == 1)
	{
		red = 0.f;
		green = 0.f;
		blue = 0.f;
	}

	platform::QuadVertex2D vertices[4];
	for (int i = 0; i < 4; ++i)
	{
		SetQuadVertex(&vertices[i], p[i][0], p[i][1], 0.f, 0.f, red, green, blue, alpha);
	}

	platform::GetRenderBackend().DrawQuad2D(vertices, false);
}

void EndRenderColor()
{
	platform::GetRenderBackend().SetCurrentColor(1.f, 1.f, 1.f, 1.f);
	platform::GetRenderBackend().SetTextureEnabled(true);
	TextureEnable = true;
}

void RenderColorBitmap(int Texture,float x,float y,float Width,float Height,float u,float v,float uWidth, float vHeight, unsigned int color)
{
	x = ConvertX(x);
	y = ConvertY(y);

	Width = ConvertX(Width);
	Height = ConvertY(Height);

    BindTexture(Texture);

	float p[4][2];

	y = WindowHeight - y;

	p[0][0] = x      ;p[0][1] = y;
	p[1][0] = x      ;p[1][1] = y-Height;
	p[2][0] = x+Width;p[2][1] = y-Height;
	p[3][0] = x+Width;p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0],u       ,v        );
	TEXCOORD(c[3],u+uWidth,v        );
	TEXCOORD(c[2],u+uWidth,v+vHeight);
	TEXCOORD(c[1],u       ,v+vHeight);

	const float red = static_cast<float>((color & 0xff)) / 255.f;
	const float green = static_cast<float>((color >> 8) & 0xff) / 255.f;
	const float blue = static_cast<float>((color >> 16) & 0xff) / 255.f;
	const float alpha = static_cast<float>((color >> 24) & 0xff) / 255.f;

	platform::QuadVertex2D vertices[4];
	for (int i = 0; i < 4; ++i)
	{
		SetQuadVertex(&vertices[i], p[i][0], p[i][1], c[i][0], c[i][1], red, green, blue, alpha);
	}

	platform::GetRenderBackend().DrawQuad2D(vertices, true);
	platform::GetRenderBackend().SetCurrentColor(1.f, 1.f, 1.f, 1.f);
}

void RenderBitmap(int Texture,float x,float y,float Width,float Height,float u,float v,float uWidth,float vHeight,bool Scale,bool StartScale,float Alpha)
{
	if(StartScale)
	{
		x = ConvertX(x);
		y = ConvertY(y);
	}
	if(Scale)
	{
		Width = ConvertX(Width);
		Height = ConvertY(Height);
	}

    BindTexture(Texture);

	float p[4][2];

	y = WindowHeight - y;
	
	p[0][0] = x      ;p[0][1] = y;
	p[1][0] = x      ;p[1][1] = y-Height;
	p[2][0] = x+Width;p[2][1] = y-Height;
	p[3][0] = x+Width;p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0],u       ,v        );
	TEXCOORD(c[3],u+uWidth,v        );
	TEXCOORD(c[2],u+uWidth,v+vHeight);
	TEXCOORD(c[1],u       ,v+vHeight);

	const float alpha = Alpha > 0.f ? Alpha : 1.f;
	platform::QuadVertex2D vertices[4];
	for (int i = 0; i < 4; ++i)
	{
		SetQuadVertex(&vertices[i], p[i][0], p[i][1], c[i][0], c[i][1], 1.f, 1.f, 1.f, alpha);
	}

	platform::GetRenderBackend().DrawQuad2D(vertices, true);
	platform::GetRenderBackend().SetCurrentColor(1.f, 1.f, 1.f, 1.f);
}

void RenderBitmapRotate(int Texture,float x,float y,float Width,float Height,float Rotate,float u,float v,float uWidth,float vHeight)
{
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);
	//x -= Width *0.5f;
	//y -= Height*0.5f;
    BindTexture(Texture);

	vec3_t p[4],p2[4];

	y = WindowHeight - y;

	Vector(-Width*0.5f, Height*0.5f,0.f,p[0]);
	Vector(-Width*0.5f,-Height*0.5f,0.f,p[1]);
	Vector( Width*0.5f,-Height*0.5f,0.f,p[2]);
	Vector( Width*0.5f, Height*0.5f,0.f,p[3]);

	vec3_t Angle;
	Vector(0.f,0.f,Rotate,Angle);
	float Matrix[3][4];
	AngleMatrix(Angle,Matrix);

	float c[4][2];
	TEXCOORD(c[0],u       ,v        );
	TEXCOORD(c[3],u+uWidth,v        );
	TEXCOORD(c[2],u+uWidth,v+vHeight);
	TEXCOORD(c[1],u       ,v+vHeight);

	platform::QuadVertex2D vertices[4];
	for(int i=0;i<4;i++)
	{
     	VectorRotate(p[i],Matrix,p2[i]);
		SetQuadVertex(&vertices[i], p2[i][0]+x, p2[i][1]+y, c[i][0], c[i][1], 1.f, 1.f, 1.f, 1.f);
	}

	platform::GetRenderBackend().DrawQuad2D(vertices, true);
}

void RenderBitRotate(int Texture,float x,float y,float Width,float Height,float Rotate)
{
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);

    BindTexture(Texture);

	vec3_t p[4],p2[4];

	y = Height - y;

	float cx = (Width/2.f) - (Width - x);
	float cy = (Height/2.f) - (Height - y);

	float ax = (-Width*0.5f) + cx;
	float bx = (Width*0.5f) + cx;
	float ay = (-Height*0.5f) + cy;
	float by = (Height*0.5f) + cy;

	Vector(ax, by, 0.f, p[0]);
	Vector(ax, ay, 0.f, p[1]);
	Vector(bx, ay, 0.f, p[2]);
	Vector(bx, by, 0.f, p[3]);

	vec3_t Angle;
	Vector(0.f,0.f,Rotate,Angle);
	float Matrix[3][4];
	AngleMatrix(Angle,Matrix);

	float c[4][2];
	TEXCOORD(c[0],0.f,0.f);
	TEXCOORD(c[3],1.f,0.f);
	TEXCOORD(c[2],1.f,1.f);
	TEXCOORD(c[1],0.f,1.f);

	platform::QuadVertex2D vertices[4];
	for(int i=0;i<4;i++)
	{
		VectorRotate(p[i],Matrix,p2[i]);
		SetQuadVertex(&vertices[i], p2[i][0]+(WindowWidth/2.f), p2[i][1]+(WindowHeight/2.f), c[i][0], c[i][1], 1.f, 1.f, 1.f, 1.f);
	}

	platform::GetRenderBackend().DrawQuad2D(vertices, true);
}

void RenderPointRotate(int Texture,float ix,float iy,float iWidth,float iHeight,float x,float y,float Width,float Height,float Rotate,float Rotate_Loc,float uWidth,float vHeight,int Num)
{
	int i = 0;
	vec3_t p,p2[4],p3,p4[4],Angle;
	float c[4][2],Matrix[3][4];

	ix = ConvertX(ix);
	iy = ConvertY(iy);
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);

    BindTexture(Texture);

	y = Height - y;
	iy = Height - iy;

	Vector((ix - (Width*0.5f)) + ((Width/2.f) - (Width - x)), (iy - (Height*0.5f)) + ((Height/2.f) - (Height - y)), 0.f, p);

	Vector(0.f,0.f,Rotate,Angle);
	AngleMatrix(Angle,Matrix);

	VectorRotate(p,Matrix,p3);

	Vector(-(iWidth*0.5f), (iHeight*0.5f), 0.f, p2[0]);
	Vector(-(iWidth*0.5f), -(iHeight*0.5f), 0.f, p2[1]);
	Vector((iWidth*0.5f), -(iHeight*0.5f), 0.f, p2[2]);
	Vector((iWidth*0.5f), (iHeight*0.5f), 0.f, p2[3]);

	Vector(0.f,0.f,Rotate_Loc,Angle);
	AngleMatrix(Angle,Matrix);

	TEXCOORD(c[0],0.f       ,0.f        );
	TEXCOORD(c[3],uWidth,0.f       );
	TEXCOORD(c[2],uWidth,vHeight);
	TEXCOORD(c[1],0.f       ,vHeight);

	platform::QuadVertex2D vertices[4];
	for(i=0;i<4;i++)
	{
		Matrix[0][3] = p3[0]+25;
		Matrix[1][3] = p3[1];
		VectorTransform(p2[i], Matrix, p4[i]);
		SetQuadVertex(&vertices[i], p4[i][0]+(WindowWidth/2.f), p4[i][1]+(WindowHeight/2.f), c[i][0], c[i][1], 1.f, 1.f, 1.f, 1.f);
	}

	platform::GetRenderBackend().DrawQuad2D(vertices, true);

	if(Num > -1)
	{
		float dx,dy;
		dx = p4[0][0]+(WindowWidth/2.f);
		dy = p4[0][1]+(WindowHeight/2.f);
		dx = dx * GWidescreen.JCWinWidth / (float)(WindowWidth);
		dy = dy * (float)(480.f/WindowHeight);
		if(Num >= 100)
		{
			g_pNewUIMiniMap->SetBtnPos(Num - 100,dx - (iWidth/2) , (480 - dy) - (iHeight/2), iWidth,iHeight);
		}
		else
		{
			g_pNewUIMiniMap->SetBtnPos(Num ,dx, 480 - dy, iWidth/2,iHeight/2);
		}
	}
}

void RenderBitmapLocalRotate(int Texture,float x,float y,float Width,float Height,float Rotate,float u,float v,float uWidth,float vHeight)
{
    BindTexture(Texture);

	vec3_t p[4];
	x = ConvertX(x);
	y = ConvertY(y);
	y = WindowHeight - y;
	Width = ConvertX(Width);
	Height = ConvertY(Height);

	vec3_t vCenter, vDir;
	Vector(x, y, 0, vCenter);
	Vector(Width*0.5f, -Height*0.5f, 0, vDir);
	p[0][0] = vCenter[0] + (vDir[0])*cosf(Rotate);
	p[0][1] = vCenter[1] + (vDir[1])*sinf(Rotate);
	p[1][0] = vCenter[0] + (vDir[0])*sinf(Rotate);
	p[1][1] = vCenter[1] - (vDir[1])*cosf(Rotate);
	p[2][0] = vCenter[0] - (vDir[0])*cosf(Rotate);
	p[2][1] = vCenter[1] - (vDir[1])*sinf(Rotate);
	p[3][0] = vCenter[0] - (vDir[0])*sinf(Rotate);
	p[3][1] = vCenter[1] + (vDir[1])*cosf(Rotate);

	float c[4][2];
	TEXCOORD(c[0],u       ,v        );
	TEXCOORD(c[3],u+uWidth,v        );
	TEXCOORD(c[2],u+uWidth,v+vHeight);
	TEXCOORD(c[1],u       ,v+vHeight);

	platform::QuadVertex2D vertices[4];
	for(int i=0;i<4;i++)
	{
		SetQuadVertex(&vertices[i], p[i][0], p[i][1], c[i][0], c[i][1], 1.f, 1.f, 1.f, 1.f);
	}

	platform::GetRenderBackend().DrawQuad2D(vertices, true);
}

void RenderBitmapAlpha(int Texture,float sx,float sy,float Width,float Height)
{
    EnableAlphaTest();
    BindTexture(Texture);

	sy = WindowHeight - sy;
	for(int y=0;y<4;y++)
	{
		for(int x=0;x<4;x++)
		{
         	float p[4][2];
			p[0][0] = sx+((x  )*Width)*0.25f; p[0][1] = sy-((y  )*Height)*0.25f;
			p[1][0] = sx+((x  )*Width)*0.25f; p[1][1] = sy-((y+1)*Height)*0.25f;
			p[2][0] = sx+((x+1)*Width)*0.25f; p[2][1] = sy-((y+1)*Height)*0.25f;
			p[3][0] = sx+((x+1)*Width)*0.25f; p[3][1] = sy-((y  )*Height)*0.25f;
			
         	float c[4][2];
			TEXCOORD(c[0],(x  )*0.25f,(y  )*0.25f);
			TEXCOORD(c[1],(x  )*0.25f,(y+1)*0.25f);
			TEXCOORD(c[2],(x+1)*0.25f,(y+1)*0.25f);
			TEXCOORD(c[3],(x+1)*0.25f,(y  )*0.25f);

			float Alpha[4] = {1.f,1.f,1.f,1.f};
			if(x==0) {Alpha[0] = 0.f;Alpha[1] = 0.f;}
			if(x==3) {Alpha[2] = 0.f;Alpha[3] = 0.f;}
			if(y==0) {Alpha[0] = 0.f;Alpha[3] = 0.f;}
			if(y==3) {Alpha[1] = 0.f;Alpha[2] = 0.f;}
			/*if(x==0&&y==0) Alpha[0] = 0.f;
			if(x==0&&y==3) Alpha[1] = 0.f;
			if(x==3&&y==3) Alpha[2] = 0.f;
			if(x==3&&y==0) Alpha[3] = 0.f;*/
			
			platform::QuadVertex2D vertices[4];
			for(int i=0;i<4;i++)
			{
				SetQuadVertex(&vertices[i], p[i][0], p[i][1], c[i][0], c[i][1], 1.f, 1.f, 1.f, Alpha[i]);
			}

			platform::GetRenderBackend().DrawQuad2D(vertices, true);
		}
	}

	platform::GetRenderBackend().SetCurrentColor(1.f, 1.f, 1.f, 1.f);
}

void RenderBitmapUV(int Texture,float x,float y,float Width,float Height,float u,float v,float uWidth,float vHeight)
{
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);
    BindTexture(Texture);

	float p[4][2];
	y = WindowHeight - y;
	p[0][0] = x      ;p[0][1] = y;
	p[1][0] = x      ;p[1][1] = y-Height;
	p[2][0] = x+Width;p[2][1] = y-Height;
	p[3][0] = x+Width;p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0],u       ,v        +vHeight*0.25f);
	TEXCOORD(c[3],u+uWidth,v        );
	TEXCOORD(c[2],u+uWidth,v+vHeight);
	TEXCOORD(c[1],u       ,v+vHeight-vHeight*0.25f);

	platform::QuadVertex2D vertices[4];
	for(int i=0;i<4;i++)
	{
		SetQuadVertex(&vertices[i], p[i][0], p[i][1], c[i][0], c[i][1], 1.f, 1.f, 1.f, 1.f);
	}

	platform::GetRenderBackend().DrawQuad2D(vertices, true);
}

///////////////////////////////////////////////////////////////////////////////
// collision detect util
///////////////////////////////////////////////////////////////////////////////

float absf(float a)
{
	if(a < 0.f) return -a;
	return a;
}

float minf(float a,float b)
{
	if(a > b) return b;
	return a;
}

float maxf(float a,float b)
{
	if(a > b) return a;
	return b;
}

int InsideTest(float x,float y,float z,int n,float *v1,float *v2,float *v3,float *v4,int flag,float type)
{
    if(type > 0.f) 
		flag <<= 3;

    int i;
    vec3_t *vtx[4];
	vtx[0] = (vec3_t *)v1;
	vtx[1] = (vec3_t *)v2;
	vtx[2] = (vec3_t *)v3;
	vtx[3] = (vec3_t *)v4;

    int j = n-1;
    switch(flag)
	{
	case 1:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[1]-y) * ((*vtx[j])[2]-z) - ((*vtx[j])[1]-y) * ((*vtx[i])[2]-z);
			if(d <= 0.f) 
				return false;
		}
		break;
	case 2:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[2]-z) * ((*vtx[j])[0]-x) - ((*vtx[j])[2]-z) * ((*vtx[i])[0]-x);
			if(d <= 0.f) 
				return false;
		}
		break;
	case 4:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[0]-x) * ((*vtx[j])[1]-y) - ((*vtx[j])[0]-x) * ((*vtx[i])[1]-y);
			if(d <= 0.f) 
				return false;
		}
		break;
	case 8:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[1]-y) * ((*vtx[j])[2]-z) - ((*vtx[j])[1]-y) * ((*vtx[i])[2]-z);
			if(d >= 0.f) 
				return false;
		}
		break;
	case 16:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[2]-z) * ((*vtx[j])[0]-x) - ((*vtx[j])[2]-z) * ((*vtx[i])[0]-x);
			if(d >= 0.f) 
				return false;
		}
		break;
	case 32:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[0]-x) * ((*vtx[j])[1]-y) - ((*vtx[j])[0]-x) * ((*vtx[i])[1]-y);
			if(d >= 0.f) 
				return false;
		}
		break;
	}
	
    return true;
}

float Distance;

void InitCollisionDetectLineToFace()
{
	Distance = 9999999.f;
}

vec3_t CollisionPosition;

bool CollisionDetectLineToFace(vec3_t Position,vec3_t Target,int Polygon,float *v1,float *v2,float *v3,float *v4,vec3_t Normal,bool Collision)
{
	vec3_t Direction;
	VectorSubtract(Target,Position,Direction);
	float a = DotProduct(Direction,Normal);
	if(a >= 0.f) return false;
	float b = DotProduct(Position,Normal) - DotProduct(v1,Normal);
	float t = -b/a;
	if(t >= 0.f && t <= Distance)
	{
		float X = Direction[0] * t + Position[0];
		float Y = Direction[1] * t + Position[1];
		float Z = Direction[2] * t + Position[2];
		int Count = 0;
		float MIN = minf(minf(absf(Direction[0]),absf(Direction[1])),absf(Direction[2]));
		if(MIN == absf(Direction[0]))
		{
			if( (Y >= minf(Position[1],Target[1]) && Y <= maxf(Position[1],Target[1])) &&
				(Z >= minf(Position[2],Target[2]) && Z <= maxf(Position[2],Target[2])) ) Count++;
		}
		else if(MIN == absf(Direction[1]))
		{
			if( (Z >= minf(Position[2],Target[2]) && Z <= maxf(Position[2],Target[2])) &&
				(X >= minf(Position[0],Target[0]) && X <= maxf(Position[0],Target[0])) ) Count++;
		}
		else
		{
			if( (X >= minf(Position[0],Target[0]) && X <= maxf(Position[0],Target[0])) &&
				(Y >= minf(Position[1],Target[1]) && Y <= maxf(Position[1],Target[1])) ) Count++;
		}
		if(Count == 0) return false;
		Count = 0;
		if(Normal[0]<=-0.5f || Normal[0]>=0.5f)
		{
			Count += InsideTest(X,Y,Z,Polygon,v1,v2,v3,v4,1,Normal[0]);
		}
		else if(Normal[1]<=-0.5f || Normal[1]>=0.5f)
		{
			Count += InsideTest(X,Y,Z,Polygon,v1,v2,v3,v4,2,Normal[1]);
		}
		else
		{
			Count += InsideTest(X,Y,Z,Polygon,v1,v2,v3,v4,4,Normal[2]);
		}
		if(Count == 0) return false;
		if(Collision)
		{
      		Distance = t;
			Vector(X,Y,Z,CollisionPosition);
		}
		return true;
	}
	return false;
}

bool ProjectLineBox(vec3_t ax, vec3_t p1, vec3_t p2, OBB_t obb)
{
	float P1 = DotProduct(ax, p1);
	float P2 = DotProduct(ax, p2);
	
	float mx1 = maxf(P1, P2);
	float mn1 = minf(P1, P2);
	
	float ST = DotProduct(ax, obb.StartPos);
	float Q1 = DotProduct(ax, obb.XAxis);
	float Q2 = DotProduct(ax, obb.YAxis);
	float Q3 = DotProduct(ax, obb.ZAxis);
	
	float mx2 = ST;
	float mn2 = ST;
	
	if (Q1>0)	mx2+=Q1; else mn2+=Q1;
	if (Q2>0)	mx2+=Q2; else mn2+=Q2;
	if (Q3>0) mx2+=Q3; else mn2+=Q3;
	
	if (mn1 > mx2) return false;
	if (mn2 > mx1) return false;
	
	return true;
}

bool CollisionDetectLineToOBB(vec3_t p1, vec3_t p2, OBB_t obb)
{
	vec3_t e1;
	vec3_t eq11,eq12,eq13;

	VectorSubtract(p2,p1,e1);

	CrossProduct( e1, obb.XAxis, eq11);
	CrossProduct( e1, obb.YAxis, eq12);
	CrossProduct( e1, obb.ZAxis, eq13);

	if (!ProjectLineBox(eq11,p1,p2,obb) ) return false;
	if (!ProjectLineBox(eq12,p1,p2,obb) ) return false;
	if (!ProjectLineBox(eq13,p1,p2,obb) ) return false;

	if (!ProjectLineBox(obb.XAxis,p1,p2,obb) ) return false;
	if (!ProjectLineBox(obb.YAxis,p1,p2,obb) ) return false;
	if (!ProjectLineBox(obb.ZAxis,p1,p2,obb) ) return false;

	return true;
}
