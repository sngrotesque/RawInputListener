#pragma once

// clang-format off
#include <windows.h>
#include <hidusage.h>

#include <iostream>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

// RawInputBuffer
class RawInputBuffer {
private:
    std::vector<BYTE> m_buffer;
    bool m_valid = false;

public:
    explicit RawInputBuffer(HRAWINPUT hRawInput) noexcept;

public:
    bool valid() const noexcept;
    const RAWINPUT *get() const noexcept;
    const RAWINPUT *operator->() const noexcept;
};

// WindowClass
class WindowClass {
private:
    LPCWSTR   m_name     = nullptr;
    HINSTANCE m_instance = nullptr;

public:
    WindowClass(LPCWSTR name, HINSTANCE hInstance, WNDPROC proc);
    ~WindowClass();

public:
    WindowClass(const WindowClass &)            = delete;
    WindowClass &operator=(const WindowClass &) = delete;

public:
    LPCWSTR name() const noexcept;
};

// MessageOnlyWindow
class MessageOnlyWindow {
private:
    HWND m_hwnd = nullptr;

public:
    MessageOnlyWindow(LPCWSTR className, LPCWSTR title, HINSTANCE hInstance);
    ~MessageOnlyWindow();

public:
    MessageOnlyWindow(const MessageOnlyWindow &)            = delete;
    MessageOnlyWindow &operator=(const MessageOnlyWindow &) = delete;

public:
    HWND get() const noexcept;
};

// RawKeyboardInput
class RawKeyboardInput {
public:
    explicit RawKeyboardInput(HWND target);
    ~RawKeyboardInput();

public:
    RawKeyboardInput(const RawKeyboardInput &)            = delete;
    RawKeyboardInput &operator=(const RawKeyboardInput &) = delete;
};
