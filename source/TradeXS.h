#pragma once
#include "NewUIBase.h"
#include "NewUIManager.h"

namespace SEASON3B
{
	class CTradeX : public CNewUIObj
	{
	public:
		enum IMAGE_LIST
		{

			IMAGE_BASE_WINDOW_BACK = CNewUIMessageBoxMng::IMAGE_MSGBOX_BACK,				//. newui_msgbox_back.jpg
			IMAGE_BASE_WINDOW_TOP = CNewUIMyInventory::IMAGE_INVENTORY_BACK_TOP,			//. newui_item_back01.tga	(190,64)
			IMAGE_BASE_WINDOW_LEFT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_LEFT,		//. newui_item_back02-l.tga	(21,320)
			IMAGE_BASE_WINDOW_RIGHT = CNewUIMyInventory::IMAGE_INVENTORY_BACK_RIGHT,		//. newui_item_back02-r.tga	(21,320)
			IMAGE_BASE_WINDOW_BOTTOM = CNewUIMyInventory::IMAGE_INVENTORY_BACK_BOTTOM,	//. newui_item_back03.tga	(190,45)
			IMAGE_BASE_WINDOW_BTN_EXIT = CNewUIMyInventory::IMAGE_INVENTORY_EXIT_BTN,		//. newui_exit_00.tga
			//--
			IMAGE_TABLE_TOP_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_LEFT,			//. newui_item_table01(L).tga (14,14)
			IMAGE_TABLE_TOP_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_RIGHT,			//. newui_item_table01(R).tga (14,14)
			IMAGE_TABLE_BOTTOM_LEFT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_LEFT,		//. newui_item_table02(L).tga (14,14)
			IMAGE_TABLE_BOTTOM_RIGHT = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_RIGHT,	//. newui_item_table02(R).tga (14,14)
			IMAGE_TABLE_TOP_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_TOP_PIXEL,			//. newui_item_table03(up).tga (1, 14)
			IMAGE_TABLE_BOTTOM_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_BOTTOM_PIXEL,	//. newui_item_table03(dw).tga (1,14)
			IMAGE_TABLE_LEFT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_LEFT_PIXEL,		//. newui_item_table03(L).tga (14,1)
			IMAGE_TABLE_RIGHT_PIXEL = CNewUIInventoryCtrl::IMAGE_ITEM_TABLE_RIGHT_PIXEL,		//. newui_item_table03(R).tga (14,1)
			//--
			IMAGE_WINDOW_TAB_BTN = CNewUIGuildInfoWindow::IMAGE_GUILDINFO_TAB_BUTTON,

		};
	private:
		enum PARTY_WINDOW_SIZE
		{
			WINDOW_WIDTH = 190,
			WINDOW_HEIGHT = 429,
		};
	private:
		CNewUIManager* m_pNewUIMng;
		POINT m_Pos;
	public:
		CTradeX();
		virtual ~CTradeX();

		bool Create(CNewUIManager* pNewUIMng, int x, int y);
		void Release();

		bool Render();
		bool Update();
		bool UpdateMouseEvent();
		bool UpdateKeyEvent();
		float GetLayerDepth();
		float GetKeyEventOrder();
		void SetPos(int x, int y);

	private:
		void LoadImages();
		void UnloadImages();
	};

}