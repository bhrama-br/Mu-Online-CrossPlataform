#pragma once

#include "Protocol.h"
#include "TradeX.h"
#include <vector>

struct PMSG_TRADE_RESPONSE_RECV
{
	PBMSG_HEAD header; // C1:37
	BYTE response;
	char name[10];
	WORD level;
	DWORD GuildNumber;
	BYTE TradeX;
};

struct PMSG_TRADEX_ITEM_ADD_RECV
{
	PSBMSG_HEAD header; // C1:B1:15
	BYTE slot;
	BYTE ItemInfo[12];
};

struct PMSG_TRADEX_STATE_RECV
{
	PSBMSG_HEAD header; // C1:0xFB:0x10
	BYTE state;
	BYTE Transation;
};

struct PMSG_TRADEX_ITEM_LIST_RECV
{
	PSWMSG_HEAD header; // C2:0xFB:0x11
	BYTE count;
	BYTE target;
};

struct PMSG_TRADEX_ITEM_DEL_RECV
{
	PSBMSG_HEAD header; // C1:0xFB:0x13
	BYTE slot;
};

struct PMSG_TRADEX_ITEM_RECV
{
	BYTE slot;
	BYTE ItemInfo[12];
};

struct PMSG_TRADEX_OPEN_WINDOW_SEND
{
	PSBMSG_HEAD header; // C2:0xFB:0x12
};

struct PMSG_TRADEX_HIDDEN_WINDOW_RECV
{
	PSBMSG_HEAD header; // C2:0xFB:0x14
	BYTE state;
};

struct PMSG_TRADE_OK_BUTTON_RECV
{
	PBMSG_HEAD header; // C1:3C
	BYTE flag;
	BYTE tradeX;
};

struct PMSG_TRADE_RESULT_RECV
{
	PBMSG_HEAD header; // C1:[3A:3D]
	BYTE result;
	BYTE tradeX;
};

struct PMSG_TRADE_OK_BUTTON_SEND
{
	PBMSG_HEAD header; // C1:3C
	BYTE flag;
	BYTE tradeX;
	BYTE Cancel;
};

struct PMSG_TRADE_COST_RECV
{
	PSBMSG_HEAD header; // C2:0xFB:0x14
	BYTE type;
	DWORD Coin;
};

class CTradeX {
public:
	enum EVENT_STATE
	{
		EVENT_NONE = 0,
		EVENT_SCROLL_BTN_DOWN,
		EVENT_SCROLL_BTN_CLIKED,
	};


	class TradeItem {
	public:
		TradeItem() {

		};

		~TradeItem() {

		};

		void Clear()
		{
			this->slot = -1;
			if (this->pItemObj)
			{
				NewDeleteItemTradeX(this->pItemObj);
				this->pItemObj = nullptr;
			}
		}

		ITEM* pItemObj;
		BYTE slot;
	};

public:
	CTradeX();
	~CTradeX();

	bool isVisible();
	bool isHidden() { return this->TradeXHidden; };
	bool CheckMouse();
	void Open();
	void Close();
	void ClearTradeX();
	void Render();
	void RenderFrame();
	void RenderText();
	void RenderButtom();
	void Update();
	void UpdateKeyEvent();
	void UpdateMouseEvent();
	void UpdateButtom();
	void RenderButtomHidden();
	void RenderTextSource();
	void RenderTextTarget();
	void RenderBoxItem(float x, float y, float width, float height);
	void RenderItensSource();
	void RenderItensTarget();
	void RenderItemInformation(float x, float y, ITEM* pItemObj);
	void RenderItemImage(ITEM* pItemObj);
	void TargetInserItens(int slot, BYTE* ItemInfo);
	void SourceInserItens(int slot, BYTE* ItemInfo);
	void DelItemTarget(BYTE slot);
	void DelItemSource(BYTE slot);
	void CalcLinesRenderTarget();
	void CalcLinesRenderSource();
	void RenderScrollTarget(float x, float y, float width, float height);
	void RenderScrollSource(float x, float y, float width, float height);
	void ScrollLinesTarget(int mode);
	void ScrollLinesSource(int mode);
	void MouseWheel(MOUSEHOOKSTRUCTEX* lpMouse);
	void ClearTradeXSource();
	void ClearTradeXTarget();
	int CountTimeTradeX(DWORD tick);
	void TradeXHiddenWindow();
	void RenderIconTradeExtended();
	void UpdateMouseEventTradeExtended();
	void TradeXOpenWindow();
	void SetItemAutoMove(bool ItemAutoMove);
	bool IsItemAutoMove() { return m_bItemAutoMove; }
	static void NewDeleteItemTradeX(ITEM* item);

	void GCTradeResponseRecv(PMSG_TRADE_RESPONSE_RECV* lpMsg);
	void TradeXItemAddRecv(PMSG_TRADEX_ITEM_ADD_RECV* lpMsg);
	void TradeXStateRecv(PMSG_TRADEX_STATE_RECV* lpMsg);
	void TradeXListRecv(PMSG_TRADEX_ITEM_LIST_RECV* lpMsg);
	void TradeXDelItemTarget(PMSG_TRADEX_ITEM_DEL_RECV* lpMsg);
	void TradeXOkButtonRecv(PMSG_TRADE_OK_BUTTON_RECV* lpMsg);
	void TradeXResultRecv(PMSG_TRADE_RESULT_RECV* lpMsg);

	bool CheckTradeXSourceEmptySlot();
	void MoveItemFromInventory();
	bool MoveItemToInventory();
	void ProcessToReceiveTradeXItemsSource(int nIndex, BYTE* pbyItemPacket);
	void ProcessToRemoveTradeXItemsSource(int nIndex, BYTE* pbyItemPacket);

public:
	int TradeXIconID;

	int ItemPointer;
	int ItemPointerTarget;

private:
	int m_PosX;
	int m_PosY;
	int m_nBackupMouseX;
	int m_nBackupMouseY;
	bool m_bItemAutoMove;

	bool m_isVisible;
	bool TradeXHidden;
	int TradeXTransation;

	bool m_TargetAcceptTradeX;
	bool m_SourceAcceptTradeX;

	float ScrollBarPosXTarget;
	float ScrollBarPosXSource;

	float TargetScrollBarOffset;
	float TargetScrollBarMaxOffset;
	float TargetScrollBarPos;
	float TargetScrollBarEsp;
	float TargetScrollBarMultiplier;

	float SourceScrollBarOffset;
	float SourceScrollBarMaxOffset;
	float SourceScrollBarPos;
	float SourceScrollBarEsp;
	float SourceScrollBarMultiplier;

	int m_iMouseYSource;
	int m_iMouseYTarget;

	EVENT_STATE m_EventStateTarget;
	EVENT_STATE m_EventStateSource;

	char NameTarget[11];

	DWORD TimeTradeX;
	DWORD ClickEvent;
	DWORD TickBlockOk;

	int m_TargetSlotBackup;

private:
	std::vector<TradeItem> m_TradeItemSource;
	std::vector<TradeItem> m_TradeItemTarget;
};

extern CTradeX gTradeX;