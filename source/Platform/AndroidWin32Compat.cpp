#include "Platform/AndroidWin32Compat.h"

#if defined(__ANDROID__)

#include "Platform/GameAssetPath.h"

#include <android/log.h>

#include <algorithm>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
	const char* kAndroidWin32CompatLogTag = "MUAndroidCompat";

	std::string g_legacy_client_app_data_root;
	POINT g_legacy_client_cursor_position = { 0, 0 };
	unsigned int g_legacy_client_double_click_time_ms = 500;

	std::string NormalizePath(const char* path)
	{
		std::string normalized = path != NULL ? path : "";
		for (size_t index = 0; index < normalized.size(); ++index)
		{
			if (normalized[index] == '\\')
			{
				normalized[index] = '/';
			}
		}
		return normalized;
	}

	bool FileExists(const std::string& path)
	{
		struct stat status = {};
		return !path.empty() && stat(path.c_str(), &status) == 0;
	}

	bool IsAbsolutePath(const std::string& path)
	{
		return !path.empty() && path[0] == '/';
	}

	std::string JoinPath(const std::string& left, const std::string& right)
	{
		if (left.empty())
		{
			return right;
		}
		if (right.empty())
		{
			return left;
		}
		if (left[left.size() - 1] == '/')
		{
			return left + right;
		}
		return left + "/" + right;
	}

	std::string Trim(const std::string& value)
	{
		size_t begin = 0;
		while (begin < value.size() && (value[begin] == ' ' || value[begin] == '\t' || value[begin] == '\r' || value[begin] == '\n'))
		{
			++begin;
		}

		size_t end = value.size();
		while (end > begin && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r' || value[end - 1] == '\n'))
		{
			--end;
		}

		return value.substr(begin, end - begin);
	}

	std::string ToLowerAscii(const std::string& value)
	{
		std::string lowered = value;
		for (size_t index = 0; index < lowered.size(); ++index)
		{
			if (lowered[index] >= 'A' && lowered[index] <= 'Z')
			{
				lowered[index] = static_cast<char>(lowered[index] - 'A' + 'a');
			}
		}
		return lowered;
	}

	std::string ResolveIniPath(const char* file_name)
	{
		const std::string normalized = NormalizePath(file_name);
		if (normalized.empty())
		{
			return normalized;
		}

		if (IsAbsolutePath(normalized) && FileExists(normalized))
		{
			return normalized;
		}

		if (FileExists(normalized))
		{
			return normalized;
		}

		std::string relative = normalized;
		if (relative.size() >= 2 && relative[0] == '.' && relative[1] == '/')
		{
			relative = relative.substr(2);
		}

		if (!g_legacy_client_app_data_root.empty())
		{
			const std::string from_app_data = JoinPath(g_legacy_client_app_data_root, relative);
			if (FileExists(from_app_data))
			{
				return from_app_data;
			}
		}

		if (ToLowerAscii(relative) == "config.ini" || ToLowerAscii(relative) == "client_runtime.ini")
		{
			if (!g_legacy_client_app_data_root.empty())
			{
				return JoinPath(g_legacy_client_app_data_root, relative);
			}
		}

		if (relative.size() >= 4 && ToLowerAscii(relative.substr(0, 4)) == "data")
		{
			const std::string resolved_asset_path = platform::ResolveGameAssetPath(relative.c_str());
			if (FileExists(resolved_asset_path))
			{
				return resolved_asset_path;
			}
			return resolved_asset_path;
		}

		return normalized;
	}

	DWORD CopyResultString(const std::string& value, const char* default_value, char* returned_string, DWORD returned_string_size)
	{
		if (returned_string == NULL || returned_string_size == 0)
		{
			return 0;
		}

		const std::string chosen_value = !value.empty() ? value : (default_value != NULL ? default_value : "");
		const size_t copy_length = std::min(static_cast<size_t>(returned_string_size - 1), chosen_value.size());
		memcpy(returned_string, chosen_value.c_str(), copy_length);
		returned_string[copy_length] = '\0';
		return static_cast<DWORD>(copy_length);
	}
}

char* itoa(int value, char* buffer, int radix)
{
	if (buffer == NULL)
	{
		return NULL;
	}

	if (radix == 16)
	{
		snprintf(buffer, 34, "%x", value);
	}
	else
	{
		snprintf(buffer, 34, "%d", value);
	}
	return buffer;
}

int wsprintfA(char* buffer, const char* format, ...)
{
	if (buffer == NULL || format == NULL)
	{
		return 0;
	}

	va_list args;
	va_start(args, format);
	const int written = vsprintf(buffer, format, args);
	va_end(args);
	return written;
}

int wsprintfW(wchar_t* buffer, const wchar_t* format, ...)
{
	if (buffer == NULL || format == NULL)
	{
		return 0;
	}

	va_list args;
	va_start(args, format);
	const int written = vswprintf(buffer, 1024, format, args);
	va_end(args);
	return written;
}

