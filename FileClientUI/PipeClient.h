#pragma once
#include "SharedProtocol.h"
#include <afxwin.h>

class PipeClient
{
public:
    // Trả true nếu gửi/nhận thành công; outResponse chứa kết quả từ Service.
    static bool SendRequest(const FileRequest& request, FileResponse& outResponse, CString& errorMsg);
};