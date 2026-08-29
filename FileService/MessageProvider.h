#pragma once
#include <map>
#include <string>
#include <windows.h>
using namespace std;

enum class MsgKey {
	CreateSuccess, DeleteSuccess, FileAlreadyExists, FileNotFound,
	AccessDenied, InvalidPath, PathNotAllowed, UnexpectedError, UnknownAction
};

class MessageProvider {
public:
	explicit MessageProvider(const wchar_t* lang) : m_lang(lang) {}
	const wchar_t* Get(MsgKey key) {
		map<MsgKey, map<wstring, const wchar_t* >>table = {
			{MsgKey::CreateSuccess, {{L"vi", L"Tao file thanh cong"}, {L"en", L"File created successfully"}}},
			{MsgKey::DeleteSuccess, {{L"vi", L"Xoa file thanh cong"}, {L"en", L"File deleted successfully"}}},
			{MsgKey::FileAlreadyExists, {{L"vi", L"File da ton tai"}, {L"en", L"File already exist"}}},
			{MsgKey::FileNotFound, {{L"vi", L"Khong tim thay file"}, {L"en", L"File not found"}}},
			{MsgKey::AccessDenied, {{L"vi", L"Khong co quyen truy cap"}, {L"en", L"Access denied"}}},
			{MsgKey::InvalidPath, {{L"vi", L"Duong dan khong hop le"}, {L"en", L"Invalid path"}}},
			{MsgKey::PathNotAllowed, {{L"vi", L"Duong dan phai nam trong C:\\Windows, Program files"}, {L"en", L"Path not allowed"}}},
			{MsgKey::UnexpectedError, {{L"vi", L"Loi khong xac dinh"}, {L"en", L"Unexpected error"}}},
			{MsgKey::UnknownAction, {{L"vi", L"Hanh vi khong hop le"}, {L"en", L"Invalid action"}}},
		};
		wstring langKey = (m_lang == L"vi") ? L"vi" : L"en";
		return table[key][langKey];
	}
private:
	wstring m_lang;
};