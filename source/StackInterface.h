// NewUIMixInventory.h: interface for the CChaosGenesis class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "NewUIBase.h"
#include "NewUIInventoryCtrl.h"
#include "NewUIMessageBox.h"
#include "NewUIMyInventory.h"
#include "UIControls.h"
#include "ProtocolSend.h"
#include "NProtocol.h"

namespace SEASON3B
{
	class CStackInterface : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{
			IMAGE_GENESISMIXINVENTORY_BACK_VOID = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,
			IMAGE_GENESISMIXINVENTORY_MIXBTN = BITMAP_INTERFACE_NEW_MIXINVENTORY_BEGIN,
		};

	private:
		enum
		{
			INVENTORY_WIDTH = 190,
			INVENTORY_HEIGHT = 429,
		};

		CNewUIManager* m_pNewUIMng;


	public:
		POINT m_Pos;
		enum EVENT_STATE
		{
			EVENT_NONE = 0,
			EVENT_SCROLL_BTN_DOWN,
			EVENT_SCROLL_BTN_CLIKED,
		};

	public:
		CStackInterface();
		virtual ~CStackInterface();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();
		void SetPos(int x, int y);
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		bool Update();
		bool Render();
		void RenderFrame();
		void RenderText();
		float GetLayerDepth();

	private:
		void LoadImages();
		void UnloadImages();
		void ResetMouseLButton();
	};
}