DWORD GetTickCount(void)
{
	const std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
	const uint64_t elapsed_ms = static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count());
	return static_cast<DWORD>(elapsed_ms & 0xFFFFFFFFu);
}

void Sleep(DWORD milliseconds)
{
	usleep(static_cast<useconds_t>(milliseconds) * 1000);
}

DWORD GetDoubleClickTime(void)
{
	return g_legacy_client_double_click_time_ms;
}

BOOL GetCursorPos(LPPOINT point)
{
	if (point == NULL)
	{
		return FALSE;
	}

	*point = g_legacy_client_cursor_position;
	return TRUE;
}

BOOL ScreenToClient(HWND, LPPOINT)
{
	return TRUE;
}

HWND GetActiveWindow(void)
{
	return reinterpret_cast<HWND>(1);
}

HWND GetFocus(void)
{
	return GetActiveWindow();
}

void* CreateWindowW(const wchar_t*, const wchar_t*, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID)
{
	return NULL;
}

BOOL SetWindowPos(HWND, HWND, int, int, int, int, UINT)
{
	return TRUE;
}

LONG_PTR GetWindowLongW(HWND, int)
{
	return 0;
}

LONG_PTR SetWindowLongW(HWND, int, LONG_PTR)
{
	return 0;
}

int MessageBoxA(HWND, const char* text, const char* caption, unsigned int)
{
	__android_log_print(
		ANDROID_LOG_WARN,
		kAndroidWin32CompatLogTag,
		"MessageBoxA: %s :: %s",
		caption != NULL ? caption : "(null)",
		text != NULL ? text : "(null)");
	return IDOK;
}

int MessageBoxW(HWND, const wchar_t* text, const wchar_t* caption, unsigned int)
{
	char text_utf8[1024] = { 0 };
	char caption_utf8[256] = { 0 };
	if (text != NULL)
	{
		wcstombs(text_utf8, text, sizeof(text_utf8) - 1);
	}
	if (caption != NULL)
	{
		wcstombs(caption_utf8, caption, sizeof(caption_utf8) - 1);
	}

	return MessageBoxA(NULL, text_utf8, caption_utf8, MB_OK);
}

HINSTANCE ShellExecuteA(HWND, const char* operation, const char* file, const char* parameters, const char* directory, int show_command)
{
	__android_log_print(
		ANDROID_LOG_INFO,
		kAndroidWin32CompatLogTag,
		"ShellExecuteA op=%s file=%s params=%s dir=%s show=%d",
		operation != NULL ? operation : "(null)",
		file != NULL ? file : "(null)",
		parameters != NULL ? parameters : "(null)",
		directory != NULL ? directory : "(null)",
		show_command);
	return reinterpret_cast<HINSTANCE>(33);
}

void ExitProcess(UINT exit_code)
{
	__android_log_print(
		ANDROID_LOG_ERROR,
		kAndroidWin32CompatLogTag,
		"ExitProcess(%u)",
		static_cast<unsigned int>(exit_code));
	std::exit(static_cast<int>(exit_code));
}

BOOL PostMessage(HWND, UINT message, WPARAM w_param, LPARAM l_param)
{
	__android_log_print(
		ANDROID_LOG_INFO,
		kAndroidWin32CompatLogTag,
		"PostMessage message=0x%x wparam=%lld lparam=%lld",
		static_cast<unsigned int>(message),
		static_cast<long long>(w_param),
		static_cast<long long>(l_param));
	return TRUE;
}

LRESULT SendMessage(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
	PostMessage(window, message, w_param, l_param);
	return 0;
}

DWORD GetFileAttributesA(const char* path)
{
	if (path == NULL || path[0] == '\0')
	{
		return INVALID_FILE_ATTRIBUTES;
	}

	struct stat status = {};
	if (stat(path, &status) != 0)
	{
		return INVALID_FILE_ATTRIBUTES;
	}

	DWORD attributes = 0;
	if (S_ISDIR(status.st_mode))
	{
		attributes |= FILE_ATTRIBUTE_DIRECTORY;
	}
	return attributes;
}

DWORD GetCurrentDirectoryA(DWORD buffer_length, char* buffer)
{
	if (buffer == NULL || buffer_length == 0)
	{
		return 0;
	}

	if (getcwd(buffer, static_cast<size_t>(buffer_length)) == NULL)
	{
		buffer[0] = '\0';
		return 0;
	}

	return static_cast<DWORD>(strlen(buffer));
}

DWORD GetModuleFileNameA(HMODULE, char* buffer, DWORD buffer_length)
{
	if (buffer == NULL || buffer_length == 0)
	{
		return 0;
	}

	const ssize_t size = readlink("/proc/self/exe", buffer, static_cast<size_t>(buffer_length - 1));
	if (size <= 0)
	{
		buffer[0] = '\0';
		return 0;
	}

	buffer[size] = '\0';
	return static_cast<DWORD>(size);
}

