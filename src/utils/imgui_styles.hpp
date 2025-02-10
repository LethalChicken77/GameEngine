#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include <cmath>

#define ImLerp(a, b, t) (ImVec4((a).x + ((b).x - (a).x) * (t), (a).y + ((b).y - (a).y) * (t), (a).z + ((b).z - (a).z) * (t), (a).w + ((b).w - (a).w) * (t)))

// Helper function to convert a single channel from sRGB to linear.
inline float SrgbToLinear(float c)
{
    return (c <= 0.04045f) ? (c / 12.92f) : powf((c + 0.055f) / 1.055f, 2.4f);
}

// Helper function to convert an ImVec4 color from sRGB to linear (leaves alpha unchanged).
inline ImVec4 SrgbToLinear(const ImVec4& col)
{
    return ImVec4(SrgbToLinear(col.x), SrgbToLinear(col.y), SrgbToLinear(col.z), col.w);
}

// A modified version of StyleColorsDark that converts its sRGB colors to linear.
void StyleColorsDarkLinear(ImGuiStyle* dst)
{
    ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImGuiCol_Text]                   = SrgbToLinear(ImVec4(1.00f, 1.00f, 1.00f, 1.00f));
    colors[ImGuiCol_TextDisabled]           = SrgbToLinear(ImVec4(0.50f, 0.50f, 0.50f, 1.00f));
    colors[ImGuiCol_WindowBg]               = SrgbToLinear(ImVec4(0.06f, 0.06f, 0.06f, 0.94f));
    colors[ImGuiCol_ChildBg]                = SrgbToLinear(ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    colors[ImGuiCol_PopupBg]                = SrgbToLinear(ImVec4(0.08f, 0.08f, 0.08f, 0.94f));
    colors[ImGuiCol_Border]                 = SrgbToLinear(ImVec4(0.43f, 0.43f, 0.50f, 0.50f));
    colors[ImGuiCol_BorderShadow]           = SrgbToLinear(ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    colors[ImGuiCol_FrameBg]                = SrgbToLinear(ImVec4(0.16f, 0.29f, 0.48f, 0.54f));
    colors[ImGuiCol_FrameBgHovered]         = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 0.40f));
    colors[ImGuiCol_FrameBgActive]          = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 0.67f));
    colors[ImGuiCol_TitleBg]                = SrgbToLinear(ImVec4(0.04f, 0.04f, 0.04f, 1.00f));
    colors[ImGuiCol_TitleBgActive]          = SrgbToLinear(ImVec4(0.16f, 0.29f, 0.48f, 1.00f));
    colors[ImGuiCol_TitleBgCollapsed]       = SrgbToLinear(ImVec4(0.00f, 0.00f, 0.00f, 0.51f));
    colors[ImGuiCol_MenuBarBg]              = SrgbToLinear(ImVec4(0.14f, 0.14f, 0.14f, 1.00f));
    colors[ImGuiCol_ScrollbarBg]            = SrgbToLinear(ImVec4(0.02f, 0.02f, 0.02f, 0.53f));
    colors[ImGuiCol_ScrollbarGrab]          = SrgbToLinear(ImVec4(0.31f, 0.31f, 0.31f, 1.00f));
    colors[ImGuiCol_ScrollbarGrabHovered]   = SrgbToLinear(ImVec4(0.41f, 0.41f, 0.41f, 1.00f));
    colors[ImGuiCol_ScrollbarGrabActive]    = SrgbToLinear(ImVec4(0.51f, 0.51f, 0.51f, 1.00f));
    colors[ImGuiCol_CheckMark]              = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    colors[ImGuiCol_SliderGrab]             = SrgbToLinear(ImVec4(0.24f, 0.52f, 0.88f, 1.00f));
    colors[ImGuiCol_SliderGrabActive]       = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    colors[ImGuiCol_Button]                 = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 0.40f));
    colors[ImGuiCol_ButtonHovered]          = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    colors[ImGuiCol_ButtonActive]           = SrgbToLinear(ImVec4(0.06f, 0.53f, 0.98f, 1.00f));
    colors[ImGuiCol_Header]                 = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 0.31f));
    colors[ImGuiCol_HeaderHovered]          = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 0.80f));
    colors[ImGuiCol_HeaderActive]           = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    colors[ImGuiCol_Separator]              = colors[ImGuiCol_Border]; // already converted
    colors[ImGuiCol_SeparatorHovered]       = SrgbToLinear(ImVec4(0.10f, 0.40f, 0.75f, 0.78f));
    colors[ImGuiCol_SeparatorActive]        = SrgbToLinear(ImVec4(0.10f, 0.40f, 0.75f, 1.00f));
    colors[ImGuiCol_ResizeGrip]             = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 0.20f));
    colors[ImGuiCol_ResizeGripHovered]      = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 0.67f));
    colors[ImGuiCol_ResizeGripActive]       = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 0.95f));
    colors[ImGuiCol_TabHovered]             = colors[ImGuiCol_HeaderHovered]; // already converted
    // For the following lerp-based colors, note that the interpolation now occurs in linear space.
    colors[ImGuiCol_Tab]                    = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
    colors[ImGuiCol_TabSelected]            = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
    colors[ImGuiCol_TabSelectedOverline]    = colors[ImGuiCol_HeaderActive];
    colors[ImGuiCol_TabDimmed]              = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
    colors[ImGuiCol_TabDimmedSelected]      = ImLerp(colors[ImGuiCol_TabSelected], colors[ImGuiCol_TitleBg], 0.40f);
    colors[ImGuiCol_TabDimmedSelectedOverline] = SrgbToLinear(ImVec4(0.50f, 0.50f, 0.50f, 0.00f));
    colors[ImGuiCol_PlotLines]              = SrgbToLinear(ImVec4(0.61f, 0.61f, 0.61f, 1.00f));
    colors[ImGuiCol_PlotLinesHovered]       = SrgbToLinear(ImVec4(1.00f, 0.43f, 0.35f, 1.00f));
    colors[ImGuiCol_PlotHistogram]          = SrgbToLinear(ImVec4(0.90f, 0.70f, 0.00f, 1.00f));
    colors[ImGuiCol_PlotHistogramHovered]   = SrgbToLinear(ImVec4(1.00f, 0.60f, 0.00f, 1.00f));
    colors[ImGuiCol_TableHeaderBg]          = SrgbToLinear(ImVec4(0.19f, 0.19f, 0.20f, 1.00f));
    colors[ImGuiCol_TableBorderStrong]      = SrgbToLinear(ImVec4(0.31f, 0.31f, 0.35f, 1.00f));
    colors[ImGuiCol_TableBorderLight]       = SrgbToLinear(ImVec4(0.23f, 0.23f, 0.25f, 1.00f));
    colors[ImGuiCol_TableRowBg]             = SrgbToLinear(ImVec4(0.00f, 0.00f, 0.00f, 0.00f));
    colors[ImGuiCol_TableRowBgAlt]          = SrgbToLinear(ImVec4(1.00f, 1.00f, 1.00f, 0.06f));
    colors[ImGuiCol_TextLink]               = colors[ImGuiCol_HeaderActive]; // already converted
    colors[ImGuiCol_TextSelectedBg]         = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 0.35f));
    colors[ImGuiCol_DragDropTarget]         = SrgbToLinear(ImVec4(1.00f, 1.00f, 0.00f, 0.90f));
    colors[ImGuiCol_NavCursor]              = SrgbToLinear(ImVec4(0.26f, 0.59f, 0.98f, 1.00f));
    colors[ImGuiCol_NavWindowingHighlight]  = SrgbToLinear(ImVec4(1.00f, 1.00f, 1.00f, 0.70f));
    colors[ImGuiCol_NavWindowingDimBg]      = SrgbToLinear(ImVec4(0.80f, 0.80f, 0.80f, 0.20f));
    colors[ImGuiCol_ModalWindowDimBg]       = SrgbToLinear(ImVec4(0.80f, 0.80f, 0.80f, 0.35f));
}