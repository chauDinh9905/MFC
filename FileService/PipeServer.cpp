#include <windows.h>
#include <sddl.h>
#include "SharedProtocol.h"

extern HANDLE g_StopEvent; // dùng từ ServiceMain.cpp

FileResponse HandleRequest(const FileRequest& req); // hiện thực ở Lab 4

HANDLE CreateSecuredPipe()
{
    // SDDL Security Descriptor Definition Language: cho phép nhóm "Authenticated Users" (AU) Generic All (GA) trên pipe này.
    // Nếu không có dòng này, mặc định pipe do Local System tạo chỉ Administrators/SYSTEM
    // mới connect được -> UI chạy quyền thường sẽ bị ERROR_ACCESS_DENIED khi CreateFile.
    PSECURITY_DESCRIPTOR pSD = nullptr;
    ConvertStringSecurityDescriptorToSecurityDescriptorW(
        L"D:(A;;GA;;;AU)",
        SDDL_REVISION_1,
        &pSD,// pSD chứa địa chỉ vùng nhớ mà kết quả của cái chuối bên trên ra dạng nhị phân để hệ điều hành hiểu được
        nullptr);

    SECURITY_ATTRIBUTES sa = {};
    sa.nLength = sizeof(sa);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = FALSE;

    HANDLE hPipe = CreateNamedPipeW(
        PIPE_NAME,
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        sizeof(FileResponse),   // out buffer size
        sizeof(FileRequest),    // in buffer size
        0,                      // default timeout
        &sa);

    LocalFree(pSD);
    return hPipe;
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
    while (true)
    {
        HANDLE hPipe = CreateSecuredPipe();
        if (hPipe == INVALID_HANDLE_VALUE)
        {
            Sleep(1000);
            continue;
        }

        // ConnectNamedPipe block cho tới khi có client kết nối.
        // Để có thể dừng service sạch sẽ, ta cần cách kiểm tra g_StopEvent song song
        // (đơn giản nhất cho bài học: kiểm tra event NGAY SAU khi 1 client vừa xử lý xong).
        BOOL connected = ConnectNamedPipe(hPipe, nullptr)
            ? TRUE
            : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (connected)
        {
            FileRequest req = {};
            DWORD bytesRead = 0;

            if (ReadFile(hPipe, &req, sizeof(req), &bytesRead, nullptr) && bytesRead == sizeof(req))
            {
                FileResponse resp = HandleRequest(req);
                DWORD bytesWritten = 0;
                WriteFile(hPipe, &resp, sizeof(resp), &bytesWritten, nullptr);
                FlushFileBuffers(hPipe);
            }
        }

        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);

        if (WaitForSingleObject(g_StopEvent, 0) == WAIT_OBJECT_0)
            break; // có lệnh Stop -> thoát vòng lặp, kết thúc thread
    }
    return 0;
}