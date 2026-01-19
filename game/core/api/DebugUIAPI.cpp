#include "DebugUIAPI.hpp"

// 標準ライブラリ
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>

// 外部ライブラリ
#include <imgui.h>

// プロジェクト内
#include "../config/SharedContext.hpp"
#include "../api/BaseSystemAPI.hpp"
#include "../api/GameplayDataAPI.hpp"
#include "../api/InputSystemAPI.hpp"

namespace game {
namespace core {

namespace {

bool ContainsCaseInsensitive(const std::string& haystack, const std::string& needle) {
    if (needle.empty()) return true;
    if (haystack.empty()) return false;

    auto toLower = [](unsigned char c) { return static_cast<char>(std::tolower(c)); };

    std::string h;
    h.resize(haystack.size());
    std::transform(haystack.begin(), haystack.end(), h.begin(), toLower);

    std::string n;
    n.resize(needle.size());
    std::transform(needle.begin(), needle.end(), n.begin(), toLower);

    return h.find(n) != std::string::npos;
}

} // namespace

DebugUIAPI::DebugUIAPI()
    : sharedContext_(nullptr),
      isInitialized_(false),
      isVisible_(false),
      nextPanelId_(1) {}

bool DebugUIAPI::Initialize(SharedContext* sharedContext) {
    if (!sharedContext) {
        isInitialized_ = false;
        return false;
    }

    sharedContext_ = sharedContext;
    isInitialized_ = true;
    return true;
}

void DebugUIAPI::Shutdown() {
    panels_.clear();
    sharedContext_ = nullptr;
    isInitialized_ = false;
    isVisible_ = false;
}

bool DebugUIAPI::IsInitialized() const {
    return isInitialized_;
}

void DebugUIAPI::UpdateToggle() {
    if (!sharedContext_ || !sharedContext_->inputAPI) {
        return;
    }

    if (sharedContext_->inputAPI->IsDebugTogglePressed()) {
        isVisible_ = !isVisible_;
    }
}

int DebugUIAPI::RegisterPanel(const std::string& name,
                              const std::function<void(SharedContext&)>& render) {
    Panel panel;
    panel.id = nextPanelId_++;
    panel.name = name;
    panel.render = render;
    panels_.push_back(panel);
    return panel.id;
}

void DebugUIAPI::UnregisterPanel(int panelId) {
    panels_.erase(
        std::remove_if(panels_.begin(), panels_.end(),
                       [panelId](const Panel& p) { return p.id == panelId; }),
        panels_.end());
}

void DebugUIAPI::Render() {
    if (!sharedContext_ || !isVisible_) {
        return;
    }

    ImGuiWindowFlags flags = 0;
    ImGui::SetNextWindowSize(ImVec2(720.0f, 540.0f), ImGuiCond_FirstUseEver);

    bool open = isVisible_;
    if (!ImGui::Begin("Debug##Global", &open, flags)) {
        ImGui::End();
        isVisible_ = open;
        return;
    }

    isVisible_ = open;

    if (ImGui::CollapsingHeader("Common", ImGuiTreeNodeFlags_DefaultOpen)) {
        RenderCommonPanel(*sharedContext_);
    }

    for (const auto& panel : panels_) {
        if (!panel.render) {
            continue;
        }
        const std::string label = panel.name + "##DebugPanel" + std::to_string(panel.id);
        if (ImGui::CollapsingHeader(label.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            panel.render(*sharedContext_);
        }
    }

    ImGui::End();
}

void DebugUIAPI::SyncEditFieldsFromSave(const GameplayDataAPI& gameplayDataAPI) {
    const auto& save = gameplayDataAPI.GetSaveData();
    editGold_ = save.gold;
    editGems_ = save.gems;
    editTickets_ = save.tickets;
    editMaxTickets_ = save.maxTickets;
    currencyEditInitialized_ = true;
}

void DebugUIAPI::RenderCommonPanel(SharedContext& ctx) {
    // ===== Currency =====
    if (ImGui::CollapsingHeader("Currency", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ctx.gameplayDataAPI) {
            const auto& save = ctx.gameplayDataAPI->GetSaveData();
            ImGui::Text("gold: %d", save.gold);
            ImGui::Text("gems: %d", save.gems);
            ImGui::Text("tickets: %d / %d", save.tickets, save.maxTickets);

            // ウィンドウを開いた直後は編集フィールドをセーブ値で初期化
            if (!currencyEditInitialized_ || ImGui::IsWindowAppearing()) {
                SyncEditFieldsFromSave(*ctx.gameplayDataAPI);
            }

            ImGui::Separator();
            ImGui::TextUnformatted("Edit (apply to PlayerDataManager)");

            ImGui::InputInt("editGold", &editGold_);
            ImGui::InputInt("editGems", &editGems_);
            ImGui::InputInt("editTickets", &editTickets_);
            ImGui::InputInt("editMaxTickets", &editMaxTickets_);

            // ローカルで最低限のクランプ（負値防止）
            editGold_ = std::max(0, editGold_);
            editGems_ = std::max(0, editGems_);
            editTickets_ = std::max(0, editTickets_);
            editMaxTickets_ = std::max(0, editMaxTickets_);

            if (ImGui::Button("Apply##Currency")) {
                // maxTickets は先に適用し、tickets は maxTickets を超えないように調整
                ctx.gameplayDataAPI->SetGold(editGold_);
                ctx.gameplayDataAPI->SetGems(editGems_);
                ctx.gameplayDataAPI->SetMaxTickets(editMaxTickets_);
                ctx.gameplayDataAPI->SetTickets(std::min(editTickets_, editMaxTickets_));
            }

            ImGui::SameLine();
            if (ImGui::Button("Save##Currency")) {
                lastSaveResult_ = ctx.gameplayDataAPI->Save();
                hasLastSaveResult_ = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Reload##Currency")) {
                SyncEditFieldsFromSave(*ctx.gameplayDataAPI);
            }

            if (hasLastSaveResult_) {
                if (lastSaveResult_) {
                    ImGui::TextUnformatted("Save: OK");
                } else {
                    ImGui::TextUnformatted("Save: FAILED (see logs)");
                }
            }
        } else {
            ImGui::TextDisabled("gameplayDataAPI: null");
        }
    }

    // ===== Managers / Context =====
    if (ImGui::CollapsingHeader("Managers / SharedContext", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("systemAPI: %s", ctx.systemAPI ? "OK" : "null");
        ImGui::Text("ecsAPI: %s", ctx.ecsAPI ? "OK" : "null");
        ImGui::Text("sceneOverlayAPI: %s", ctx.sceneOverlayAPI ? "OK" : "null");
        ImGui::Text("gameplayDataAPI: %s", ctx.gameplayDataAPI ? "OK" : "null");

        if (ctx.gameplayDataAPI) {
            ImGui::Text("characters: %d", static_cast<int>(ctx.gameplayDataAPI->GetCharacterCount()));
            ImGui::Text("stages: %d", static_cast<int>(ctx.gameplayDataAPI->GetStageCount()));
        } else {
            ImGui::Text("characters: n/a");
            ImGui::Text("stages: n/a");
        }

        ImGui::Text("currentStageId: %s", ctx.currentStageId.c_str());
        ImGui::Text("deltaTime: %.4f", ctx.deltaTime);
        ImGui::Text("isPaused: %s", ctx.isPaused ? "true" : "false");
        ImGui::Text("requestShutdown: %s", ctx.requestShutdown ? "true" : "false");
    }

    // ===== BaseSystemAPI =====
    if (ImGui::CollapsingHeader("BaseSystemAPI", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ctx.systemAPI) {
            ImGui::Text("IsInitialized: %s", ctx.systemAPI->IsInitialized() ? "true" : "false");
            ImGui::Text("IsResourcesInitialized: %s",
                        ctx.systemAPI->Resource().IsResourcesInitialized() ? "true" : "false");
            ImGui::Text("IsImGuiInitialized: %s",
                        ctx.systemAPI->Render().IsImGuiInitialized() ? "true" : "false");
            ImGui::Text("FPS: %d", ctx.systemAPI->Timing().GetFPS());
            ImGui::Text("Resolution: screen=%dx%d internal=%dx%d",
                        ctx.systemAPI->Render().GetScreenWidth(),
                        ctx.systemAPI->Render().GetScreenHeight(),
                        ctx.systemAPI->Render().GetInternalWidth(),
                        ctx.systemAPI->Render().GetInternalHeight());
            ImGui::Text("Volume: master=%.2f se=%.2f bgm=%.2f",
                        ctx.systemAPI->Audio().GetMasterVolume(),
                        ctx.systemAPI->Audio().GetSEVolume(),
                        ctx.systemAPI->Audio().GetBGMVolume());
            ImGui::Text("CurrentMusic: %s",
                        ctx.systemAPI->Audio().GetCurrentMusicName().c_str());
        } else {
            ImGui::TextDisabled("systemAPI: null");
        }
    }

    // ===== Texture Cache =====
    if (ImGui::CollapsingHeader("Texture Cache", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (!ctx.systemAPI) {
            ImGui::TextDisabled("systemAPI: null");
        } else {
            const size_t count = ctx.systemAPI->Resource().GetTextureCacheCount();
            ImGui::Text("count: %d", static_cast<int>(count));

            ImGui::InputText("filter", textureFilter_.data(), textureFilter_.size());

            std::vector<ResourceSystemAPI::TextureCacheEntry> entries =
                ctx.systemAPI->Resource().GetTextureCacheEntries();
            std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
                return a.key < b.key;
            });

            const std::string filterStr(textureFilter_.data());

            if (ImGui::BeginTable("TextureCacheTable##DebugCommon", 5,
                                  ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY,
                                  ImVec2(0.0f, 220.0f))) {
                ImGui::TableSetupColumn("key", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("id", ImGuiTableColumnFlags_WidthFixed, 80.0f);
                ImGui::TableSetupColumn("w", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("h", ImGuiTableColumnFlags_WidthFixed, 60.0f);
                ImGui::TableSetupColumn("bytes", ImGuiTableColumnFlags_WidthFixed, 100.0f);
                ImGui::TableHeadersRow();

                for (const auto& e : entries) {
                    if (!ContainsCaseInsensitive(e.key, filterStr)) {
                        continue;
                    }

                    const std::uint64_t bytes = static_cast<std::uint64_t>(std::max(0, e.width)) *
                                                static_cast<std::uint64_t>(std::max(0, e.height)) * 4ULL;

                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    ImGui::TextUnformatted(e.key.c_str());

                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%u", e.id);

                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%d", e.width);

                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%d", e.height);

                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("%llu", static_cast<unsigned long long>(bytes));
                }

                ImGui::EndTable();
            }
        }
    }
}

} // namespace core
} // namespace game
