#pragma once
#include <windows.h>

#pragma pack(push, 1)

enum class FileAction : int
{
    Create = 0,
    Delete = 1
};

struct FileRequest
{
    FileAction action;
    wchar_t    path[260];   // MAX_PATH
    wchar_t    lang[8];     // "vi" hoặc "en"
};

struct FileResponse
{
    BOOL    success;
    DWORD errorCode;
    wchar_t message[512];
};

#pragma pack(pop)

// Tên pipe dùng chung cho cả server và client
inline const wchar_t* PIPE_NAME = L"\\\\.\\pipe\\FileManagerServicePipe";