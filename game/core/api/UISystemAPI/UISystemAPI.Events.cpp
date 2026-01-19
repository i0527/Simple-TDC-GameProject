#include "../UISystemAPI.hpp"

namespace game {
namespace core {

ui::UIEvent UISystemAPI::MakeClickEvent(float x, float y) const {
    ui::UIEvent ev;
    ev.type = ui::UIEventType::Click;
    ev.x = x;
    ev.y = y;
    return ev;
}

ui::UIEvent UISystemAPI::MakeHoverEvent(float x, float y) const {
    ui::UIEvent ev;
    ev.type = ui::UIEventType::Hover;
    ev.x = x;
    ev.y = y;
    return ev;
}

ui::UIEvent UISystemAPI::MakeKeyEvent(int key) const {
    ui::UIEvent ev;
    ev.type = ui::UIEventType::Key;
    ev.key = key;
    return ev;
}

ui::UIEventResult UISystemAPI::DispatchEvent(
    const ui::UIEvent& ev,
    const std::shared_ptr<ui::IUIComponent>& root) const {
    if (!root) {
        return {};
    }
    return root->HandleEvent(ev);
}

ui::UIEventResult UISystemAPI::DispatchEvent(
    const ui::UIEvent& ev,
    ui::IUIComponent* root) const {
    if (!root) {
        return {};
    }
    return root->HandleEvent(ev);
}

} // namespace core
} // namespace game
