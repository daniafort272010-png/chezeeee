#pragma once

#include <algorithm>

class StyleManager {
public:
    void Styles() {
        auto& style = GImGui->Style;
        style.WindowRounding = 8;
        style.WindowPadding = ImVec2{ 0, 0 };
        style.WindowBorderSize = 1;
        style.FrameRounding = 3;
        style.FramePadding = ImVec2{ 14, 12 };
        style.FrameBorderSize = 0;
        style.PopupRounding = 6;
        style.PopupBorderSize = 1;
        style.ChildRounding = 6;
        style.ChildBorderSize = 1;
        style.ItemSpacing = ImVec2{ 14, 14 };
        style.ItemInnerSpacing = ImVec2{ 10, 10 };
        style.ScrollbarRounding = 4;
        style.ScrollbarSize = 4;
        style.WindowMinSize = ImVec2{ 1, 1 };
    }

    void Colors() {
        ImVec4* colors = GImGui->Style.Colors;
        const ImVec4 bg_main(0.055f, 0.070f, 0.100f, 1.0f);
        const ImVec4 bg_child(0.080f, 0.100f, 0.140f, 1.0f);
        const ImVec4 frame_bg(0.115f, 0.145f, 0.200f, 1.0f);
        const ImVec4 hovered(0.150f, 0.210f, 0.310f, 1.0f);
        const ImVec4 active(0.180f, 0.270f, 0.410f, 1.0f);
        const ImVec4 accent(0.36f, 0.64f, 1.00f, 1.0f);
        const ImVec4 accent_active(0.55f, 0.76f, 1.00f, 1.0f);
        const ImVec4 text(0.94f, 0.96f, 1.00f, 1.0f);
        const ImVec4 muted(0.64f, 0.70f, 0.80f, 1.0f);
        const ImVec4 border(0.38f, 0.44f, 0.54f, 1.0f);

        colors[ImGuiCol_Scheme] = accent;
        colors[ImGuiCol_Text] = text;
        colors[ImGuiCol_TextButton] = text;
        colors[ImGuiCol_TextDisabled] = muted;
        colors[ImGuiCol_TextHovered] = accent_active;
        colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.28f);
        colors[ImGuiCol_WindowBg] = bg_main;
        colors[ImGuiCol_ChildBg] = bg_child;
        colors[ImGuiCol_PopupBg] = bg_child;
        colors[ImGuiCol_Border] = border;
        colors[ImGuiCol_BorderShadow] = ImVec4(0, 0, 0, 0);
        colors[ImGuiCol_FrameBg] = frame_bg;
        colors[ImGuiCol_FrameBgHovered] = hovered;
        colors[ImGuiCol_FrameBgActive] = active;
        colors[ImGuiCol_Button] = frame_bg;
        colors[ImGuiCol_ButtonHovered] = hovered;
        colors[ImGuiCol_ButtonActive] = active;
        colors[ImGuiCol_CheckMark] = accent;
        colors[ImGuiCol_SliderGrab] = accent;
        colors[ImGuiCol_SliderGrabActive] = accent_active;
        colors[ImGuiCol_Header] = frame_bg;
        colors[ImGuiCol_HeaderHovered] = hovered;
        colors[ImGuiCol_HeaderActive] = active;
        colors[ImGuiCol_Tab] = frame_bg;
        colors[ImGuiCol_TabHovered] = hovered;
        colors[ImGuiCol_TabActive] = active;
        colors[ImGuiCol_TabUnfocused] = bg_child;
        colors[ImGuiCol_TabUnfocusedActive] = frame_bg;
        colors[ImGuiCol_ScrollbarBg] = ImVec4(0, 0, 0, 0);
        colors[ImGuiCol_ScrollbarGrab] = accent;
        colors[ImGuiCol_ScrollbarGrabHovered] = accent_active;
        colors[ImGuiCol_ScrollbarGrabActive] = accent_active;
        colors[ImGuiCol_Separator] = border;
        colors[ImGuiCol_NavHighlight] = accent;
    }

    static StyleManager& get() {
        static StyleManager s{};
        return s;
    }
};
