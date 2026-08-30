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

static bool EnsureParentDirectoryExists(const wstring& filePath, DWORD& outError) {
    wstring dir = filePath.substr(0, filePath.find_last_of(L'\\'));
    int result = SHCreateDirectoryExW(nullptr, dir.c_str(), nullptr);
    if (result == ERROR_SUCCESS || result == ERROR_ALREADY_EXISTS) return true;
    outError = (DWORD)result;
    return false;
}

static bool ValidatePathFormat(const wchar_t* rawPath, wstring& fullPath, DWORD& errorCode, MsgKey& errKey) {
    if (rawPath == nullptr || wcslen(rawPath) == 0) {
        errKey = MsgKey::InvalidPath;
        errorCode = ERROR_INVALID_PARAMETER;
        return false;
    }
    wchar_t buffer[MAX_PATH] = {};
    DWORD len = GetFullPathNameW(rawPath, MAX_PATH, buffer, nullptr);
    if (len == 0 || len >= MAX_PATH) {
        errKey = MsgKey::InvalidPath;
        errorCode = GetLastError();
        return false;
    }
    fullPath = buffer;
    if (fullPath.back() == L'\\'){
        errKey = MsgKey::InvalidPath;
        errorCode = ERROR_INVALID_NAME;
        return false;
    }
    return true;
}

FileResponse HandleRequest(const FileRequest& req)
{
    FileResponse resp = {};
    MessageProvider msg(req.lang);

    wstring fullPath;
    DWORD errorCode = 0;
    MsgKey errKey;

    if (!ValidatePathFormat(req.path, fullPath, errorCode, errKey)) {
        resp.success = FALSE;
        resp.errorCode = errorCode;
        swprintf_s(resp.message, L"%s (Ma loi: %lu)", msg.Get(errKey), errorCode);
        return resp;
    }
    if (!IsPathInsideWhitelist(fullPath))
    {
        resp.errorCode = errorCode;
        resp.success = FALSE;
        swprintf_s(resp.message, L"%s (Ma loi: %lu)", msg.Get(errKey), errorCode);
        return resp;
    }

    switch (req.action)
    {
    case FileAction::Create:
    {
        if (GetFileAttributesW(fullPath.c_str()) != INVALID_FILE_ATTRIBUTES)
        {
            resp.success = FALSE;
            resp.errorCode = ERROR_ALREADY_EXISTS;
            wcscpy_s(resp.message, msg.Get(MsgKey::FileAlreadyExists));
            return resp;
        }
        DWORD dirErr = 0;
        if (!EnsureParentDirectoryExists(fullPath, dirErr)) {
            resp.success = FALSE;
            resp.errorCode = dirErr;
            swprintf_s(resp.message, L"%s (Ma loi: %lu)", msg.Get(MsgKey::UnexpectedError), dirErr);
            return resp;
        }

        HANDLE hFile = CreateFileW(fullPath.c_str(), GENERIC_WRITE, 0, nullptr,
            CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hFile == INVALID_HANDLE_VALUE)
        {
            DWORD err = GetLastError();
            resp.success = FALSE;
            resp.errorCode = err;
            if (err == ERROR_ACCESS_DENIED)
                wcscpy_s(resp.message, msg.Get(MsgKey::AccessDenied));
            else
                wcscpy_s(resp.message, msg.Get(MsgKey::UnexpectedError));
            return resp;
        }
        CloseHandle(hFile);

        resp.success = TRUE;
        resp.errorCode = 0;
        wcscpy_s(resp.message, msg.Get(MsgKey::CreateSuccess));
        return resp;
    }
    case FileAction::Delete:
    {
        if (GetFileAttributesW(fullPath.c_str()) == INVALID_FILE_ATTRIBUTES)
        {
            DWORD err = GetLastError();
            resp.success = FALSE;
            resp.errorCode = err;
            swprintf_s(resp.message, L"%s (Ma loi: %lu)", msg.Get(MsgKey::FileNotFound), err);
            return resp;
        }

        if (!DeleteFileW(fullPath.c_str()))
        {
            DWORD err = GetLastError();
            resp.success = FALSE;
            resp.errorCode = err;
            if (GetLastError() == ERROR_ACCESS_DENIED)
                wcscpy_s(resp.message, msg.Get(MsgKey::AccessDenied));
            else
                wcscpy_s(resp.message, msg.Get(MsgKey::UnexpectedError));
            return resp;
        }

        resp.success = TRUE;
        resp.errorCode = 0;
        wcscpy_s(resp.message, msg.Get(MsgKey::DeleteSuccess));
        return resp;
    }
    default:
        resp.success = FALSE;
        wcscpy_s(resp.message, msg.Get(MsgKey::UnknownAction));
        return resp;
    }
}