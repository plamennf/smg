#include "main.h"

#ifdef PLATFORM_WINDOWS

#include <Windows.h>
#include <Windowsx.h>

extern "C" {
    __declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

int os_window_width;
int os_window_height;
bool os_window_is_open;

HWND g_hwnd;
WINDOWPLACEMENT prev_wp;
bool was_resized;

#define WINDOW_CLASS_NAME L"SMGWin32WindowClass"

static LARGE_INTEGER global_perf_freq;
static u64 nanoseconds_per_tick;

static LRESULT CALLBACK os_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_CREATE: {
            CREATESTRUCTW *cs = (CREATESTRUCTW *)lparam;
            SetWindowLongPtrW(hwnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        } break;
        
        case WM_CLOSE:
        case WM_DESTROY: {
            os_window_is_open = false;
        } break;

        case WM_SIZE: {
            RECT rect;
            GetClientRect(hwnd, &rect);
            os_window_width  = rect.right  - rect.left;
            os_window_height = rect.bottom - rect.top;

            was_resized = true;
        } break;

        case WM_SYSCHAR: {
            // Prevent windows from beeping when pressing an alt-key combo.
        } break;

        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP: {
            Key_Code key_code = (Key_Code)wparam;
            bool is_down = (msg == WM_KEYDOWN) || (msg == WM_SYSKEYDOWN);

            set_key_state(key_code, is_down);
        } break;

        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP: {
            bool is_down = msg == WM_LBUTTONDOWN;

            set_mouse_button_state(MOUSE_BUTTON_LEFT, is_down);
        } break;

        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP: {
            bool is_down = msg == WM_RBUTTONDOWN;

            set_mouse_button_state(MOUSE_BUTTON_RIGHT, is_down);
        } break;

        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP: {
            bool is_down = msg == WM_MBUTTONDOWN;

            set_mouse_button_state(MOUSE_BUTTON_MIDDLE, is_down);
        } break;

        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP: {
            int button = GET_XBUTTON_WPARAM(wparam);

            Mouse_Button mouse_button;
            if (button & XBUTTON1) {
                mouse_button = MOUSE_BUTTON_X1;
            } else if (button & XBUTTON2) {
                mouse_button = MOUSE_BUTTON_X2;
            } else {
                Assert(!"Invalid mouse button");
                mouse_button = (Mouse_Button)0;
            }

            bool is_down = msg == WM_XBUTTONDOWN;
            
            set_mouse_button_state(mouse_button, is_down);
            
            return TRUE;
        } break;

        case WM_MOUSEMOVE: {
            int x_pos = GET_X_LPARAM(lparam);
            int y_pos = GET_Y_LPARAM(lparam);

            mouse_cursor_x_delta = x_pos - mouse_cursor_x;
            mouse_cursor_y_delta = mouse_cursor_y - y_pos;

            mouse_cursor_x = x_pos;
            mouse_cursor_y = y_pos;
        } break;

        case WM_MOUSEWHEEL: {
            int total_wheel_delta = GET_WHEEL_DELTA_WPARAM(wparam);

            mouse_scroll_wheel_y_delta = total_wheel_delta / WHEEL_DELTA; // 120
        } break;

        case WM_MOUSEHWHEEL: {
            int total_wheel_delta = GET_WHEEL_DELTA_WPARAM(wparam);

            mouse_scroll_wheel_x_delta = total_wheel_delta / WHEEL_DELTA; // 120
        } break;
            
        default: {
            return DefWindowProcW(hwnd, msg, wparam, lparam);
        } break;
    }

    return 0;
}

bool os_window_create(int width, int height, char *title) {
    WNDCLASSEXW wc   = {};
    wc.cbSize        = sizeof(wc);
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.lpfnWndProc   = os_wnd_proc;
    wc.hInstance     = GetModuleHandleW(NULL);
    wc.hIcon         = LoadIconW(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hIconSm       = LoadIconW(NULL, IDI_APPLICATION);

    if (RegisterClassExW(&wc) == 0) {
        logprintf("Failed to register the window class!\n");
        return false;
    }

    if (width <= 0 || height <= 0) {
        HWND desktop = GetDesktopWindow();
        RECT desktop_rect;
        GetWindowRect(desktop, &desktop_rect);

        int desktop_width  = desktop_rect.right - desktop_rect.left;
        int desktop_height = desktop_rect.bottom - desktop_rect.top;

        width  = (int)((double)desktop_width  * 2.0/3.0);
        height = (int)((double)desktop_height * 2.0/3.0);
    }

    DWORD window_style = WS_OVERLAPPEDWINDOW;
    
    RECT  window_rect  = {0, 0, width, height};
    AdjustWindowRect(&window_rect, window_style, FALSE);

    int window_width   = window_rect.right  - window_rect.left;
    int window_height  = window_rect.bottom - window_rect.top;

    wchar_t wide_title[4096] = {};
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wide_title, ArrayCount(wide_title));
    g_hwnd = CreateWindowExW(0, WINDOW_CLASS_NAME, wide_title, window_style,
                             CW_USEDEFAULT, CW_USEDEFAULT,
                             window_width, window_height,
                             NULL, NULL, GetModuleHandleW(NULL), NULL);
    if (g_hwnd == NULL) {
        logprintf("Failed to create window!\n");
        return false;
    }

    MONITORINFOEXW mi = { sizeof(mi) };
    GetMonitorInfoW(MonitorFromWindow(g_hwnd, MONITOR_DEFAULTTOPRIMARY), &mi);

    int monitor_width_without_taskbar  = mi.rcWork.right  - mi.rcWork.left;
    int monitor_height_without_taskbar = mi.rcWork.bottom - mi.rcWork.top;

    int window_x = mi.rcWork.left + ((monitor_width_without_taskbar  - window_width)  / 2);
    int window_y = mi.rcWork.top  + ((monitor_height_without_taskbar - window_height) / 2);

    SetWindowPos(g_hwnd, HWND_TOP, window_x, window_y, 0, 0, SWP_NOSIZE);

    UpdateWindow(g_hwnd);
    ShowWindow(g_hwnd, SW_SHOWDEFAULT);

    os_window_width   = width;
    os_window_height  = height;
    os_window_is_open = true;

    return true;
}

