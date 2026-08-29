#pragma once
#include <afxstr.h>

class LanguageSettings {
public:
	static void Save(const CString& lang);
	static CString Load();
};