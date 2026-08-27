#include "pch.h"
#include "PipeClient.h"

bool PipeClient::SendRequest(const FileRequest& request, FileResponse& outResponse, CString& errorMsg)
{
    // Chờ tối đa 3 giây nếu tất cả instance pipe đang bận (server đang xử lý client khác)
    if (!WaitNamedPipeW(PIPE_NAME, 3000))
    {
        errorMsg = _T("Service chưa chạy hoặc không phản hồi (WaitNamedPipe timeout).");
        return false;
    }

    HANDLE hPipe = CreateFileW(
        PIPE_NAME,
        GENERIC_READ | GENERIC_WRITE,
        0,                      // không share
        nullptr,                // security attributes mặc định cho client
        OPEN_EXISTING,
        0,
        nullptr);

    if (hPipe == INVALID_HANDLE_VALUE)
    {
        errorMsg.Format(_T("Không thể kết nối Service (lỗi %lu)."), GetLastError());
        return false;
    }

    DWORD mode = PIPE_READMODE_MESSAGE;
    SetNamedPipeHandleState(hPipe, &mode, nullptr, nullptr);

    DWORD bytesWritten = 0;
    BOOL writeOk = WriteFile(hPipe, &request, sizeof(request), &bytesWritten, nullptr);

    DWORD bytesRead = 0;
    BOOL readOk = FALSE;
    if (writeOk)
        readOk = ReadFile(hPipe, &outResponse, sizeof(outResponse), &bytesRead, nullptr);

    CloseHandle(hPipe);

    if (!writeOk || !readOk)
    {
        errorMsg.Format(_T("Lỗi giao tiếp với Service (lỗi %lu)."), GetLastError());
        return false;
    }

    return true;
}