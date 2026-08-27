#include <windows.h>
#include "SharedProtocol.h"

#define SERVICE_NAME L"FileManagerService"

SERVICE_STATUS        g_ServiceStatus = {};
SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
HANDLE                g_StopEvent = nullptr;

VOID WINAPI ServiceMain(DWORD argc, LPWSTR* argv);
DWORD WINAPI ServiceCtrlHandler(DWORD ctrl, DWORD eventType, LPVOID eventData, LPVOID context);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);

int wmain(int argc, wchar_t* argv[])
{
    SERVICE_TABLE_ENTRY serviceTable[] =
    {
        { (LPWSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION)ServiceMain },
        { nullptr, nullptr }
    };

    // Đăng ký với Service Control Manager (SCM).
    // Nếu chạy KHÔNG phải do SCM khởi động (ví dụ double-click thủ công),
    // hàm này sẽ trả FALSE ngay lập tức - ta dùng đặc điểm này để debug (xem Lab 8).
    if (!StartServiceCtrlDispatcher(serviceTable))
    {
        DWORD err = GetLastError();
        // ERROR_FAILED_SERVICE_CONTROLLER_CONNECT nghĩa là đang chạy tay,
        // không phải do SCM start.
        return err;
    }
    return 0;
}

VOID WINAPI ServiceMain(DWORD argc, LPWSTR* argv)
{
    g_StatusHandle = RegisterServiceCtrlHandlerEx(SERVICE_NAME, ServiceCtrlHandler, nullptr);
    if (!g_StatusHandle) return;

    ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
    g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
    g_ServiceStatus.dwWin32ExitCode = NO_ERROR;
    g_ServiceStatus.dwCheckPoint = 0; // như kiểu mã số báo cáo của lần cập nhật gần nhất của service đến SCM sau mỗi .dwWaitHint
    g_ServiceStatus.dwWaitHint = 3000;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    g_StopEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

    HANDLE hThread = CreateThread(nullptr, 0, ServiceWorkerThread, nullptr, 0, nullptr);

    g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);

    // Chờ tới khi có lệnh Stop
    WaitForSingleObject(hThread, INFINITE);

    g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
}

DWORD WINAPI ServiceCtrlHandler(DWORD ctrl, DWORD eventType, LPVOID eventData, LPVOID context)
{
    switch (ctrl)
    {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
        SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
        SetEvent(g_StopEvent); // báo hiệu cho worker thread dừng vòng lặp
        return NO_ERROR;
    default:
        return NO_ERROR;
    }
}