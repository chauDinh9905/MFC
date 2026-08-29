#include "pch.h"
#include "LanguageSettings.h"
#include <windows.h>

static const wchar_t* REG_PATH = L"Software\\FileManagerClientApp";
static const wchar_t* REG_VALUE = L"Language";

void LanguageSettings::Save(const CString& lang) {
	HKEY hKey;
	LONG result = RegCreateKeyExW(HKEY_CURRENT_USER, REG_PATH, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
	if (result == ERROR_SUCCESS) {
		CStringW value(lang);
		RegSetValueExW(hKey, REG_VALUE, 0, REG_SZ, (const BYTE*)(LPCWSTR)value, (value.GetLength() + 1) * sizeof(wchar_t));
		RegCloseKey(hKey);
	}
}

CString LanguageSettings::Load() {
	HKEY hKey;
	CString result = _T("vi");
	if (RegOpenKeyExW(HKEY_CURRENT_USER, REG_PATH, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
		wchar_t buffer[8] = {};
		DWORD bufSize = sizeof(buffer);
		DWORD type = 0;
		if (RegQueryValueExW(hKey, REG_VALUE, nullptr, &type, (LPBYTE)buffer, &bufSize) == ERROR_SUCCESS && type == REG_SZ) {
			result = buffer;
		}
		RegCloseKey(hKey);
	}
	return result;
}