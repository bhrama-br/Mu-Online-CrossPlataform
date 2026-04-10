#pragma once

BoostSmartPointer(DARK_SPIRIT_HELPER_VIEW);
class DARK_SPIRIT_HELPER_VIEW
{
public:
	static DARK_SPIRIT_HELPER_VIEWPtr Make();

	DARK_SPIRIT_HELPER_VIEW() : m_pOwner(nullptr), ModelIndex(-1) 
	{
		this->Init();
	};

	virtual ~DARK_SPIRIT_HELPER_VIEW() 
	{
		this->Relese();
	};

	void Init() 
	{

	};

	bool Create(CHARACTER* m_Owner, int ModelIndex);
	bool IsSameOwner(CHARACTER*Owner) { return (Owner == m_pOwner) ? 1 : 0; };

	void Relese() 
	{
		ModelIndex = -1;

		if (m_pOwner) 
		{
			m_pOwner = nullptr;
		}
	}

	int ModelIndex;
	CHARACTER* m_pOwner;
};