#include <windows.h>
#include "SharedProtocol.h"
#include <shellapi.h>  // cho ShellExecuteEx
#include <stdio.h>
#include <string>
using namespace std;
#define SERVICE_NAME L"FileManagerService"

SERVICE_STATUS        g_ServiceStatus = {};
SERVICE_STATUS_HANDLE g_StatusHandle = nullptr;
HANDLE                g_StopEvent = nullptr;

VOID WINAPI ServiceMain(DWORD argc, LPWSTR* argv);
DWORD WINAPI ServiceCtrlHandler(DWORD ctrl, DWORD eventType, LPVOID eventData, LPVOID context);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);


void InstallService()
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH); // lấy đường dẫn chính file .exe đang chạy

    SC_HANDLE hSCManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager)
    {
        wprintf(L"Loi OpenSCManager: %lu (co the ban chua chay quyen Admin)\n", GetLastError());
        return;
    }

    SC_HANDLE hService = CreateServiceW(
        hSCManager,
        SERVICE_NAME,                   // tên nội bộ service
        L"File Manager Service",        // tên hiển thị trong services.msc
        SERVICE_ALL_ACCESS,
        SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START,              // tương đương "start= auto"
        SERVICE_ERROR_NORMAL,
        exePath,                         // binPath - chính là file .exe đang chạy
        nullptr, nullptr, nullptr, nullptr, nullptr);

    if (!hService)
    {
        DWORD err = GetLastError();
        if (err == ERROR_SERVICE_EXISTS)
            wprintf(L"Service da ton tai san.\n");
        else
            wprintf(L"Loi CreateService: %lu\n", err);
        CloseServiceHandle(hSCManager);
        return;
    }

    wprintf(L"Cai dat service thanh cong.\n");

    // Tự khởi động luôn sau khi cài, không cần gõ thêm lệnh start
    StartServiceW(hService, 0, nullptr);

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

void UninstallService()
{
    SC_HANDLE hSCManager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (!hSCManager) { wprintf(L"Loi OpenSCManager: %lu\n", GetLastError()); return; }

    SC_HANDLE hService = OpenServiceW(hSCManager, SERVICE_NAME, SERVICE_STOP | DELETE);
    if (!hService)
    {
        wprintf(L"Loi OpenService: %lu (co the service chua duoc cai)\n", GetLastError());
        CloseServiceHandle(hSCManager);
        return;
    }

    SERVICE_STATUS status;
    ControlService(hService, SERVICE_CONTROL_STOP, &status); // dừng trước khi xoá, bỏ qua lỗi nếu đang không chạy

    if (DeleteService(hService))
        wprintf(L"Go cai dat service thanh cong.\n");
    else
        wprintf(L"Loi DeleteService: %lu\n", GetLastError());

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);
}

bool IsRunningAsAdmin()
{
    BOOL isAdmin = FALSE;
    PSID adminGroup = nullptr;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;

    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup))
    {
        CheckTokenMembership(nullptr, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin;
}

void RelaunchAsAdmin(const wchar_t* args)
{
    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = L"runas";   // yêu cầu Windows tự hiện UAC prompt xin quyền Admin
    sei.lpFile = exePath;
    sei.lpParameters = args;
    sei.nShow = SW_NORMAL;

    if (!ShellExecuteExW(&sei))
    {
        wprintf(L"Nguoi dung tu choi cap quyen Admin, huy cai dat.\n");
    }
}

int wmain(int argc, wchar_t* argv[])
{
    if (argc > 1)
    {
        wstring arg = argv[1];

        if (arg == L"-install")
        {
            if (!IsRunningAsAdmin())
            {
                RelaunchAsAdmin(L"-install"); // tự mở lại chính nó dưới quyền Admin
                return 0;
            }
            InstallService();
            return 0;
        }
        else if (arg == L"-uninstall")
        {
            if (!IsRunningAsAdmin())
            {
                RelaunchAsAdmin(L"-uninstall");
                return 0;
            }
            UninstallService();
            return 0;
        }
    }

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