void os_poll_events() {
    clear_key_states();
    clear_mouse_button_states();
    
    mouse_cursor_x_delta = 0;
    mouse_cursor_y_delta = 0;

    mouse_scroll_wheel_x_delta = 0;
    mouse_scroll_wheel_y_delta = 0;
    
    MSG msg;
    while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void *os_window_get_native() {
    return (void *)g_hwnd;
}

bool os_window_was_resized() {
    bool result = was_resized;
    was_resized = false;
    return result;
}

// From https://devblogs.microsoft.com/oldnewthing/20100412-00/?p=14353
void os_window_toggle_fullscreen() {
    DWORD style = GetWindowLong(g_hwnd, GWL_STYLE);
    if (style & WS_OVERLAPPEDWINDOW) {
        MONITORINFO mi = { sizeof(mi) };
        if (GetWindowPlacement(g_hwnd, &prev_wp) &&
            GetMonitorInfo(MonitorFromWindow(g_hwnd,
                                             MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLong(g_hwnd, GWL_STYLE,
                          style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(g_hwnd, HWND_TOP,
                         mi.rcMonitor.left, mi.rcMonitor.top,
                         mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else {
        SetWindowLong(g_hwnd, GWL_STYLE,
                      style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(g_hwnd, &prev_wp);
        SetWindowPos(g_hwnd, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }

    RECT rect;
    GetClientRect(g_hwnd, &rect);
    os_window_width  = rect.right  - rect.left;
    os_window_height = rect.bottom - rect.top;

    //was_resized = true;
}

void os_init() {
    timeBeginPeriod(1);

    // Try Windows 10+ per-monitor V2 awareness
    HMODULE shcore = LoadLibraryA("Shcore.dll");
    if (shcore) {
        typedef HRESULT(WINAPI *SetProcessDpiAwarenessFn)(int);
        SetProcessDpiAwarenessFn SetProcessDpiAwarenessPtr =
            (SetProcessDpiAwarenessFn)GetProcAddress(shcore, "SetProcessDpiAwareness");
        if (SetProcessDpiAwarenessPtr) {
            // 2 - PROCESS_PER_MONITOR_DPI_AWARE
            SetProcessDpiAwarenessPtr(2);
            FreeLibrary(shcore);
            return;
        }
        FreeLibrary(shcore);
    }

    // Try Windows 8.1+ API (SetProcessDpiAwarenessContext)
    HMODULE user32 = LoadLibraryA("User32.dll");
    if (user32) {
        typedef BOOL(WINAPI *SetProcessDpiAwarenessContextFn)(HANDLE);
        SetProcessDpiAwarenessContextFn SetProcessDpiAwarenessContextPtr =
            (SetProcessDpiAwarenessContextFn)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
        if (SetProcessDpiAwarenessContextPtr) {
            SetProcessDpiAwarenessContextPtr(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
            FreeLibrary(user32);
            return;
        }
        FreeLibrary(user32);
    }

    // Fallback for Windows 7 and older
    HMODULE user32_old = LoadLibraryA("User32.dll");
    if (user32_old) {
        typedef BOOL(WINAPI *SetProcessDPIAwareFn)(void);
        SetProcessDPIAwareFn SetProcessDPIAwarePtr =
            (SetProcessDPIAwareFn)GetProcAddress(user32_old, "SetProcessDPIAware");
        if (SetProcessDPIAwarePtr) {
            SetProcessDPIAwarePtr();
        }
        FreeLibrary(user32_old);
    }
}

void os_shutdown() {
    timeEndPeriod(1);
}

u64 os_get_time_in_nanoseconds() {
    if (!global_perf_freq.QuadPart) {
        QueryPerformanceFrequency(&global_perf_freq);
        nanoseconds_per_tick = 1000000000 / global_perf_freq.QuadPart;
    }
    
    LARGE_INTEGER perf_counter;
    QueryPerformanceCounter(&perf_counter);
    
    return perf_counter.QuadPart * nanoseconds_per_tick;
}

void os_show_and_unlock_cursor() {
    // Unlock cursor from any clipping region
    ClipCursor(NULL);

    // Make sure cursor is visible
    while (ShowCursor(TRUE) < 0) {
        // ShowCursor uses an internal counter; loop until visible
    }
}

void os_hide_and_lock_cursor() {
    HWND hwnd = g_hwnd;

    // Hide cursor
    while (ShowCursor(FALSE) >= 0) {
        // loop until actually hidden
    }

    // Get client rect and convert to screen coords
    RECT rect;
    GetClientRect(hwnd, &rect);

    POINT ul = { rect.left, rect.top };
    POINT lr = { rect.right, rect.bottom };

    ClientToScreen(hwnd, &ul);
    ClientToScreen(hwnd, &lr);

    rect.left   = ul.x;
    rect.top    = ul.y;
    rect.right  = lr.x;
    rect.bottom = lr.y;

    // Lock cursor to window client area
    ClipCursor(&rect);

    POINT old_point;
    GetCursorPos(&old_point);
    
    int center_x = (rect.left + rect.right) / 2;
    int center_y = (rect.top + rect.bottom) / 2;
    SetCursorPos(center_x, center_y);

    mouse_cursor_x_delta = old_point.x - center_x;
    mouse_cursor_y_delta = center_y - old_point.y;
}

#endif
