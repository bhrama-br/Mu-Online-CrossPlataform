#ifndef _IEXPLORER_H_
#define _IEXPLORER_H_

#pragma once


namespace leaf {
	/* Open URL for default web-browser    */
	/* This function return process handle */
	inline bool OpenExplorer(const std::string& url)
	{
#if defined(__ANDROID__)
		// TODO: Android intent to open URL
		(void)url;
		return false;
#else
		// SW_SHOW
		if(32 < (UINT)ShellExecute(NULL, "open", url.c_str(), NULL, NULL, SW_NORMAL))
			return true;
		return false;
#endif
	}
}


#endif // _IEXPLORER_H_