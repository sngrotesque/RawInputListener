// clang-format off
#include "raw.hh"

namespace {
    // 便捷提取：如果 raw 是键盘事件，返回对应 RAWKEYBOARD 指针，否则 nullptr
    inline const RAWKEYBOARD *as_keyboard(const RAWINPUT &raw) noexcept
    {
        return raw.header.dwType == RIM_TYPEKEYBOARD ? &raw.data.keyboard : nullptr;
    }

    std::runtime_error error_message(DWORD code, const std::string &message)
    {
        return std::runtime_error(std::format("[{}] {}", code, message));
    }
}

// RawInputBuffer
RawInputBuffer::RawInputBuffer(HRAWINPUT hRawInput) noexcept
{
    // WndProc 不能抛出异常
    UINT size = 0;

    // 第一次调用：查询缓冲区大小
    UINT code = GetRawInputData(
        hRawInput, RID_INPUT, nullptr, &size, sizeof(RAWINPUTHEADER)
    );
    if ((code != 0) || (size == 0)) {
        return; // 错误
    }

    m_buffer.resize(size);

    // 第二次调用：填充数据
    UINT copied = GetRawInputData(
        hRawInput, RID_INPUT, m_buffer.data(), &size, sizeof(RAWINPUTHEADER)
    );
    if (copied != size) {
        // 填充数据失败，释放缓冲区
        m_buffer.shrink_to_fit();
        return;
    }
    m_valid = true;
}

bool RawInputBuffer::valid() const noexcept
{
    return m_valid;
}

const RAWINPUT *RawInputBuffer::get() const noexcept
{
    return reinterpret_cast<const RAWINPUT *>(m_buffer.data());
}

const RAWINPUT *RawInputBuffer::operator->() const noexcept
{
    return get();
}

// WindowClass
WindowClass::WindowClass(LPCWSTR name, HINSTANCE hInstance, WNDPROC proc)
    : m_name(name), m_instance(hInstance)
{
    WNDCLASSEXW wc = {
        .cbSize        = sizeof(WNDCLASSEXW),
        .lpfnWndProc   = proc,
        .hInstance     = hInstance,
        .lpszClassName = name,
    };
    if (!RegisterClassExW(&wc)) {
        throw error_message(GetLastError(), "RegisterClassEx failed.");
    }
}

WindowClass::~WindowClass()
{
    if (m_name) {
        UnregisterClassW(m_name, m_instance);
    }
}

LPCWSTR WindowClass::name() const noexcept
{
    return m_name;
}

// MessageOnlyWindow
MessageOnlyWindow::MessageOnlyWindow(LPCWSTR className, LPCWSTR title, HINSTANCE hInstance)
{
    m_hwnd = CreateWindowExW(
        /*dwExStyle   */ 0,
        /*lpClassName */ className,
        /*lpWindowName*/ title,
        /*dwStyle     */ 0,
        /*X, Y, W, H  */ 0, 0, 0, 0,
        /*hWndParent  */ HWND_MESSAGE,   // 仅消息窗口
        /*hMenu       */ nullptr,
        /*hInstance   */ hInstance,
        /*lpParam     */ nullptr
    );
    if (!m_hwnd) {
        throw error_message(GetLastError(), "CreateWindowEx failed.");
    }
}

MessageOnlyWindow::~MessageOnlyWindow()
{
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
    }
}

HWND MessageOnlyWindow::get() const noexcept
{
    return m_hwnd;
}

// RawKeyboardInput
RawKeyboardInput::RawKeyboardInput(HWND target)
{
    RAWINPUTDEVICE rid = {
        .usUsagePage = HID_USAGE_PAGE_GENERIC,
        .usUsage     = HID_USAGE_GENERIC_KEYBOARD,
        .dwFlags     = RIDEV_INPUTSINK,
        .hwndTarget  = target,
    };
    if (!RegisterRawInputDevices(&rid, 1, sizeof(rid))) {
        throw error_message(GetLastError(), "RegisterRawInputDevices failed.");
    }
}

RawKeyboardInput::~RawKeyboardInput()
{
    RAWINPUTDEVICE rid = {
        .usUsagePage = HID_USAGE_PAGE_GENERIC,
        .usUsage     = HID_USAGE_GENERIC_KEYBOARD,
        .dwFlags     = RIDEV_REMOVE,
        .hwndTarget  = nullptr,
    };
    RegisterRawInputDevices(&rid, 1, sizeof(rid));  // 忽略返回值
}

void vkey_handler(USHORT vk, UINT msg)
{
    switch (vk) {
        case VK_END: {
            ExitProcess(0); break;
        }
        default: {
            if (msg == WM_KEYDOWN) {
                std::cout << std::format("事件：虚拟按键 {:02x} 被按下。\n", vk);
            } else if (msg == WM_KEYUP) {
                std::cout << std::format("事件：虚拟按键 {:02x} 被松开。\n", vk);
            }
        }
    }
}

// 窗口过程：处理 WM_INPUT 消息
LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
        case WM_INPUT: {
            HRAWINPUT h_raw = reinterpret_cast<HRAWINPUT>(lParam);
            RawInputBuffer raw(h_raw);
            if (!raw.valid()) {
                break;
            }

            const RAWKEYBOARD *kb = as_keyboard(*raw.get());
            if (!kb) {
                break; // 如果不是键盘事件
            }

            vkey_handler(kb->VKey, kb->Message);

            break;
        }
    }
    return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}
