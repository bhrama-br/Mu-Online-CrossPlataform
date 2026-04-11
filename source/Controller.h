#pragma once
// ----------------------------------------------------------------------------------------------

#if defined(__ANDROID__)
#ifndef HHOOK
typedef void* HHOOK;
#endif
#endif

class Controller
{
public:
	bool	Load();
	// ----
	static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
	// ----
	HINSTANCE	Instance;
private:
	HHOOK		MouseHook;
	HHOOK		KeyboardHook;
	// ----
}; extern Controller gController;
// ----------------------------------------------------------------------------------------------