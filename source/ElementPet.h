#pragma once

#include <vector>
#include "HelperView.h"

class CElementPet {
public:
	CElementPet();
	~CElementPet();

	void Relese();

	bool CreateHelper(int Index, int ModelIndex, vec3_t Position, CHARACTER* m_Owner, int SubType = 0);
	void DeleteHelper(CHARACTER*Owner, int itemType = -1, bool allDelete = false);
	void Register(BoostSmart_Ptr(HELPER_VIEW) info);
	void UnRegister(CHARACTER*Owner, int itemType, bool isUnregistAll = false);
	bool CheckExistsHelper(CHARACTER* Owner);

	void RenderElement();
	void UpdateElement();

private:
	std::vector<BoostSmart_Ptr(HELPER_VIEW)> m_ElementPet;
};

extern CElementPet gElementPetFirst;
extern CElementPet gElementPetSecond;