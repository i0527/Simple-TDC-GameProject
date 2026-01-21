#pragma once

#include "../api/BaseSystemAPI.hpp"
#include <imgui.h>

namespace game {
namespace core {
namespace ui {

enum class ImGuiSoundType {
    Click,
    Switch,
    Tap
};

namespace ImGuiSound {

inline const char* ResolveSoundName(ImGuiSoundType type) {
    switch (type) {
    case ImGuiSoundType::Switch:
        return "switch-a";
    case ImGuiSoundType::Tap:
        return "tap-a";
    case ImGuiSoundType::Click:
    default:
        return "click-a";
    }
}

inline void PlaySound(BaseSystemAPI* systemAPI, ImGuiSoundType type) {
    if (!systemAPI) {
        return;
    }

    systemAPI->Audio().PlaySound(ResolveSoundName(type));
}

inline bool Button(BaseSystemAPI* systemAPI, const char* label,
                   const ImVec2& size = ImVec2(0, 0),
                   ImGuiSoundType type = ImGuiSoundType::Click) {
    const bool pressed = ImGui::Button(label, size);
    if (pressed) {
        PlaySound(systemAPI, type);
    }
    return pressed;
}

inline bool InvisibleButton(BaseSystemAPI* systemAPI, const char* strId,
                            const ImVec2& size, ImGuiButtonFlags flags = 0,
                            ImGuiSoundType type = ImGuiSoundType::Tap) {
    const bool pressed = ImGui::InvisibleButton(strId, size, flags);
    if (pressed) {
        PlaySound(systemAPI, type);
    }
    return pressed;
}

inline bool ImageButton(BaseSystemAPI* systemAPI, const char* strId, ImTextureID userTextureId,
                        const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0),
                        const ImVec2& uv1 = ImVec2(1, 1),
                        const ImVec4& bgCol = ImVec4(0, 0, 0, 0),
                        const ImVec4& tintCol = ImVec4(1, 1, 1, 1),
                        ImGuiSoundType type = ImGuiSoundType::Click) {
    const bool pressed = ImGui::ImageButton(strId, userTextureId, size, uv0, uv1, bgCol, tintCol);
    if (pressed) {
        PlaySound(systemAPI, type);
    }
    return pressed;
}

inline bool Checkbox(BaseSystemAPI* systemAPI, const char* label, bool* value,
                     ImGuiSoundType type = ImGuiSoundType::Switch) {
    const bool changed = ImGui::Checkbox(label, value);
    if (changed) {
        PlaySound(systemAPI, type);
    }
    return changed;
}

inline bool Selectable(BaseSystemAPI* systemAPI, const char* label,
                       bool selected = false, ImGuiSelectableFlags flags = 0,
                       const ImVec2& size = ImVec2(0, 0),
                       ImGuiSoundType type = ImGuiSoundType::Tap) {
    const bool pressed = ImGui::Selectable(label, selected, flags, size);
    if (pressed) {
        PlaySound(systemAPI, type);
    }
    return pressed;
}

inline bool Combo(BaseSystemAPI* systemAPI, const char* label,
                  int* currentItem, const char* const items[], int itemsCount,
                  int popupMaxHeightInItems = -1,
                  ImGuiSoundType type = ImGuiSoundType::Switch) {
    const bool changed =
        ImGui::Combo(label, currentItem, items, itemsCount, popupMaxHeightInItems);
    if (changed) {
        PlaySound(systemAPI, type);
    }
    return changed;
}

} // namespace ImGuiSound
} // namespace ui
} // namespace core
} // namespace game
