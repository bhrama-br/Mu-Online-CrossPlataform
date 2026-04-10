// LoadData.h: interface for the CLoadData class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CLoadData
{
public:
	CLoadData();
	virtual ~CLoadData();
	void AccessModel(int Type, const char* Dir, const char* FileName,int i=-1);
	void OpenTexture(int Model, const char* SubFolder, int Wrap=GL_REPEAT, int Type=GL_NEAREST,bool Check=true);

	public:
};

extern CLoadData gLoadData;
