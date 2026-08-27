#include <windows.h>
#include <shlobj.h>   // SHGetKnownFolderPath
#include <string>
#include <algorithm>
#include "SharedProtocol.h"
#include "MessageProvider.h" // Lab 6

static bool GetKnownFolder(REFKNOWNFOLDERID id, std::wstring& outPath)
{
    PWSTR raw = nullptr;
    if (SHGetKnownFolderPath(id, 0, nullptr, &raw) != S_OK) return false;
    outPath = raw;
    CoTaskMemFree(raw);
    return true;
}

static bool IsPathInsideWhitelist(const std::wstring& fullPath)
{
    std::wstring roots[3];
    GetKnownFolder(FOLDERID_Windows, roots[0]);
    GetKnownFolder(FOLDERID_ProgramFiles, roots[1]);
    GetKnownFolder(FOLDERID_ProgramFilesX86, roots[2]);

    auto toLower = [](std::wstring s) {
        std::transform(s.begin(), s.end(), s.begin(), ::towlower);
        return s;
        };

    std::wstring fullLower = toLower(fullPath);

    for (auto& root : roots)
    {
        std::wstring rootLower = toLower(root);
        if (fullLower.rfind(rootLower, 0) == 0) // fullLower bắt đầu bằng rootLower
        {
            // Đảm bảo đúng ranh giới thư mục (không phải "prefix giả",
            // ví dụ "C:\Program FilesEvil" không được tính là con của "C:\Program Files")
            if (fullLower.size() == rootLower.size() || fullLower[rootLower.size()] == L'\\')
                return true;
        }
    }
    return false;
}

FileResponse HandleRequest(const FileRequest& req)
{
    FileResponse resp = {};
    MessageProvider msg(req.lang);

    wchar_t fullPath[MAX_PATH] = {};
    DWORD len = GetFullPathNameW(req.path, MAX_PATH, fullPath, nullptr);

    if (len == 0 || len >= MAX_PATH)
    {
        resp.success = FALSE;
        wcscpy_s(resp.message, msg.Get(MsgKey::InvalidPath));
        return resp;
    }

    if (!IsPathInsideWhitelist(fullPath))
    {
        resp.success = FALSE;
        wcscpy_s(resp.message, msg.Get(MsgKey::PathNotAllowed));
        return resp;
    }

    switch (req.action)
    {
    case FileAction::Create:
    {
        if (GetFileAttributesW(fullPath) != INVALID_FILE_ATTRIBUTES)
        {
            resp.success = FALSE;
            wcscpy_s(resp.message, msg.Get(MsgKey::FileAlreadyExists));
            return resp;
        }

        HANDLE hFile = CreateFileW(fullPath, GENERIC_WRITE, 0, nullptr,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            resp.success = FALSE;
            if (GetLastError() == ERROR_ACCESS_DENIED)
                wcscpy_s(resp.message, msg.Get(MsgKey::AccessDenied));
            else
                wcscpy_s(resp.message, msg.Get(MsgKey::UnexpectedError));
            return resp;
        }
        CloseHandle(hFile);

        resp.success = TRUE;
        wcscpy_s(resp.message, msg.Get(MsgKey::CreateSuccess));
        return resp;
    }
    case FileAction::Delete:
    {
        if (GetFileAttributesW(fullPath) == INVALID_FILE_ATTRIBUTES)
        {
            resp.success = FALSE;
            wcscpy_s(resp.message, msg.Get(MsgKey::FileNotFound));
            return resp;
        }

        if (!DeleteFileW(fullPath))
        {
            resp.success = FALSE;
            if (GetLastError() == ERROR_ACCESS_DENIED)
                wcscpy_s(resp.message, msg.Get(MsgKey::AccessDenied));
            else
                wcscpy_s(resp.message, msg.Get(MsgKey::UnexpectedError));
            return resp;
        }

        resp.success = TRUE;
        wcscpy_s(resp.message, msg.Get(MsgKey::DeleteSuccess));
        return resp;
    }
    default:
        resp.success = FALSE;
        wcscpy_s(resp.message, msg.Get(MsgKey::UnknownAction));
        return resp;
    }
}