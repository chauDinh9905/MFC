#pragma once
#include <map>
#include <string>
#include <afxstr.h>
using namespace std;
enum class UiKey {
	CreateButton,
	DeleteButton,
	OpenDirTooltip,
	EditPathHint
};

class UiStrings {
public:
	static CString Get(UiKey key, const CString& lang) {
		static map<UiKey, map<CString, CString>> table = {
			{UiKey::CreateButton, {{_T("vi"), _T("Tao file")}, {_T("en"), _T("Create file")}}},
			{UiKey::DeleteButton, {{_T("vi"), _T("Xoa file")}, {_T("en"), _T("Delete file")}}},
			{UiKey::OpenDirTooltip, {{_T("vi"), _T("Mo thu muc win")}, {_T("en"), _T("Open win derectory")}}},
			{UiKey::EditPathHint, {{_T("vi"), _T("Nhap duong dan file")}, {_T("en"), _T("Enter file path")}}},
		};
		return table[key][lang];
	}
};