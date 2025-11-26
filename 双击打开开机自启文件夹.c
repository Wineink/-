#include <windows.h>
#include <shlobj.h>
#include <stdio.h>

// 链接所需库
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")

// 关键：Windows GUI程序的入口函数是 WinMain（而非 main），避免控制台窗口
int WINAPI WinMain(
    HINSTANCE hInstance,  // 应用程序实例句柄
    HINSTANCE hPrevInstance,  // 前一个实例句柄（Win32中已废弃，恒为NULL）
    LPSTR lpCmdLine,  // 命令行参数
    int nCmdShow      // 窗口显示方式
) {
    char startupPath[MAX_PATH] = {0};

    // 获取当前用户的开机自启文件夹路径
    HRESULT hr = SHGetFolderPathA(
        NULL,
        CSIDL_STARTUP,
        NULL,
        SHGFP_TYPE_CURRENT,
        startupPath
    );

    if (FAILED(hr)) {
        MessageBoxA(NULL, "获取开机自启文件夹失败！", "错误", MB_ICONERROR);
        return 1;
    }

    // 打开文件夹
    HINSTANCE hInst = ShellExecuteA(
        NULL,
        "open",
        startupPath,
        NULL,
        NULL,
        SW_SHOWNORMAL
    );

    if ((UINT)hInst <= 32) {
        char errMsg[256] = {0};
        sprintf(errMsg, "打开文件夹失败！错误码：%u", (UINT)hInst);
        MessageBoxA(NULL, errMsg, "错误", MB_ICONERROR);
        return 1;
    }

    return 0;
}