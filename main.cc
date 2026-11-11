// clang-format off
#include "raw.hh"

// 窗口过程：处理 WM_INPUT 消息
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int main(int argc, char **argv)
{
    SetConsoleOutputCP(CP_UTF8);

    try {
        HINSTANCE hInstance = GetModuleHandleW(nullptr);

        // 声明顺序 = 析构逆序，保证：~RawKeyboardInput → ~MessageOnlyWindow → ~WindowClass
        WindowClass        wc(L"RawInputEndKeyMonitor", hInstance, WndProc);
        MessageOnlyWindow  hwnd(wc.name(), L"RawInputHiddenWindow", hInstance);
        RawKeyboardInput   keyboard(hwnd.get());

        MSG msg = {};
        while (GetMessageW(&msg, nullptr, 0, 0)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }

    return 0;
}