#include "Hooks.hpp"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT hooks::methods::WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    // Feed releases, focus changes and capture changes even while the menu is hidden.
    // Otherwise a button pressed before closing the menu can remain stuck in ImGui.
    if (ImGui::GetCurrentContext()) {
        const LRESULT handled = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
        if (setting::menu::render_menu) {
            if (handled)
                return handled;
            switch (uMsg) {
            case WM_LBUTTONDOWN: case WM_LBUTTONUP: case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDOWN: case WM_RBUTTONUP: case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDOWN: case WM_MBUTTONUP: case WM_MBUTTONDBLCLK:
            case WM_MOUSEWHEEL: case WM_MOUSEHWHEEL: case WM_MOUSEMOVE:
            case WM_INPUT:
                return 0;
            case WM_XBUTTONDOWN: case WM_XBUTTONUP: case WM_XBUTTONDBLCLK:
                return TRUE;
            }
            const ImGuiIO& io = ImGui::GetIO();
            if (io.WantCaptureKeyboard || io.WantTextInput) {
                switch (uMsg) {
                case WM_KEYDOWN: case WM_KEYUP: case WM_CHAR:
                    return 0;
                }
            }
        }
    }

    return oWndProc ? CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam)
                    : DefWindowProc(hWnd, uMsg, wParam, lParam);
}
