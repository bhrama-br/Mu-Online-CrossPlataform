#pragma once

#if defined(__ANDROID__)

class CUITextInputBox;

namespace platform
{
	void ClearLegacyAndroidTextInputFocus();
	bool IsLegacyAndroidTextInputFocused(const CUITextInputBox* input_box);
	bool AppendLegacyAndroidTextInputChar(CUITextInputBox* input_box, char value);
	bool BackspaceLegacyAndroidTextInput(CUITextInputBox* input_box);
}

#endif