DWORD GetPrivateProfileStringA(const char* section, const char* key, const char* default_value, char* returned_string, DWORD returned_string_size, const char* file_name)
{
	const std::string resolved_path = ResolveIniPath(file_name);
	std::ifstream file(resolved_path.c_str(), std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		return CopyResultString(std::string(), default_value, returned_string, returned_string_size);
	}

	const std::string target_section = ToLowerAscii(section != NULL ? section : "");
	const std::string target_key = ToLowerAscii(key != NULL ? key : "");
	std::string current_section;
	std::string line;
	while (std::getline(file, line))
	{
		line = Trim(line);
		if (line.empty() || line[0] == ';' || line[0] == '#')
		{
			continue;
		}

		if (line.size() >= 2 && line[0] == '[' && line[line.size() - 1] == ']')
		{
			current_section = ToLowerAscii(Trim(line.substr(1, line.size() - 2)));
			continue;
		}

		const size_t equals = line.find('=');
		if (equals == std::string::npos)
		{
			continue;
		}

		const std::string current_key = ToLowerAscii(Trim(line.substr(0, equals)));
		if (current_section == target_section && current_key == target_key)
		{
			return CopyResultString(Trim(line.substr(equals + 1)), default_value, returned_string, returned_string_size);
		}
	}

	return CopyResultString(std::string(), default_value, returned_string, returned_string_size);
}

BOOL GetTextExtentPointA(HDC, LPCSTR text, int length, LPSIZE size)
{
	if (size == NULL)
	{
		return FALSE;
	}

	const int safe_length = length >= 0 ? length : (text != NULL ? static_cast<int>(strlen(text)) : 0);
	size->cx = safe_length * 8;
	size->cy = 16;
	return TRUE;
}

BOOL GetTextExtentPoint32A(HDC hdc, LPCSTR text, int length, LPSIZE size)
{
	return GetTextExtentPointA(hdc, text, length, size);
}

BOOL TextOutA(HDC, int, int, LPCSTR, int)
{
	return TRUE;
}

HFONT CreateFontA(int, int, int, int, int, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, const char*)
{
	return reinterpret_cast<HFONT>(1);
}

BOOL DeleteObject(HGDIOBJ)
{
	return TRUE;
}

int WSAGetLastError(void)
{
	return errno;
}

void _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext)
{
	if (drive != NULL)
	{
		drive[0] = '\0';
	}
	if (dir != NULL)
	{
		dir[0] = '\0';
	}
	if (fname != NULL)
	{
		fname[0] = '\0';
	}
	if (ext != NULL)
	{
		ext[0] = '\0';
	}

	if (path == NULL || path[0] == '\0')
	{
		return;
	}

	const std::string normalized = NormalizePath(path);
	const size_t last_slash = normalized.find_last_of('/');
	const size_t last_dot = normalized.find_last_of('.');
	const bool dot_is_extension = last_dot != std::string::npos &&
		(last_slash == std::string::npos || last_dot > last_slash);

	if (dir != NULL && last_slash != std::string::npos)
	{
		const std::string directory = normalized.substr(0, last_slash + 1);
		strncpy(dir, directory.c_str(), _MAX_DIR - 1);
		dir[_MAX_DIR - 1] = '\0';
	}

	const size_t file_begin = (last_slash == std::string::npos) ? 0 : last_slash + 1;
	const size_t file_length = dot_is_extension ? (last_dot - file_begin) : std::string::npos;
	if (fname != NULL)
	{
		const std::string file_name = normalized.substr(file_begin, file_length);
		strncpy(fname, file_name.c_str(), _MAX_FNAME - 1);
		fname[_MAX_FNAME - 1] = '\0';
	}

	if (ext != NULL && dot_is_extension)
	{
		const std::string extension = normalized.substr(last_dot);
		strncpy(ext, extension.c_str(), _MAX_EXT - 1);
		ext[_MAX_EXT - 1] = '\0';
	}
}

namespace platform
{
	void SetLegacyClientAppDataRoot(const char* absolute_root_path)
	{
		g_legacy_client_app_data_root = absolute_root_path != NULL ? NormalizePath(absolute_root_path) : "";
	}

	const char* GetLegacyClientAppDataRoot()
	{
		return g_legacy_client_app_data_root.c_str();
	}

	void SetLegacyClientCursorPosition(long x, long y)
	{
		g_legacy_client_cursor_position.x = x;
		g_legacy_client_cursor_position.y = y;
	}

	void SetLegacyClientDoubleClickTimeMs(unsigned int milliseconds)
	{
		g_legacy_client_double_click_time_ms = milliseconds > 0 ? milliseconds : 500;
	}
}

#endif
