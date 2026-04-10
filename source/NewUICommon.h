//////////////////////////////////////////////////////////////////////
// NewUICommon.h: interface for the CNewUICommon class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NEWUICOMMON_H__0668BCBC_7537_454B_82FD_9D6BBBBDBA84__INCLUDED_)
#define AFX_NEWUICOMMON_H__0668BCBC_7537_454B_82FD_9D6BBBBDBA84__INCLUDED_

#pragma once

namespace SEASON3B
{
	bool CreateOkMessageBox(const unicode::t_string& strMsg, DWORD dwColor = 0xffffffff, float fPriority = 3.f);

	int IsPurchaseShop(); 
	#define g_IsPurchaseShop SEASON3B::IsPurchaseShop()

	bool CheckMouseIn(int x, int y, int width, int height);

#ifdef NEW_MUHELPER_ON
	void RenderImage(GLuint uiImageType, float x, float y);
#endif
	void RenderImage(GLuint uiImageType, float x, float y, float width, float height);
	void RenderImageA(GLuint uiImageType, float x, float y, float width, float height);	
	void RenderImageScale(GLuint uiImageType, float x, float y, float width, float height, float su, float sv, float uw, float vh, float alpha = 0.0f);
	void RenderImageScaleAuto(GLuint uiImageType, float x, float y, float width, float height);
	void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv);
	void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv, DWORD color);
	void RenderImage(GLuint uiImageType, float x, float y, float width, float height, float su, float sv,float uw, float vh, DWORD color = RGBA(255,255,255,255));
	
	float RenderNumber(float x, float y, int iNum, float fScale = 1.0f);
	
	bool IsNone(int iVirtKey);
	bool IsRelease(int iVirtKey);
	bool IsPress(int iVirtKey);
	bool IsRepeat(int iVirtKey);
	
	class CNewKeyInput
	{
		struct INPUTSTATEINFO 
		{
			BYTE byKeyState;
		} m_pInputInfo[256];

#ifndef ASG_FIX_ACTIVATE_APP_INPUT
		void Init();
#endif

	public:
		enum KEY_STATE 
		{ 
			KEY_NONE=0,			
			KEY_RELEASE,		
			KEY_PRESS,			
			KEY_REPEAT			
		};
		~CNewKeyInput();
		
		static CNewKeyInput* GetInstance();
#ifdef ASG_FIX_ACTIVATE_APP_INPUT
		void Init();
#endif	
		void ScanAsyncKeyState();

		bool IsNone(int iVirtKey);
		bool IsRelease(int iVirtKey);
		bool IsPress(int iVirtKey); 
		bool IsRepeat(int iVirtKey);
		void SetKeyState(int iVirtKey, KEY_STATE KeyState);
		

	protected:
		CNewKeyInput();
	};
#ifdef NEW_MUHELPER_ON
	class CCheckBox
	{
	public:
		CCheckBox() {
			Initialize();
		}
		~CCheckBox() {
			Destroy();
		}
	private:
		HFONT m_hTextFont;
		int m_ImgIndex;
		WORD output_width;
		WORD output_height;
		POINT m_Pos;
		SIZE m_Size;
		unicode::t_string m_Name;
		bool m_bEnable;
	public:
		void Initialize();
		void Destroy();
		void RegisterImgIndex(int imgindex);
		void ChangeCheckBoxInfo(int x, int y, int sx, int sy);
		void ChangeCheckBoxText(unicode::t_string btname);;
		bool Create(int imgindex, int x, int y, int sx, int sy, unicode::t_string btname);
		bool Render();
		bool UpdateMouseEvent();
		void SetPos(int x, int y);
		void SetSize(int width, int height);
		bool IsSelected() {
			return m_bEnable;
		};
		void SetSelected(bool enable) {
			m_bEnable = enable != false;
		}
	};

	class stMacroUIImage
	{
	public:
		stMacroUIImage();
		~stMacroUIImage();
	public:
		POINT m_Pos;
		float output_width;
		float output_height;
		int m_ImgIndex;
	public:
		void Register(int imgindex, int x, int y, int width, int height);
		void Render() {
			RenderImage(m_ImgIndex, m_Pos.x, m_Pos.y, output_width, output_height);
		};
	};

	class stMacroUIText
	{
	public:
		stMacroUIText();
		~stMacroUIText();
	public:
		POINT m_Pos;
		unicode::t_string m_Name;
	public:
		void Register(int x, int y, const char* pzText);
		void Render();
	};


	class CSlideBar
	{
	public:
		CSlideBar();
		virtual ~CSlideBar();
	private:
		POINT m_Pos;
		__int16 m_iValue;
		__int16 m_MaxLength;
		__int16 m_MinLength;
		int m_Imgindex;
		int m_ImgBack;
		__int16 m_Width;
		__int16 m_Height;
	public:
		void Create(int imgback, int imgindex, int x, int y, __int16 sx, __int16 sy, __int16 iMaxLength, __int16 start);
		bool MouseUpdate();
		void Render();
		int GetSlideLevel();
		void SetSlideLevel(__int16 Value);
	};


	class COptionButtonGroup
	{
	public:
		COptionButtonGroup();
		virtual ~COptionButtonGroup();
	private:
		std::vector<CCheckBox*> m_box;
		int m_imgindex;
		int m_index;
		int m_x;
		int m_y;
		int m_w;
		int m_h;
	public:
		void Create(int imgindex, int x, int y, int w, int h, unsigned char count);
		void SetOptionText(unsigned char index, int offset_x, int offset_y, const char* text);
		bool UpdateMouseEvent();
		bool Render();
		void SetIndex(int index);
		int GetIndex();
	};
#endif
}

#define g_pNewKeyInput	SEASON3B::CNewKeyInput::GetInstance()

#endif // !defined(AFX_NEWUICOMMON_H__0668BCBC_7537_454B_82FD_9D6BBBBDBA84__INCLUDED_)
