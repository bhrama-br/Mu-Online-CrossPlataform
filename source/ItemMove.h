#pragma once

#define GetChangeUp(x) (((x>>4)&0x01)?3:(((x>>3)&0x01)?2:1))

class cItemMove
{
public:
	cItemMove();

	virtual ~cItemMove();

	void cItemMove::ResetMouseRButton();

	void RightClickMoveItem();

	void RightClickDeleteItem(BYTE result);

	BYTE GetInventoryWearSlot(int ItemIndex);

	bool CheckCanMove();

	int GetFlag();

public:
	bool ItemMove;

	int SourceFlag;

	int SourceSlot;

	int TargetSlot;
};

extern cItemMove gItemMove;