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

        ImVec4 bg_main = ImVec4(0.945f, 0.965f, 0.990f, 1.00f);
        ImVec4 bg_child = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        ImVec4 accent = ImVec4(0.00f, 0.22f, 0.62f, 1.00f);
        ImVec4 text = ImVec4(0.075f, 0.13f, 0.23f, 1.00f);
        ImVec4 border = ImVec4(0.73f, 0.79f, 0.88f, 1.00f);
        ImVec4 frame_bg = ImVec4(0.89f, 0.925f, 0.98f, 1.00f);

        ImVec4 accent_active = ImVec4((std::min)(accent.x + 0.15f, 1.0f), (std::min)(accent.y + 0.12f, 1.0f), (std::min)(accent.z + 0.10f, 1.0f), 1.0f);
        ImVec4 accent_hovered = ImVec4((std::min)(accent.x + 0.05f, 1.0f), (std::min)(accent.y + 0.05f, 1.0f), (std::min)(accent.z + 0.05f, 1.0f), 1.0f);

        colors[ImGuiCol_Scheme] = accent;

        colors[ImGuiCol_TextButton] = text;
        colors[ImGuiCol_TextDisabled] = ImVec4(0.32f, 0.39f, 0.49f, 1.0f);

        colors[ImGuiCol_WindowBg] = bg_main;
        colors[ImGuiCol_ChildBg] = bg_child;
        colors[ImGuiCol_PopupBg] = bg_main;
        colors[ImGuiCol_Border] = border;
        colors[ImGuiCol_Text] = text;
        colors[ImGuiCol_TextHovered] = accent;
        colors[ImGuiCol_TextSelectedBg] = ImVec4(accent.x, accent.y, accent.z, 0.07f);

        colors[ImGuiCol_FrameBg] = frame_bg;
        colors[ImGuiCol_FrameBgHovered] = ImVec4((std::min)(frame_bg.x + 0.02f, 1.0f), (std::min)(frame_bg.y + 0.02f, 1.0f), (std::min)(frame_bg.z + 0.02f, 1.0f), 1.0f);
        colors[ImGuiCol_FrameBgActive] = ImVec4((std::min)(frame_bg.x + 0.04f, 1.0f), (std::min)(frame_bg.y + 0.04f, 1.0f), (std::min)(frame_bg.z + 0.04f, 1.0f), 1.0f);

        colors[ImGuiCol_Button] = frame_bg;
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.78f, 0.85f, 0.96f, 1.0f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.65f, 0.76f, 0.93f, 1.0f);

        colors[ImGuiCol_CheckMark] = accent;
        colors[ImGuiCol_SliderGrab] = accent;
        colors[ImGuiCol_SliderGrabActive] = accent_active;

        colors[ImGuiCol_Header] = ImVec4(accent.x, accent.y, accent.z, 0.3f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(accent.x, accent.y, accent.z, 0.5f);
        colors[ImGuiCol_HeaderActive] = ImVec4(0.65f, 0.76f, 0.93f, 1.0f);

        colors[ImGuiCol_Tab] = frame_bg;
        colors[ImGuiCol_TabHovered] = ImVec4(0.78f, 0.85f, 0.96f, 1.0f);
        colors[ImGuiCol_TabActive] = ImVec4(0.65f, 0.76f, 0.93f, 1.0f);
        colors[ImGuiCol_TabUnfocused] = frame_bg;
        colors[ImGuiCol_TabUnfocusedActive] = frame_bg;

        colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.0f);
        colors[ImGuiCol_ScrollbarGrab] = accent;
        colors[ImGuiCol_ScrollbarGrabHovered] = accent;
        colors[ImGuiCol_ScrollbarGrabActive] = accent;
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_Separator] = border;
        colors[ImGuiCol_NavHighlight] = accent;
    }

    static StyleManager& get() {
        static StyleManager s{ };
        return s;
    }
};