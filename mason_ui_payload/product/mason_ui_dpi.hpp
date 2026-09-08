#pragma once

// Mason UI customization v1. One logical canvas for the menu and every HUD.
// No game or external-process integration. Used by the single-window DX11 host.
#include "mason_ui_options.hpp"
#include "imgui.h"
#include "imgui_internal.h"
#include <cfloat>
#include <cmath>
#include <vector>

namespace mason_ui::dpi {
struct MouseEventBackup { ImU32 id; float x; float y; };
struct FrameState {
    ImGuiContext* context = nullptr;
    float scale = 1.0f;
    ImVec2 pixels = ImVec2(0, 0);
    bool restore_warp_position = false;
    ImVec2 warp_position = ImVec2(0, 0);
    int rendered_frame = -1;
    std::vector<MouseEventBackup> queued_mouse_positions;
};
inline FrameState frame;

inline bool valid_position(float x, float y) {
    return std::isfinite(x) && std::isfinite(y) && x > -FLT_MAX / 2 && y > -FLT_MAX / 2;
}
inline void scale_position(ImVec2& value, float factor) {
    if (valid_position(value.x, value.y)) { value.x *= factor; value.y *= factor; }
}

// Must precede ImGui_ImplWin32_NewFrame (handles optional navigation cursor warps).
inline void before_platform_frame() {
    auto* context = ImGui::GetCurrentContext();
    if (frame.context != context) { frame = FrameState{}; frame.context = context; }
    auto& io = ImGui::GetIO();
    frame.restore_warp_position = io.WantSetMousePos && valid_position(io.MousePos.x, io.MousePos.y);
    if (frame.restore_warp_position) {
        frame.warp_position = io.MousePos;
        scale_position(io.MousePos, frame.scale);
    }
}

// Win32 has supplied physical pixels. Transform input BEFORE ImGui processes it.
inline void before_new_frame() {
    auto& g = *ImGui::GetCurrentContext();
    auto& io = g.IO;
    if (frame.restore_warp_position) io.MousePos = frame.warp_position;
    frame.restore_warp_position = false;
    frame.pixels = io.DisplaySize;
    const float previous_scale = frame.scale;
    bool dragging = g.ActiveId != 0;
    for (int i = 0; i < ImGuiMouseButton_COUNT; ++i) dragging = dragging || io.MouseDown[i];
    // Apply a changed percentage only after the active interaction is released.
    if (!dragging) frame.scale = std::clamp(options.dpi_percent, 75, 200) / 100.0f;
    io.DisplaySize = ImVec2(frame.pixels.x / frame.scale, frame.pixels.y / frame.scale);
    io.DisplayFramebufferScale = ImVec2(1, 1);

    if (frame.scale != previous_scale) {
        const float ratio = previous_scale / frame.scale;
        scale_position(io.MousePos, ratio);
        scale_position(io.MousePosPrev, ratio);
        for (int i = 0; i < ImGuiMouseButton_COUNT; ++i) scale_position(io.MouseClickedPos[i], ratio);
        // Keep top-level HUD anchors on screen; child/popup placement follows parents.
        for (int i = 0; i < g.Windows.Size; ++i) {
            auto* window = g.Windows[i];
            if (window->Flags & (ImGuiWindowFlags_ChildWindow | ImGuiWindowFlags_Popup | ImGuiWindowFlags_Tooltip)) continue;
            ImVec2 pos(window->Pos.x * ratio, window->Pos.y * ratio);
            pos.x = std::clamp(pos.x, 0.0f, (std::max)(0.0f, io.DisplaySize.x - window->Size.x));
            pos.y = std::clamp(pos.y, 0.0f, (std::max)(0.0f, io.DisplaySize.y - window->Size.y));
            ImGui::SetWindowPos(window->Name, pos, ImGuiCond_Always);
        }
    }

    frame.queued_mouse_positions.clear();
    for (int i = 0; i < g.InputEventsQueue.Size; ++i) {
        auto& event = g.InputEventsQueue[i];
        if (event.Type != ImGuiInputEventType_MousePos) continue;
        frame.queued_mouse_positions.push_back({event.EventId, event.MousePos.PosX, event.MousePos.PosY});
        if (valid_position(event.MousePos.PosX, event.MousePos.PosY)) {
            event.MousePos.PosX /= frame.scale;
            event.MousePos.PosY /= frame.scale;
        }
    }
}

// NewFrame may leave trickled events queued. Restore their EXACT physical values
// now, so they are not divided twice on the next frame. Other event types stay intact.
inline void after_new_frame() {
    auto& queue = ImGui::GetCurrentContext()->InputEventsQueue;
    for (int i = 0; i < queue.Size; ++i) {
        auto& event = queue[i];
        if (event.Type != ImGuiInputEventType_MousePos) continue;
        for (const auto& saved : frame.queued_mouse_positions) {
            if (saved.id != event.EventId) continue;
            event.MousePos.PosX = saved.x;
            event.MousePos.PosY = saved.y;
            break;
        }
    }
    frame.queued_mouse_positions.clear();
}

// Transform the completed logical draw data once. This scales text, icons,
// shadows, popup geometry, keybinds, watermark and hit logs by the same amount.
inline void scale_draw_data(ImDrawData* data) {
    if (!data || frame.rendered_frame == ImGui::GetFrameCount()) return;
    frame.rendered_frame = ImGui::GetFrameCount();
    const float s = frame.scale;
    for (int i = 0; i < data->CmdListsCount; ++i) {
        auto* list = data->CmdLists[i];
        for (int j = 0; j < list->VtxBuffer.Size; ++j) {
            list->VtxBuffer[j].pos.x *= s;
            list->VtxBuffer[j].pos.y *= s;
        }
        for (int j = 0; j < list->CmdBuffer.Size; ++j) {
            auto& clip = list->CmdBuffer[j].ClipRect;
            clip.x *= s; clip.y *= s; clip.z *= s; clip.w *= s;
        }
    }
    data->DisplayPos.x *= s; data->DisplayPos.y *= s;
    data->DisplaySize = frame.pixels;
    data->FramebufferScale = ImVec2(1, 1);
}
}
