#include "CodexOverlay.hpp"
#include "../../../utils/Log.h"
#include "../../api/BaseSystemAPI.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../ecs/entities/Character.hpp"
#include "../../ecs/entities/CharacterStatCalculator.hpp"
#include "../../ui/OverlayColors.hpp"
#include <algorithm>
#include <iomanip>
#include <regex>
#include <sstream>

namespace game {
namespace core {

namespace {

constexpr float kPanelHeaderH = 32.0f;
constexpr float kContentOffsetX = 20.0f;
constexpr float kContentOffsetY = 70.0f;
constexpr float kTabBarInset = 10.0f;
constexpr float kTabBarYOffset = 20.0f;
constexpr float kTabBarHeight = 40.0f;
constexpr float kTabBarGap = 4.0f;
constexpr float kTabButtonWidth = 110.0f;
constexpr float kTabButtonGap = 10.0f;

// UTF-8の次のコード�Eイント墁E��へ進める�E�不正な場合�E1バイト進める�E�E
size_t Utf8Next(const std::string &s, size_t i) {
  if (i >= s.size())
    return s.size();
  const unsigned char c = static_cast<unsigned char>(s[i]);
  if (c < 0x80)
    return i + 1;
  if ((c & 0xE0) == 0xC0)
    return std::min(s.size(), i + 2);
  if ((c & 0xF0) == 0xE0)
    return std::min(s.size(), i + 3);
  if ((c & 0xF8) == 0xF0)
    return std::min(s.size(), i + 4);
  return i + 1;
}

} // namespace

// ========== コンストラクタ・チE��トラクタ ==========

CodexOverlay::CodexOverlay()
    : systemAPI_(nullptr), isInitialized_(false), requestClose_(false),
      hasTransitionRequest_(false), requestedNextState_(GameState::Title) {}

// ========== IOverlay 実裁E==========

bool CodexOverlay::Initialize(BaseSystemAPI *systemAPI, UISystemAPI *uiAPI) {
  if (isInitialized_) {
    LOG_ERROR("CodexOverlay already initialized");
    return false;
  }

  if (!systemAPI) {
    LOG_ERROR("CodexOverlay: systemAPI is null");
    return false;
  }

  systemAPI_ = systemAPI;
  requestClose_ = false;
  hasTransitionRequest_ = false;

  activeTab_ = CodexTab::Characters;
  tabEntries_ = {};
  tabSelectedIndex_ = {{-1, -1, -1}};
  tabScrollOffset_ = {{0, 0, 0}};

  character_viewport_.has_error = false;
  character_viewport_.error_message.clear();

  isInitialized_ = true;
  LOG_INFO("CodexOverlay initialized");
  return true;
}

void CodexOverlay::Update(SharedContext &ctx, float deltaTime) {
  if (!isInitialized_) {
    return;
  }

  LayoutPanels();
  EnsureEntriesLoaded(ctx);

  // 試着状態�E初期化（キャラ選択が変わったら保存状態から復允E��E
  if (activeTab_ == CodexTab::Characters) {
    const auto *ch = GetSelectedCharacter();
    if (ch && tryOnCharacterId_ != ch->id) {
      tryOnCharacterId_ = ch->id;
      if (ctx.gameplayDataAPI) {
        tryOnState_ = ctx.gameplayDataAPI->GetCharacterState(ch->id);
      } else {
        tryOnState_ = {};
        tryOnState_.level = std::max(1, ch->default_level);
      }
      tryOnState_.unlocked = true; // 図鑑�EレビューではロチE��扱ぁE��しなぁE
    }
  }

  // ESCキーで閉じめE
  if (ctx.inputAPI && ctx.inputAPI->IsEscapePressed()) { // ESC key
    requestClose_ = true;
  }

  // アニメーション更新
  const entities::Character *selected = GetSelectedCharacter();
  if (selected && !character_viewport_.has_error) {
    const entities::Character::SpriteInfo *sprite_info = nullptr;
    if (character_viewport_.current_animation == AnimationType::Move) {
      sprite_info = &selected->move_sprite;
    } else {
      sprite_info = &selected->attack_sprite;
    }

    if (sprite_info && sprite_info->frame_count > 0 &&
        !character_viewport_.is_paused) {
      character_viewport_.animation_timer +=
          deltaTime * std::max(0.01f, character_viewport_.speed_multiplier);
      if (character_viewport_.animation_timer >= sprite_info->frame_duration) {
        character_viewport_.animation_frame++;
        character_viewport_.animation_timer = 0.0f;

        // フレーム数チェチE��
        if (character_viewport_.animation_frame >= sprite_info->frame_count) {
          if (character_viewport_.current_animation == AnimationType::Attack) {
            // 攻撁E��ニメーション終亁EↁE移動アニメーションに戻めE
            character_viewport_.current_animation = AnimationType::Move;
            character_viewport_.animation_frame = 0;
          } else {
            // 移動アニメーション�E�ルーチE
            character_viewport_.animation_frame = 0;
          }
        }
      }
    }
  }

  // マウスイベント�E琁E
  // コンチE��チE��域のオフセチE���E��EチE��ーとタブバーの間！E
  const float contentOffsetX = kContentOffsetX;
  const float contentOffsetY = kContentOffsetY;
  const float tabBarHeight = kTabBarHeight;
  const float tabBarGap = kTabBarGap;
  const float panelStartY =
      contentOffsetY + tabBarHeight + tabBarGap; // タブバーの下に配置
  const auto mouse_pos =
      ctx.inputAPI ? ctx.inputAPI->GetMousePosition() : Vec2{0.0f, 0.0f};

  // コンチE��チE��域冁E�E相対座標に変換�E�パネル座標系�E�E
  const float relative_x = mouse_pos.x - contentOffsetX;
  const float relative_y = mouse_pos.y - panelStartY;
  if (ctx.inputAPI && ctx.inputAPI->IsLeftClickPressed()) { // Left mouse button
    // タブバー�E�画面座標系で直接判定！E
    const float tabX = contentOffsetX + kTabBarInset;
    const float tabY = contentOffsetY + kTabBarInset + kTabBarYOffset;
    const float tabW = kTabButtonWidth;
    const float tabH = tabBarHeight;
    const float tabGap = kTabButtonGap;
    if (mouse_pos.y >= tabY && mouse_pos.y < tabY + tabH) {
      if (mouse_pos.x >= tabX && mouse_pos.x < tabX + tabW) {
        SwitchTab(CodexTab::Characters);
        return;
      } else if (mouse_pos.x >= tabX + (tabW + tabGap) * 1.0f &&
                 mouse_pos.x < tabX + (tabW + tabGap) * 1.0f + tabW) {
        SwitchTab(CodexTab::Equipment);
        return;
      } else if (mouse_pos.x >= tabX + (tabW + tabGap) * 2.0f &&
                 mouse_pos.x < tabX + (tabW + tabGap) * 2.0f + tabW) {
        SwitchTab(CodexTab::Passives);
        return;
      }
    }

    // コンチE��チE��域外�E無要E
    if (relative_x < 0.0f || relative_x >= 1880.0f || relative_y < 0.0f ||
        relative_y >= 900.0f) {
      return;
    }

    // グリチE��一覧クリチE��
    {
      const float listX = list_panel_.x + contentOffsetX;
      const float listY = list_panel_.y + panelStartY;
      const float listW = list_panel_.width;
      const float listH = list_panel_.height;

      if (mouse_pos.x >= listX && mouse_pos.x < listX + listW &&
          mouse_pos.y >= listY && mouse_pos.y < listY + listH) {
        const float innerX = mouse_pos.x - listX - list_panel_.padding;
        const float innerY =
            mouse_pos.y - listY - list_panel_.padding - kPanelHeaderH;
        if (innerX >= 0.0f && innerY >= 0.0f) {
          const int columns = std::max(
              1,
              static_cast<int>(std::floor(
                  (listW - list_panel_.padding * 2.0f + list_panel_.card_gap) /
                  (list_panel_.card_width + list_panel_.card_gap))));
          const int col = static_cast<int>(
              innerX / (list_panel_.card_width + list_panel_.card_gap));
          const int row = static_cast<int>(
              innerY / (list_panel_.card_height + list_panel_.card_gap));
          if (col >= 0 && col < columns && row >= 0) {
            const int ti = TabIndex(activeTab_);
            const auto &entries = tabEntries_[ti];
            const int index = (tabScrollOffset_[ti] + row) * columns + col;
            if (index >= 0 && index < static_cast<int>(entries.size())) {
              OnListItemClick(index);
              return;
            }
          }
        }
      }
    }

    // ビューポ�Eト�EのクリチE��判宁E
    if (relative_x >= character_viewport_.x &&
        relative_x < character_viewport_.x + character_viewport_.width &&
        relative_y >= character_viewport_.y &&
        relative_y < character_viewport_.y + character_viewport_.height) {
      if (activeTab_ == CodexTab::Characters) {
        // ビューア操作バー判定（ビューポ�Eト下部�E�E
        const float ctrlH = 32.0f;
        const float ctrlPad = 8.0f;
        const float ctrlY = character_viewport_.y + character_viewport_.height -
                            ctrlH - ctrlPad;
        if (relative_y >= ctrlY) {
          const float btnH = ctrlH;
          const float btnW = 64.0f;
          const float btnGap = 8.0f;
          float bx = character_viewport_.x + 10.0f;
          const float by = ctrlY;
          auto hit = [&](float x, float y, float w, float h) {
            return (relative_x >= x && relative_x < x + w && relative_y >= y &&
                    relative_y < y + h);
          };

          // Move / Attack
          if (hit(bx, by, btnW, btnH)) {
            character_viewport_.current_animation = AnimationType::Move;
            character_viewport_.animation_frame = 0;
            character_viewport_.animation_timer = 0.0f;
            return;
          }
          bx += btnW + btnGap;
          if (hit(bx, by, btnW, btnH)) {
            character_viewport_.current_animation = AnimationType::Attack;
            character_viewport_.animation_frame = 0;
            character_viewport_.animation_timer = 0.0f;
            return;
          }
          bx += btnW + btnGap;

          // Play/Pause
          if (hit(bx, by, btnW, btnH)) {
            character_viewport_.is_paused = !character_viewport_.is_paused;
            return;
          }
          bx += btnW + btnGap;

          // Prev / Next
          if (hit(bx, by, btnW, btnH)) {
            // prev
            const auto *ch = GetSelectedCharacter();
            if (!ch)
              return;
            const auto &sprite =
                (character_viewport_.current_animation == AnimationType::Move)
                    ? ch->move_sprite
                    : ch->attack_sprite;
            const int n = std::max(1, sprite.frame_count);
            character_viewport_.animation_frame =
                (character_viewport_.animation_frame - 1 + n) % n;
            character_viewport_.animation_timer = 0.0f;
            character_viewport_.is_paused = true;
            return;
          }
          bx += btnW + btnGap;
          if (hit(bx, by, btnW, btnH)) {
            // next
            const auto *ch = GetSelectedCharacter();
            if (!ch)
              return;
            const auto &sprite =
                (character_viewport_.current_animation == AnimationType::Move)
                    ? ch->move_sprite
                    : ch->attack_sprite;
            const int n = std::max(1, sprite.frame_count);
            character_viewport_.animation_frame =
                (character_viewport_.animation_frame + 1) % n;
            character_viewport_.animation_timer = 0.0f;
            character_viewport_.is_paused = true;
            return;
          }
          bx += btnW + btnGap;

          // Speed: 0.5x / 1x / 2x
          if (hit(bx, by, btnW, btnH)) {
            character_viewport_.speed_multiplier = 0.5f;
            return;
          }
          bx += btnW + btnGap;
          if (hit(bx, by, btnW, btnH)) {
            character_viewport_.speed_multiplier = 1.0f;
            return;
          }
          bx += btnW + btnGap;
          if (hit(bx, by, btnW, btnH)) {
            character_viewport_.speed_multiplier = 2.0f;
            return;
          }
          bx += btnW + btnGap;

          // Zoom - / +
          if (hit(bx, by, btnW, btnH)) {
            character_viewport_.zoom =
                std::max(0.5f, character_viewport_.zoom - 0.1f);
            return;
          }
          bx += btnW + btnGap;
          if (hit(bx, by, btnW, btnH)) {
            character_viewport_.zoom =
                std::min(2.0f, character_viewport_.zoom + 0.1f);
            return;
          }
          return;
        }

        // 操作バー以外�EクリチE��: 攻撁E��ニメーションに刁E��替ぁE
        character_viewport_.current_animation = AnimationType::Attack;
        character_viewport_.animation_frame = 0;
        character_viewport_.animation_timer = 0.0f;
      }
    }

    // ドロチE�Eダウンが開ぁE��ぁE��場合：選抁Eor 外クリチE��で閉じめE
    auto closeDropdown = [&]() {
      dropdownKind_ = DropdownKind::None;
      dropdownSlotIndex_ = -1;
      dropdownScrollPx_ = 0.0f;
    };

    auto isInRect = [&](float x, float y, float w, float h) {
      return (relative_x >= x && relative_x < x + w && relative_y >= y &&
              relative_y < y + h);
    };

    // ドロチE�Eダウンリスト�EクリチE��処琁E
    if (dropdownKind_ != DropdownKind::None && dropdownSlotIndex_ >= 0 &&
        dropdownSlotIndex_ < 3 && ctx.gameplayDataAPI) {
      const float y0 = status_panel_.y + status_panel_.padding + kPanelHeaderH;
      const float labelW = 140.0f;
      const float fieldH = 28.0f;
      float fieldX = 0.0f, fieldY = 0.0f, fieldW = 0.0f;

      if (dropdownKind_ == DropdownKind::EquipmentSlot) {
        fieldY = y0 + status_panel_.line_height *
                          (2.0f + static_cast<float>(dropdownSlotIndex_));
        fieldX = status_panel_.x + status_panel_.padding + labelW;
        fieldW = status_panel_.width - status_panel_.padding * 2.0f - labelW;
      } else if (dropdownKind_ == DropdownKind::PassiveSlot) {
        fieldY = y0 + status_panel_.line_height *
                          (6.0f + static_cast<float>(dropdownSlotIndex_));
        fieldX = status_panel_.x + status_panel_.padding + labelW;
        const float btn = 28.0f;
        const float gap = 8.0f;
        const float minusX = status_panel_.x + status_panel_.width -
                             status_panel_.padding - (btn * 2.0f + gap);
        fieldW = std::max(60.0f, minusX - gap - fieldX);
      }

      const float itemH = 28.0f;
      const int maxVisible = 8;
      const float listH =
          std::min(static_cast<float>(maxVisible), 10.0f) * itemH;
      const float listY = fieldY + fieldH;

      // リスト�EクリチE��判宁E
      if (isInRect(fieldX, listY, fieldW, listH)) {
        std::vector<std::pair<std::string, std::string>> items;

        if (dropdownKind_ == DropdownKind::EquipmentSlot) {
          items.emplace_back("", "なし");
          const auto &allEq = ctx.gameplayDataAPI->GetAllEquipment();
          for (const auto *eq : allEq) {
            if (eq)
              items.emplace_back(eq->id, eq->name);
          }
        } else if (dropdownKind_ == DropdownKind::PassiveSlot) {
          items.emplace_back("", "なし");
          const auto &allPs = ctx.gameplayDataAPI->GetAllPassiveSkills();
          for (const auto *ps : allPs) {
            if (ps)
              items.emplace_back(ps->id, ps->name);
          }
        }

        const int totalItems = static_cast<int>(items.size());
        const int visibleStart = std::max(
            0, static_cast<int>(std::floor(dropdownScrollPx_ / itemH)));
        const float clickY = relative_y - listY;
        const int clickedIndex =
            visibleStart + static_cast<int>(std::floor(
                               (clickY + dropdownScrollPx_ -
                                static_cast<float>(visibleStart) * itemH) /
                               itemH));

        if (clickedIndex >= 0 && clickedIndex < totalItems) {
          if (dropdownKind_ == DropdownKind::EquipmentSlot) {
            tryOnState_.equipment[dropdownSlotIndex_] =
                items[clickedIndex].first;
          } else if (dropdownKind_ == DropdownKind::PassiveSlot) {
            tryOnState_.passives[dropdownSlotIndex_].id =
                items[clickedIndex].first;
            if (items[clickedIndex].first.empty()) {
              tryOnState_.passives[dropdownSlotIndex_].level = 1;
            }
          }
          closeDropdown();
          return;
        }
      }

      // フィールド外クリチE��で閉じる（フィールド�E体�EクリチE��は既に処琁E��み�E�E
      if (!isInRect(fieldX, fieldY, fieldW, fieldH + listH)) {
        closeDropdown();
      }
    }

    // スチE�Eタスパネル冁E��リチE���E�試着UI�E�E
    if (activeTab_ == CodexTab::Characters && relative_x >= status_panel_.x &&
        relative_x < status_panel_.x + status_panel_.width &&
        relative_y >= status_panel_.y &&
        relative_y < status_panel_.y + status_panel_.height) {

      const float x0 = status_panel_.x + status_panel_.padding;
      float y0 = status_panel_.y + status_panel_.padding + kPanelHeaderH;
      const float btn = 28.0f;
      const float gap = 8.0f;
      const float labelW = 140.0f;
      const float fieldH = 28.0f;

      // 「試着�E�保存なし）」�E衁E
      y0 += status_panel_.line_height;

      // Lv [-][+]
      {
        const float rowY = y0;
        const float minusX = x0 + 220.0f;
        const float plusX = minusX + btn + gap;
        if (isInRect(minusX, rowY, btn, btn)) {
          tryOnState_.level = std::max(1, tryOnState_.level - 1);
          closeDropdown();
          return;
        }
        if (isInRect(plusX, rowY, btn, btn)) {
          tryOnState_.level = std::min(50, tryOnState_.level + 1);
          closeDropdown();
          return;
        }
      }
      y0 += status_panel_.line_height;

      // 「裁E��」ラベル衁E
      y0 += status_panel_.line_height;

      // 裁E��スロチE��: ドロチE�Eダウン選抁E
      for (int i = 0; i < 3; ++i) {
        const float rowY = y0;
        const float fieldX = x0 + labelW;
        const float fieldW =
            status_panel_.width - status_panel_.padding * 2.0f - labelW;
        if (isInRect(fieldX, rowY, fieldW, fieldH)) {
          dropdownKind_ = DropdownKind::EquipmentSlot;
          dropdownSlotIndex_ = i;
          dropdownScrollPx_ = 0.0f;
          return;
        }
        y0 += status_panel_.line_height;
      }

      // 「パチE��ブ」ラベル衁E
      y0 += status_panel_.line_height;

      // パッシチE ドロチE�Eダウン選抁E+ Lv[-][+]
      for (int i = 0; i < 3; ++i) {
        const float rowY = y0;
        const float minusX = status_panel_.x + status_panel_.width -
                             status_panel_.padding - (btn * 2.0f + gap);
        const float plusX = minusX + btn + gap;
        const float fieldX = x0 + labelW;
        const float fieldW = std::max(60.0f, minusX - gap - fieldX);

        if (isInRect(minusX, rowY, btn, btn)) {
          tryOnState_.passives[i].level =
              std::max(1, tryOnState_.passives[i].level - 1);
          closeDropdown();
          return;
        }
        if (isInRect(plusX, rowY, btn, btn)) {
          tryOnState_.passives[i].level =
              std::min(50, tryOnState_.passives[i].level + 1);
          closeDropdown();
          return;
        }
        if (isInRect(fieldX, rowY, fieldW, fieldH)) {
          dropdownKind_ = DropdownKind::PassiveSlot;
          dropdownSlotIndex_ = i;
          dropdownScrollPx_ = 0.0f;
          return;
        }
        y0 += status_panel_.line_height;
      }

      // スチE�Eタスパネル冁E�E「その他クリチE��」�EドロチE�Eダウン閉じめE
      if (dropdownKind_ != DropdownKind::None) {
        closeDropdown();
      }
    } else if ((activeTab_ == CodexTab::Equipment ||
                activeTab_ == CodexTab::Passives) &&
               relative_x >= status_panel_.x &&
               relative_x < status_panel_.x + status_panel_.width &&
               relative_y >= status_panel_.y &&
               relative_y < status_panel_.y + status_panel_.height) {
      // 裁E��/パッシチEↁE試着へ適用
      const auto *entry = GetSelectedEntry();
      if (!entry)
        return;

      auto hit = [&](float x, float y, float w, float h) {
        return (relative_x >= x && relative_x < x + w && relative_y >= y &&
                relative_y < y + h);
      };

      auto ensureTryOnTarget = [&]() {
        auto &chars = tabEntries_[TabIndex(CodexTab::Characters)];
        if (chars.empty())
          return;

        if (tryOnCharacterId_.empty()) {
          int idx = tabSelectedIndex_[TabIndex(CodexTab::Characters)];
          if (idx < 0 || idx >= static_cast<int>(chars.size()))
            idx = 0;
          tryOnCharacterId_ = chars[idx].id;
          if (ctx.gameplayDataAPI) {
            tryOnState_ =
                ctx.gameplayDataAPI->GetCharacterState(tryOnCharacterId_);
          } else {
            tryOnState_ = {};
            if (chars[idx].character) {
              tryOnState_.level =
                  std::max(1, chars[idx].character->default_level);
            }
          }
        }

        // キャラタブ選択を試着対象に合わせる�E�次フレームでtryOnState_が上書きされなぁE��ぁE���E�E
        for (int i = 0; i < static_cast<int>(chars.size()); ++i) {
          if (chars[i].id == tryOnCharacterId_) {
            tabSelectedIndex_[TabIndex(CodexTab::Characters)] = i;
            break;
          }
        }
        tryOnState_.unlocked = true;
      };

      ensureTryOnTarget();
      if (tryOnCharacterId_.empty())
        return;

      const float btn = 60.0f;
      const float gap = 10.0f;
      const float x0 = status_panel_.x + status_panel_.padding;
      const float y0 = status_panel_.y + status_panel_.height -
                       status_panel_.padding - 28.0f;

      const float applyW = 140.0f;
      const float applyH = 32.0f;
      const float applyX = status_panel_.x + status_panel_.width -
                           status_panel_.padding - applyW;
      const float applyY = status_panel_.y + status_panel_.height -
                           status_panel_.padding - applyH;
      if (isInRect(applyX, applyY, applyW, applyH)) {
        int slotIndex = 0;
        if (activeTab_ == CodexTab::Equipment && entry->equipment) {
          for (int i = 0; i < 3; ++i) {
            if (tryOnState_.equipment[i].empty()) {
              slotIndex = i;
              break;
            }
          }
          tryOnState_.equipment[slotIndex] = entry->equipment->id;
          SwitchTab(CodexTab::Characters);
          return;
        }
        if (activeTab_ == CodexTab::Passives && entry->passive) {
          for (int i = 0; i < 3; ++i) {
            if (tryOnState_.passives[i].id.empty()) {
              slotIndex = i;
              break;
            }
          }
          tryOnState_.passives[slotIndex].id = entry->passive->id;
          tryOnState_.passives[slotIndex].level =
              std::max(1, tryOnState_.passives[slotIndex].level);
          SwitchTab(CodexTab::Characters);
          return;
        }
      }

      for (int i = 0; i < 3; ++i) {
        const float bx = x0 + (btn + gap) * static_cast<float>(i);
        if (!isInRect(bx, y0, btn, 28.0f))
          continue;

        if (activeTab_ == CodexTab::Equipment && entry->equipment) {
          tryOnState_.equipment[i] = entry->equipment->id;
          SwitchTab(CodexTab::Characters);
          return;
        }
        if (activeTab_ == CodexTab::Passives && entry->passive) {
          tryOnState_.passives[i].id = entry->passive->id;
          tryOnState_.passives[i].level =
              std::max(1, tryOnState_.passives[i].level);
          SwitchTab(CodexTab::Characters);
          return;
        }
      }
    }
  }

  // スクロール処琁E
  float wheel_delta = ctx.inputAPI ? ctx.inputAPI->GetMouseWheelMove() : 0.0f;
  if (wheel_delta != 0.0f) {
    // ドロチE�Eダウン優先スクロール�E�キャラタブ！E
    if (activeTab_ == CodexTab::Characters &&
        dropdownKind_ != DropdownKind::None && ctx.gameplayDataAPI &&
        dropdownSlotIndex_ >= 0 && dropdownSlotIndex_ < 3) {

      const float x0 = status_panel_.x + status_panel_.padding;
      const float y0 = status_panel_.y + status_panel_.padding + kPanelHeaderH;
      const float labelW = 140.0f;
      const float fieldH = 28.0f;

      float fieldX = 0.0f, fieldY = 0.0f, fieldW = 0.0f;
      if (dropdownKind_ == DropdownKind::EquipmentSlot) {
        fieldY = y0 + status_panel_.line_height *
                          (2.0f + static_cast<float>(dropdownSlotIndex_));
        fieldX = x0 + labelW;
        fieldW = status_panel_.width - status_panel_.padding * 2.0f - labelW;
      } else if (dropdownKind_ == DropdownKind::PassiveSlot) {
        const float btn = 28.0f;
        const float gap = 8.0f;
        const float minusX = status_panel_.x + status_panel_.width -
                             status_panel_.padding - (btn * 2.0f + gap);
        fieldY = y0 + status_panel_.line_height *
                          (6.0f + static_cast<float>(dropdownSlotIndex_));
        fieldX = x0 + labelW;
        fieldW = std::max(60.0f, minusX - gap - fieldX);
      }

      const float listX = fieldX;
      const float listY = fieldY + fieldH;
      const float listW = fieldW;
      const float listH = 220.0f;

      if (relative_x >= listX && relative_x < listX + listW &&
          relative_y >= listY && relative_y < listY + listH) {
        dropdownScrollPx_ -= wheel_delta * 28.0f * 2.0f;
        if (dropdownScrollPx_ < 0.0f)
          dropdownScrollPx_ = 0.0f;
        // 上限クランプ�E描画側で行う
        return;
      }
    }

    const bool overInfo = (relative_x >= info_panel_.x &&
                           relative_x < info_panel_.x + info_panel_.width &&
                           relative_y >= info_panel_.y &&
                           relative_y < info_panel_.y + info_panel_.height);

    // キャラタチE ビューポ�Eト上ではズーム
    const bool overViewport =
        (relative_x >= character_viewport_.x &&
         relative_x < character_viewport_.x + character_viewport_.width &&
         relative_y >= character_viewport_.y &&
         relative_y < character_viewport_.y + character_viewport_.height);

    const float listX = list_panel_.x + contentOffsetX;
    const float listY = list_panel_.y + panelStartY;
    const bool overList =
        (mouse_pos.x >= listX && mouse_pos.x < listX + list_panel_.width &&
         mouse_pos.y >= listY && mouse_pos.y < listY + list_panel_.height);

    if (overList) {
      OnListScroll(static_cast<int>(wheel_delta));
    } else if (activeTab_ == CodexTab::Characters && overViewport) {
      const float step = 0.08f;
      if (wheel_delta > 0.0f) {
        character_viewport_.zoom =
            std::min(2.0f, character_viewport_.zoom + step);
      } else {
        character_viewport_.zoom =
            std::max(0.5f, character_viewport_.zoom - step);
      }
    } else if (overInfo) {
      // 右説明パネルのスクロール�E�描画側でクランプ！E
      infoScrollPx_ -= wheel_delta * info_panel_.line_height * 2.0f;
    }
  }
}

void CodexOverlay::Render(SharedContext &ctx) {
  if (!isInitialized_) {
    return;
  }

  LayoutPanels();
  EnsureEntriesLoaded(ctx);

  // コンチE��チE��域のオフセチE���E��EチE��ーとタブバーの間！E
  const float contentOffsetX = kContentOffsetX;
  const float contentOffsetY = kContentOffsetY;
  const float tabBarHeight = kTabBarHeight;
  const float tabBarGap = kTabBarGap;
  const float panelStartY =
      contentOffsetY + tabBarHeight + tabBarGap; // タブバーの下に配置

  // 描画前に座標をオフセチE��
  float saved_list_x = list_panel_.x;
  float saved_list_y = list_panel_.y;
  float saved_viewport_x = character_viewport_.x;
  float saved_viewport_y = character_viewport_.y;
  float saved_status_x = status_panel_.x;
  float saved_status_y = status_panel_.y;
  float saved_info_x = info_panel_.x;
  float saved_info_y = info_panel_.y;

  list_panel_.x += contentOffsetX;
  list_panel_.y += panelStartY;
  character_viewport_.x += contentOffsetX;
  character_viewport_.y += panelStartY;
  status_panel_.x += contentOffsetX;
  status_panel_.y += panelStartY;
  info_panel_.x += contentOffsetX;
  info_panel_.y += panelStartY;

  RenderTabBar(contentOffsetX, contentOffsetY);
  RenderListPanel();
  RenderCharacterViewport();
  RenderStatusPanel(ctx);
  RenderInfoPanel();

  // 座標を允E��戻ぁE
  list_panel_.x = saved_list_x;
  list_panel_.y = saved_list_y;
  character_viewport_.x = saved_viewport_x;
  character_viewport_.y = saved_viewport_y;
  status_panel_.x = saved_status_x;
  status_panel_.y = saved_status_y;
  info_panel_.x = saved_info_x;
  info_panel_.y = saved_info_y;
}

void CodexOverlay::Shutdown() {
  if (!isInitialized_) {
    return;
  }

  tabEntries_ = {};
  isInitialized_ = false;
  systemAPI_ = nullptr;
  LOG_INFO("CodexOverlay shutdown");
}

// ========== 描画ヘルパ�E ==========

void CodexOverlay::LayoutPanels() {
  // コンチE��チE��域: 1880x900�E�Epdate冁E�E相対座標と一致�E�E
  // タブバー領域: y=10..50
  constexpr float margin = kTabBarInset;
  constexpr float tabBarH = kTabBarHeight;
  constexpr float gap = kTabBarGap;
  const bool isDenseTab = (activeTab_ != CodexTab::Characters);

  const float yTop = margin + tabBarH + gap; // 70
  const float yBottom = 900.0f - margin;     // 890
  const float bodyH = std::max(0.0f, yBottom - yTop);

  list_panel_.x = margin;
  list_panel_.y = yTop;
  list_panel_.width = 520.0f;
  list_panel_.height = bodyH;
  list_panel_.padding = isDenseTab ? 12.0f : 16.0f;
  list_panel_.card_width = isDenseTab ? 140.0f : 150.0f;
  list_panel_.card_height = isDenseTab ? 100.0f : 120.0f;
  list_panel_.card_gap = isDenseTab ? 8.0f : 12.0f;

  const float rightW = 520.0f;
  const float centerX = list_panel_.x + list_panel_.width + 20.0f;
  const float centerW = std::max(0.0f, 1880.0f - margin * 2.0f -
                                           list_panel_.width - rightW - 40.0f);
  const float rightX = centerX + centerW + 20.0f;

  const float previewH = bodyH * (isDenseTab ? 0.44f : 0.52f);
  const float infoH = std::max(0.0f, bodyH - previewH - gap);

  character_viewport_.x = centerX;
  character_viewport_.y = yTop;
  character_viewport_.width = centerW;
  character_viewport_.height = previewH;

  info_panel_.x = centerX;
  info_panel_.y = character_viewport_.y + character_viewport_.height + gap;
  info_panel_.width = centerW;
  info_panel_.height = infoH;
  info_panel_.padding = isDenseTab ? 16.0f : 20.0f;
  info_panel_.line_height = isDenseTab ? 30.0f : 36.0f;
  info_panel_.font_size = isDenseTab ? 20 : 22;

  status_panel_.x = rightX;
  status_panel_.y = yTop;
  status_panel_.width = rightW;
  status_panel_.height = bodyH;
  status_panel_.padding = isDenseTab ? 14.0f : 20.0f;
  status_panel_.line_height = isDenseTab ? 28.0f : 34.0f;
  status_panel_.font_size = isDenseTab ? 22 : 24;
}

void CodexOverlay::RenderTabBar(float offsetX, float offsetY) {
  // 入力�E�E�Epdate�E��E contentOffset
  // を引いた「コンチE��チE��標系」で判定してぁE��ため、E 描画側も同じく
  // contentOffset を足して表示位置を合わせる、E
  const float x = offsetX + kTabBarInset;
  const float y = offsetY + kTabBarInset + kTabBarYOffset;
  const float w = kTabButtonWidth;
  const float h = kTabBarHeight;
  const float gap = kTabButtonGap;

  auto drawTab = [&](const std::string &label, CodexTab tab, float bx) {
    const bool active = (activeTab_ == tab);
    const Color bg = active ? ui::OverlayColors::CARD_BG_SELECTED
                            : ui::OverlayColors::PANEL_BG_PRIMARY;
    const Color border = active ? ui::OverlayColors::BORDER_BLUE
                                : ui::OverlayColors::BORDER_DEFAULT;
    const Color text =
        active ? ui::OverlayColors::TEXT_BLUE : ui::OverlayColors::TEXT_PRIMARY;

    systemAPI_->Render().DrawRectangle(bx, y, w, h, bg);
    systemAPI_->Render().DrawRectangleLines(bx, y, w, h, 2.0f, border);

    Vector2 sz = systemAPI_->Render().MeasureTextDefault(label, 22.0f);
    systemAPI_->Render().DrawTextDefault(label, bx + (w - sz.x) * 0.5f,
                                         y + (h - sz.y) * 0.5f, 22.0f, text);
  };

  drawTab("キャラ", CodexTab::Characters, x);
  drawTab("装備", CodexTab::Equipment, x + (w + gap) * 1.0f);
  drawTab("パッシブ", CodexTab::Passives, x + (w + gap) * 2.0f);
}

void CodexOverlay::RenderListPanel() {
  const float x = list_panel_.x;
  const float y = list_panel_.y;
  const float w = list_panel_.width;
  const float h = list_panel_.height;

  systemAPI_->Render().DrawRectangle(x, y, w, h,
                                     ui::OverlayColors::PANEL_BG_SECONDARY);
  systemAPI_->Render().DrawRectangleLines(x, y, w, h, 2.0f,
                                          ui::OverlayColors::BORDER_DEFAULT);

  // ヘッダ
  systemAPI_->Render().DrawRectangle(x, y, w, kPanelHeaderH,
                                     ui::OverlayColors::PANEL_BG_PRIMARY);
  systemAPI_->Render().DrawRectangleLines(x, y, w, kPanelHeaderH, 1.0f,
                                          ui::OverlayColors::BORDER_DEFAULT);
  std::string title = "一覧";
  if (activeTab_ == CodexTab::Characters)
    title = "キャラクター";
  else if (activeTab_ == CodexTab::Equipment)
    title = "装備";
  else if (activeTab_ == CodexTab::Passives)
    title = "パッシブ";
  systemAPI_->Render().DrawTextDefault(title, x + 12.0f, y + 6.0f, 20.0f,
                                       ui::OverlayColors::TEXT_PRIMARY);

  const float innerX = x + list_panel_.padding;
  const float innerY = y + kPanelHeaderH + list_panel_.padding;
  const float innerW = w - list_panel_.padding * 2.0f;
  const float innerH = h - kPanelHeaderH - list_panel_.padding * 2.0f;

  const int columns =
      std::max(1, static_cast<int>(std::floor(
                      (innerW + list_panel_.card_gap) /
                      (list_panel_.card_width + list_panel_.card_gap))));
  const int visibleRows =
      std::max(1, static_cast<int>(std::floor(
                      (innerH + list_panel_.card_gap) /
                      (list_panel_.card_height + list_panel_.card_gap))));

  const int ti = TabIndex(activeTab_);
  const auto &entries = tabEntries_[ti];
  const int totalItems = static_cast<int>(entries.size());
  const int totalRows = (totalItems + columns - 1) / columns;
  const int startRow = std::max(
      0, std::min(tabScrollOffset_[ti], std::max(0, totalRows - visibleRows)));
  const int endRow = std::min(totalRows, startRow + visibleRows);

  for (int row = startRow; row < endRow; ++row) {
    for (int col = 0; col < columns; ++col) {
      const int index = row * columns + col;
      if (index >= totalItems)
        break;

      const float cardX =
          innerX + col * (list_panel_.card_width + list_panel_.card_gap);
      const float cardY = innerY + (row - startRow) * (list_panel_.card_height +
                                                       list_panel_.card_gap);
      const bool selected = (index == tabSelectedIndex_[ti]);
      const Color bg = selected ? ui::OverlayColors::CARD_BG_SELECTED
                                : ui::OverlayColors::CARD_BG_NORMAL;
      const Color border = selected ? ui::OverlayColors::BORDER_BLUE
                                    : ui::OverlayColors::BORDER_DEFAULT;

      systemAPI_->Render().DrawRectangle(cardX, cardY, list_panel_.card_width,
                                         list_panel_.card_height, bg);
      systemAPI_->Render().DrawRectangleLines(
          cardX, cardY, list_panel_.card_width, list_panel_.card_height, 2.0f,
          border);

      const auto &entry = entries[index];
      if (entry.type == CodexEntry::Type::Character && entry.character &&
          !entry.character->icon_path.empty()) {
        void *texturePtr =
            systemAPI_->Resource().GetTexture(entry.character->icon_path);
        if (texturePtr) {
          Texture2D *texture = static_cast<Texture2D *>(texturePtr);
          if (texture && texture->id != 0) {
            Rectangle src{0.0f, 0.0f, static_cast<float>(texture->width),
                          static_cast<float>(texture->height)};
            const float pad = 6.0f;
            const float maxW =
                std::max(0.0f, list_panel_.card_width - pad * 2.0f);
            const float maxH =
                std::max(0.0f, list_panel_.card_height - pad * 2.0f - 20.0f);
            const float scale =
                std::min(maxW / static_cast<float>(texture->width),
                         maxH / static_cast<float>(texture->height));
            const float drawW = static_cast<float>(texture->width) * scale;
            const float drawH = static_cast<float>(texture->height) * scale;
            Rectangle dst{cardX + (list_panel_.card_width - drawW) * 0.5f,
                          cardY + pad, drawW, drawH};
            systemAPI_->Render().DrawTexturePro(*texture, src, dst,
                                                {0.0f, 0.0f}, 0.0f, WHITE);
          }
        }
      }

      const float labelY = cardY + list_panel_.card_height - 22.0f;
      systemAPI_->Render().DrawTextDefault(
          entry.name, cardX + 6.0f, labelY, 18.0f,
          entry.is_discovered ? ui::OverlayColors::TEXT_PRIMARY
                              : ui::OverlayColors::TEXT_MUTED);
    }
  }

  if (totalRows > visibleRows) {
    const float scrollBarW = 8.0f;
    const float scrollInset = 4.0f;
    const float scrollBarX = x + w - scrollBarW - scrollInset;
    const float scrollBarY = innerY;
    const float scrollBarH = innerH;
    const float thumbH = std::max(
        28.0f, scrollBarH * (static_cast<float>(visibleRows) / totalRows));
    const float maxScroll = std::max(1, totalRows - visibleRows);
    const float t =
        static_cast<float>(startRow) / static_cast<float>(maxScroll);
    const float thumbY = scrollBarY + (scrollBarH - thumbH) * t;
    systemAPI_->Render().DrawRectangle(scrollBarX, scrollBarY, scrollBarW,
                                       scrollBarH,
                                       ui::OverlayColors::PANEL_BG_PRIMARY);
    systemAPI_->Render().DrawRectangle(scrollBarX, thumbY, scrollBarW, thumbH,
                                       ui::OverlayColors::BORDER_BLUE);
  }
}

void CodexOverlay::RenderCharacterViewport() {
  // 背景
  systemAPI_->Render().DrawRectangle(
      character_viewport_.x, character_viewport_.y, character_viewport_.width,
      character_viewport_.height, ui::OverlayColors::PANEL_BG_SECONDARY);

  // 枠緁E
  systemAPI_->Render().DrawRectangleLines(
      character_viewport_.x, character_viewport_.y, character_viewport_.width,
      character_viewport_.height, 2.0f, ui::OverlayColors::BORDER_DEFAULT);

  // ヘッダ
  systemAPI_->Render().DrawRectangle(
      character_viewport_.x, character_viewport_.y, character_viewport_.width,
      kPanelHeaderH, ui::OverlayColors::PANEL_BG_SECONDARY);
  systemAPI_->Render().DrawRectangleLines(
      character_viewport_.x, character_viewport_.y, character_viewport_.width,
      kPanelHeaderH, 1.0f, ui::OverlayColors::BORDER_DEFAULT);
  systemAPI_->Render().DrawTextDefault(
      "プレビュー", character_viewport_.x + 10.0f, character_viewport_.y + 6.0f,
      20.0f, ui::OverlayColors::TEXT_PRIMARY);

  // エラー表示また�Eキャラクター表示
  if (character_viewport_.has_error) {
    systemAPI_->Render().DrawTextDefault(
        "エラー: " + character_viewport_.error_message,
        character_viewport_.x + 20.0f,
        character_viewport_.y + character_viewport_.height / 2.0f - 20.0f,
        20.0f, RED);
  } else {
    if (activeTab_ == CodexTab::Characters) {
      const entities::Character *selected = GetSelectedCharacter();
      if (!selected)
        return;
      // チE��スチャ読み込み試行（エラーハンドリング付き�E�E
      try {
        const entities::Character::SpriteInfo *sprite_info = nullptr;
        if (character_viewport_.current_animation == AnimationType::Move) {
          sprite_info = &selected->move_sprite;
        } else {
          sprite_info = &selected->attack_sprite;
        }

        if (sprite_info && !sprite_info->sheet_path.empty() &&
            sprite_info->frame_count > 0) {
          void *texturePtr =
              systemAPI_->Resource().GetTexture(sprite_info->sheet_path);
          if (texturePtr) {
            Texture2D *texture = static_cast<Texture2D *>(texturePtr);

            // 現在のフレームのソース矩形を計箁E
            int frame_x =
                character_viewport_.animation_frame * sprite_info->frame_width;
            Rectangle sourceRect = {
                static_cast<float>(frame_x), 0.0f,
                static_cast<float>(sprite_info->frame_width),
                static_cast<float>(sprite_info->frame_height)};

            // 描画可能領域�E��EチE��+操作バーを除外！E
            constexpr float ctrlH = 32.0f;
            constexpr float ctrlPad = 8.0f;
            const float reservedBottom = ctrlH + ctrlPad * 2.0f;
            const float drawX = character_viewport_.x;
            const float drawY = character_viewport_.y + kPanelHeaderH;
            const float drawW = character_viewport_.width;
            const float drawH =
                std::max(0.0f, character_viewport_.height - kPanelHeaderH -
                                   reservedBottom);

            // ビューポ�Eトに収まるよぁE��スケールを計算（アスペクト比を維持E��E
            float scaleX =
                (character_viewport_.width * 0.95f) / sprite_info->frame_width;
            float scaleY = (drawH * 0.95f) / sprite_info->frame_height;
            float scale = std::min(scaleX, scaleY) * character_viewport_.zoom;

            float scaledWidth = sprite_info->frame_width * scale;
            float scaledHeight = sprite_info->frame_height * scale;

            // 描画先矩形�E�ビューポ�Eト�E中央�E�E
            Rectangle destRect = {drawX + (drawW - scaledWidth) / 2.0f,
                                  drawY + (drawH - scaledHeight) / 2.0f,
                                  scaledWidth, scaledHeight};

            // スプライトシート�Eフレームを描画�E�ErawTextureProでスケール適用�E�E
            Vector2 origin = {0.0f, 0.0f}; // 左上を原点とする
            BeginScissorMode(static_cast<int>(drawX), static_cast<int>(drawY),
                             static_cast<int>(drawW), static_cast<int>(drawH));
            systemAPI_->Render().DrawTexturePro(*texture, sourceRect, destRect,
                                                origin, 0.0f, WHITE);
            EndScissorMode();
          } else {
            character_viewport_.has_error = true;
            character_viewport_.error_message = "チE��スチャが見つかりません";
          }
        } else {
          systemAPI_->Render().DrawTextDefault(
              selected->name, character_viewport_.x + 20.0f,
              character_viewport_.y + 20.0f, 24.0f, LIGHTGRAY);
        }
      } catch (const std::exception &e) {
        character_viewport_.has_error = true;
        character_viewport_.error_message = "リソース読み込みエラー";
        LOG_WARN("CodexOverlay: Resource load error: {}", e.what());
      }
    } else {
      const auto *entry = GetSelectedEntry();
      if (!entry)
        return;
      systemAPI_->Render().DrawTextDefault(
          entry->name, character_viewport_.x + 20.0f,
          character_viewport_.y + kPanelHeaderH + 20.0f, 26.0f,
          ui::OverlayColors::TEXT_PRIMARY);
    }
  }

  // ===== ビューア操作バー�E�キャラタブ�Eみ�E�E====
  if (activeTab_ == CodexTab::Characters) {
    const float ctrlH = 32.0f;
    const float ctrlPad = 8.0f;
    const float ctrlY =
        character_viewport_.y + character_viewport_.height - ctrlH - ctrlPad;
    const float ctrlX = character_viewport_.x + 10.0f;
    const float ctrlW = character_viewport_.width - 20.0f;

    // 背景
    systemAPI_->Render().DrawRectangle(ctrlX - 6.0f, ctrlY - 6.0f,
                                       ctrlW + 12.0f, ctrlH + 12.0f,
                                       ui::OverlayColors::PANEL_BG_PRIMARY);

    const float btnH = ctrlH;
    const float btnW = 64.0f;
    const float btnGap = 8.0f;
    float bx = ctrlX;
    const float by = ctrlY;

    auto drawBtn = [&](const std::string &label, bool active) {
      const Color bg = active ? ui::OverlayColors::BUTTON_BLUE
                              : ui::OverlayColors::CARD_BG_NORMAL;
      const Color border = active ? ui::OverlayColors::BORDER_BLUE
                                  : ui::OverlayColors::BORDER_DEFAULT;
      const Color text = ui::OverlayColors::TEXT_PRIMARY;
      systemAPI_->Render().DrawRectangle(bx, by, btnW, btnH, bg);
      systemAPI_->Render().DrawRectangleLines(bx, by, btnW, btnH, 1.0f, border);
      Vector2 sz = systemAPI_->Render().MeasureTextDefault(label, 18.0f);
      systemAPI_->Render().DrawTextDefault(label, bx + (btnW - sz.x) * 0.5f,
                                           by + (btnH - sz.y) * 0.5f, 18.0f,
                                           text);
      bx += btnW + btnGap;
    };

    drawBtn("Move",
            character_viewport_.current_animation == AnimationType::Move);
    drawBtn("Atk",
            character_viewport_.current_animation == AnimationType::Attack);
    drawBtn(character_viewport_.is_paused ? "Play" : "Pause", false);
    drawBtn("<", false);
    drawBtn(">", false);
    drawBtn("0.5x", character_viewport_.speed_multiplier == 0.5f);
    drawBtn("1x", character_viewport_.speed_multiplier == 1.0f);
    drawBtn("2x", character_viewport_.speed_multiplier == 2.0f);
    drawBtn("-", false);
    drawBtn("+", false);
  }
}

void CodexOverlay::RenderStatusPanel(SharedContext &ctx) {
  // 背景
  systemAPI_->Render().DrawRectangle(status_panel_.x, status_panel_.y,
                                     status_panel_.width, status_panel_.height,
                                     ui::OverlayColors::PANEL_BG_SECONDARY);
  systemAPI_->Render().DrawRectangleLines(
      status_panel_.x, status_panel_.y, status_panel_.width,
      status_panel_.height, 2.0f, ui::OverlayColors::BORDER_DEFAULT);

  // ヘッダ
  systemAPI_->Render().DrawRectangle(status_panel_.x, status_panel_.y,
                                     status_panel_.width, kPanelHeaderH,
                                     ui::OverlayColors::PANEL_BG_SECONDARY);
  systemAPI_->Render().DrawRectangleLines(
      status_panel_.x, status_panel_.y, status_panel_.width, kPanelHeaderH,
      1.0f, ui::OverlayColors::BORDER_DEFAULT);
  std::string headerTitle = "試着・統計";
  if (activeTab_ == CodexTab::Equipment)
    headerTitle = "装備情報";
  else if (activeTab_ == CodexTab::Passives)
    headerTitle = "パッシブ情報";
  systemAPI_->Render().DrawTextDefault(headerTitle, status_panel_.x + 10.0f,
                                       status_panel_.y + 6.0f, 20.0f,
                                       ui::OverlayColors::TEXT_PRIMARY);

  if (activeTab_ != CodexTab::Characters) {
    const auto *entry = GetSelectedEntry();
    if (!entry)
      return;

    float x = status_panel_.x + status_panel_.padding;
    float y = status_panel_.y + status_panel_.padding + kPanelHeaderH;

    if (entry->type == CodexEntry::Type::Equipment && entry->equipment) {
      const auto *eq = entry->equipment;
      systemAPI_->Render().DrawTextDefault("装備ボーナス", x, y, 24.0f,
                                           ui::OverlayColors::TEXT_PRIMARY);
      y += status_panel_.line_height;
      systemAPI_->Render().DrawTextDefault(
          "ATK +" + std::to_string(static_cast<int>(eq->attack_bonus)), x, y,
          22.0f, ui::OverlayColors::TEXT_SECONDARY);
      y += status_panel_.line_height;
      systemAPI_->Render().DrawTextDefault(
          "DEF +" + std::to_string(static_cast<int>(eq->defense_bonus)), x, y,
          22.0f, ui::OverlayColors::TEXT_SECONDARY);
      y += status_panel_.line_height;
      systemAPI_->Render().DrawTextDefault(
          "HP  +" + std::to_string(static_cast<int>(eq->hp_bonus)), x, y, 22.0f,
          ui::OverlayColors::TEXT_SECONDARY);
    } else if (entry->type == CodexEntry::Type::Passive && entry->passive) {
      const auto *ps = entry->passive;
      systemAPI_->Render().DrawTextDefault("パッシブ効果", x, y, 24.0f,
                                           ui::OverlayColors::TEXT_PRIMARY);
      y += status_panel_.line_height;
      systemAPI_->Render().DrawTextDefault(
          "value: " + std::to_string(ps->value), x, y, 22.0f,
          ui::OverlayColors::TEXT_SECONDARY);
    }

    // 試着へ適用ボタン
    {
      const float btnW = 60.0f;
      const float btnH = 28.0f;
      const float gap = 10.0f;
      const float by =
          status_panel_.y + status_panel_.height - status_panel_.padding - btnH;
      float bx = x;

      systemAPI_->Render().DrawTextDefault("試着へ", x, by - 24.0f, 20.0f,
                                           ui::OverlayColors::TEXT_MUTED);

      auto drawSmallBtn = [&](const std::string &label) {
        systemAPI_->Render().DrawRectangle(bx, by, btnW, btnH,
                                           ui::OverlayColors::CARD_BG_NORMAL);
        systemAPI_->Render().DrawRectangleLines(
            bx, by, btnW, btnH, 1.0f, ui::OverlayColors::BORDER_DEFAULT);
        Vector2 sz = systemAPI_->Render().MeasureTextDefault(label, 18.0f);
        systemAPI_->Render().DrawTextDefault(label, bx + (btnW - sz.x) * 0.5f,
                                             by + (btnH - sz.y) * 0.5f, 18.0f,
                                             ui::OverlayColors::TEXT_PRIMARY);
        bx += btnW + gap;
      };
      drawSmallBtn("S1");
      drawSmallBtn("S2");
      drawSmallBtn("S3");
    }

    // 即時試着ボタン
    {
      const float btnW = 140.0f;
      const float btnH = 32.0f;
      const float bx =
          status_panel_.x + status_panel_.width - status_panel_.padding - btnW;
      const float by =
          status_panel_.y + status_panel_.height - status_panel_.padding - btnH;
      systemAPI_->Render().DrawRectangle(bx, by, btnW, btnH,
                                         ui::OverlayColors::BUTTON_BLUE);
      systemAPI_->Render().DrawRectangleLines(bx, by, btnW, btnH, 1.0f,
                                              ui::OverlayColors::BORDER_BLUE);
      Vector2 sz = systemAPI_->Render().MeasureTextDefault("試着する", 20.0f);
      systemAPI_->Render().DrawTextDefault(
          "試着する", bx + (btnW - sz.x) * 0.5f, by + (btnH - sz.y) * 0.5f,
          20.0f, ui::OverlayColors::TEXT_DARK);
    }
    return;
  }

  const entities::Character *selected = GetSelectedCharacter();
  if (!selected)
    return;

  // スチE�Eタス表示
  float x = status_panel_.x + status_panel_.padding;
  float y = status_panel_.y + status_panel_.padding + kPanelHeaderH;

  // --- ヘルチE---
  auto fmtPercent = [](float ratio) -> std::string {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << ratio;
    return oss.str();
  };
  auto safePct = [](float base, float final) -> float {
    if (std::abs(base) < 1e-6f)
      return 0.0f;
    return (final / base - 1.0f) * 100.0f;
  };
  auto drawKV = [&](const std::string &label, const std::string &value,
                    float rowY) {
    systemAPI_->Render().DrawTextDefault(label, x, rowY, 22.0f,
                                         ui::OverlayColors::TEXT_SECONDARY);
    Vector2 sz = systemAPI_->Render().MeasureTextDefault(value, 22.0f);
    systemAPI_->Render().DrawTextDefault(
        value, x + status_panel_.width - status_panel_.padding * 2.0f - sz.x,
        rowY, 22.0f, ui::OverlayColors::TEXT_PRIMARY);
  };
  auto drawBtn = [&](float bx, float by, float w, float h,
                     const std::string &label) {
    systemAPI_->Render().DrawRectangle(bx, by, w, h,
                                       ui::OverlayColors::CARD_BG_NORMAL);
    systemAPI_->Render().DrawRectangleLines(bx, by, w, h, 1.0f,
                                            ui::OverlayColors::BORDER_DEFAULT);
    Vector2 sz = systemAPI_->Render().MeasureTextDefault(label, 18.0f);
    systemAPI_->Render().DrawTextDefault(label, bx + (w - sz.x) * 0.5f,
                                         by + (h - sz.y) * 0.5f, 18.0f,
                                         ui::OverlayColors::TEXT_PRIMARY);
  };

  // --- 試着UI（保存なし） --
  systemAPI_->Render().DrawTextDefault("試着（保存なし）", x, y, 24.0f,
                                       ui::OverlayColors::TEXT_PRIMARY);
  y += status_panel_.line_height;

  // Lv row
  {
    const float rowY = y;
    drawKV("Lv", std::to_string(std::max(1, std::min(50, tryOnState_.level))),
           rowY);
    const float btn = 28.0f;
    const float gap = 8.0f;
    const float bx = x + 220.0f;
    drawBtn(bx, rowY, btn, btn, "-");
    drawBtn(bx + btn + gap, rowY, btn, btn, "+");
    y += status_panel_.line_height;
  }

  // 装備3枠（ドロップダウン選択、X削除）
  systemAPI_->Render().DrawTextDefault("装備", x, y, 22.0f,
                                       ui::OverlayColors::TEXT_SECONDARY);
  y += status_panel_.line_height;
  const float labelW = 140.0f;
  const float fieldH = 28.0f;
  for (int i = 0; i < 3; ++i) {
    const float rowY = y;
    const std::string &eid = tryOnState_.equipment[i];
    std::string name = eid.empty() ? "なし" : eid;
    if (!eid.empty() && ctx.gameplayDataAPI) {
      const auto *eq = ctx.gameplayDataAPI->GetEquipment(eid);
      if (eq)
        name = eq->name;
    }
    systemAPI_->Render().DrawTextDefault("Slot" + std::to_string(i + 1), x,
                                         rowY, 22.0f,
                                         ui::OverlayColors::TEXT_SECONDARY);

    const float fieldX = x + labelW;
    const float fieldW =
        status_panel_.width - status_panel_.padding * 2.0f - labelW;
    const bool isOpen = (dropdownKind_ == DropdownKind::EquipmentSlot &&
                         dropdownSlotIndex_ == i);

    // フィールド背景
    systemAPI_->Render().DrawRectangle(fieldX, rowY, fieldW, fieldH,
                                       isOpen
                                           ? ui::OverlayColors::CARD_BG_SELECTED
                                           : ui::OverlayColors::CARD_BG_NORMAL);
    systemAPI_->Render().DrawRectangleLines(fieldX, rowY, fieldW, fieldH, 1.0f,
                                            ui::OverlayColors::BORDER_DEFAULT);

    // チE��スト（右寁E���E�E
    Vector2 nameSz = systemAPI_->Render().MeasureTextDefault(name, 20.0f);
    systemAPI_->Render().DrawTextDefault(
        name, fieldX + fieldW - nameSz.x - 8.0f,
        rowY + (fieldH - nameSz.y) * 0.5f, 20.0f,
        ui::OverlayColors::TEXT_PRIMARY);

    // ドロチE�Eダウン矢印�E�▼�E�E
    const float arrowW = 12.0f;
    const float arrowX = fieldX + fieldW - arrowW - 4.0f;
    const float arrowY = rowY + (fieldH - 8.0f) * 0.5f;
    systemAPI_->Render().DrawTextDefault("▼", arrowX, arrowY, 16.0f,
                                         ui::OverlayColors::TEXT_MUTED);

    y += status_panel_.line_height;
  }

  // パッシブ + Lv（ドロップダウン選択、X削除）
  systemAPI_->Render().DrawTextDefault("パッシブ", x, y, 22.0f,
                                       ui::OverlayColors::TEXT_SECONDARY);
  y += status_panel_.line_height;
  const float btn = 28.0f;
  const float gap = 8.0f;
  for (int i = 0; i < 3; ++i) {
    const float rowY = y;
    const std::string &pid = tryOnState_.passives[i].id;
    std::string name = pid.empty() ? "なし" : pid;
    if (!pid.empty() && ctx.gameplayDataAPI) {
      const auto *ps = ctx.gameplayDataAPI->GetPassiveSkill(pid);
      if (ps)
        name = ps->name;
    }
    const int plv = std::max(1, tryOnState_.passives[i].level);
    systemAPI_->Render().DrawTextDefault("Slot" + std::to_string(i + 1), x,
                                         rowY, 22.0f,
                                         ui::OverlayColors::TEXT_SECONDARY);

    const float minusX = status_panel_.x + status_panel_.width -
                         status_panel_.padding - (btn * 2.0f + gap);
    const float plusX = minusX + btn + gap;
    const float fieldX = x + labelW;
    const float fieldW = std::max(60.0f, minusX - gap - fieldX);
    const bool isOpen =
        (dropdownKind_ == DropdownKind::PassiveSlot && dropdownSlotIndex_ == i);

    // フィールド背景
    systemAPI_->Render().DrawRectangle(fieldX, rowY, fieldW, fieldH,
                                       isOpen
                                           ? ui::OverlayColors::CARD_BG_SELECTED
                                           : ui::OverlayColors::CARD_BG_NORMAL);
    systemAPI_->Render().DrawRectangleLines(fieldX, rowY, fieldW, fieldH, 1.0f,
                                            ui::OverlayColors::BORDER_DEFAULT);

    // チE��スト（右寁E���E�E
    const std::string displayText = name + " Lv" + std::to_string(plv);
    Vector2 textSz =
        systemAPI_->Render().MeasureTextDefault(displayText, 20.0f);
    systemAPI_->Render().DrawTextDefault(
        displayText, fieldX + fieldW - textSz.x - 8.0f,
        rowY + (fieldH - textSz.y) * 0.5f, 20.0f,
        ui::OverlayColors::TEXT_PRIMARY);

    // ドロチE�Eダウン矢印�E�▼�E�E
    const float arrowW = 12.0f;
    const float arrowX = fieldX + fieldW - arrowW - 4.0f;
    const float arrowY = rowY + (fieldH - 8.0f) * 0.5f;
    systemAPI_->Render().DrawTextDefault("▼", arrowX, arrowY, 16.0f,
                                         ui::OverlayColors::TEXT_MUTED);

    // Lv [-] [+]
    drawBtn(minusX, rowY, btn, btn, "-");
    drawBtn(plusX, rowY, btn, btn, "+");

    y += status_panel_.line_height;
  }

  // --- 統計（base/bonus/final + %増 + DPS/EHP/効率） --
  y += status_panel_.line_height * 0.25f;
  systemAPI_->Render().DrawTextDefault("統計", x, y, 24.0f,
                                       ui::OverlayColors::TEXT_PRIMARY);
  y += status_panel_.line_height;

  if (!ctx.gameplayDataAPI) {
    systemAPI_->Render().DrawTextDefault(
        "GameplayDataAPI がないため統計を計算できません", x, y, 20.0f,
        ui::OverlayColors::TEXT_MUTED);
    return;
  }

  const auto *itemPassiveManager =
      ctx.gameplayDataAPI ? ctx.gameplayDataAPI->GetItemPassiveManager()
                          : nullptr;
  if (!itemPassiveManager) {
    return;
  }
  const auto calc = entities::CharacterStatCalculator::Calculate(
      *selected, tryOnState_, *itemPassiveManager);
  const int cost = std::max(1, selected->cost);

  const float dps =
      (calc.attackSpan.final > 0.0f)
          ? (static_cast<float>(calc.attack.final) / calc.attackSpan.final)
          : 0.0f;
  constexpr float DEF_EHP_DIV = 100.0f;
  const float ehp =
      static_cast<float>(calc.hp.final) *
      (1.0f + static_cast<float>(calc.defense.final) / DEF_EHP_DIV);
  const float dpsPerCost = dps / static_cast<float>(cost);
  const float ehpPerCost = ehp / static_cast<float>(cost);

  auto drawStatLine = [&](const std::string &label, float base, float bonus,
                          float final) {
    std::ostringstream oss;
    oss << static_cast<int>(std::round(base)) << "  (+"
        << static_cast<int>(std::round(bonus)) << ")  => "
        << static_cast<int>(std::round(final));
    const float pct = safePct(base, final);
    oss << "  [" << (pct >= 0.0f ? "+" : "") << fmtPercent(pct) << "%]";
    drawKV(label, oss.str(), y);
    y += status_panel_.line_height;
  };

  drawStatLine("HP", static_cast<float>(calc.hp.base),
               static_cast<float>(calc.hp.bonus),
               static_cast<float>(calc.hp.final));
  drawStatLine("ATK", static_cast<float>(calc.attack.base),
               static_cast<float>(calc.attack.bonus),
               static_cast<float>(calc.attack.final));
  drawStatLine("DEF", static_cast<float>(calc.defense.base),
               static_cast<float>(calc.defense.bonus),
               static_cast<float>(calc.defense.final));
  drawStatLine("SPD", calc.moveSpeed.base, calc.moveSpeed.bonus,
               calc.moveSpeed.final);
  drawStatLine("RNG", calc.range.base, calc.range.bonus, calc.range.final);

  {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << calc.attackSpan.base << " ("
        << (calc.attackSpan.final >= calc.attackSpan.base ? "+" : "")
        << std::setprecision(2)
        << (calc.attackSpan.final - calc.attackSpan.base) << ") => "
        << std::setprecision(2) << calc.attackSpan.final;
    const float pct = safePct(calc.attackSpan.base, calc.attackSpan.final);
    oss << "  [" << (pct >= 0.0f ? "+" : "") << fmtPercent(pct) << "%]";
    drawKV("SPAN", oss.str(), y);
    y += status_panel_.line_height;
  }

  {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << dps;
    drawKV("DPS", oss.str(), y);
    y += status_panel_.line_height;
  }
  {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << ehp;
    drawKV("EHP", oss.str(), y);
    y += status_panel_.line_height;
  }
  {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << dpsPerCost;
    drawKV("DPS/COST", oss.str(), y);
    y += status_panel_.line_height;
  }
  {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << ehpPerCost;
    drawKV("EHP/COST", oss.str(), y);
    y += status_panel_.line_height;
  }

  // AttackType, EffectTypeを文字�Eに変換する関数
  auto attackTypeToString = [](entities::AttackType type) -> std::string {
    switch (type) {
    case entities::AttackType::Single:
      return "単体";
    case entities::AttackType::Range:
      return "範囲";
    case entities::AttackType::Line:
      return "直線";
    default:
      return "不明";
    }
  };

  auto effectTypeToString = [](entities::EffectType type) -> std::string {
    switch (type) {
    case entities::EffectType::Normal:
      return "通常";
    case entities::EffectType::Fire:
      return "炎";
    case entities::EffectType::Ice:
      return "氷";
    case entities::EffectType::Lightning:
      return "雷";
    case entities::EffectType::Heal:
      return "回復";
    default:
      return "不明";
    }
  };

  // ※ 旧「主要スチE��け」表示は、統計表示に置き換え済み�E�封E��皁E��
  // attackType/effectType 等も統合して表示�E�E

  // --- ドロチE�Eダウンリスト描画 ---
  if (dropdownKind_ != DropdownKind::None && dropdownSlotIndex_ >= 0 &&
      dropdownSlotIndex_ < 3 && ctx.gameplayDataAPI) {
    const float y0 = status_panel_.y + status_panel_.padding + kPanelHeaderH;
    const float labelW = 140.0f;
    const float fieldH = 28.0f;
    float fieldX = 0.0f, fieldY = 0.0f, fieldW = 0.0f;

    if (dropdownKind_ == DropdownKind::EquipmentSlot) {
      fieldY = y0 + status_panel_.line_height *
                        (2.0f + static_cast<float>(dropdownSlotIndex_));
      fieldX = x + labelW;
      fieldW = status_panel_.width - status_panel_.padding * 2.0f - labelW;
    } else if (dropdownKind_ == DropdownKind::PassiveSlot) {
      fieldY = y0 + status_panel_.line_height *
                        (6.0f + static_cast<float>(dropdownSlotIndex_));
      fieldX = x + labelW;
      const float btn = 28.0f;
      const float gap = 8.0f;
      const float minusX = status_panel_.x + status_panel_.width -
                           status_panel_.padding - (btn * 2.0f + gap);
      fieldW = std::max(60.0f, minusX - gap - fieldX);
    }

    const float itemH = 28.0f;
    const int maxVisible = 8;
    const float listH = std::min(static_cast<float>(maxVisible), 10.0f) * itemH;
    const float listY = fieldY + fieldH;

    // リスト背景
    systemAPI_->Render().DrawRectangle(fieldX, listY, fieldW, listH,
                                       ui::OverlayColors::PANEL_BG_SECONDARY);

    // スクロール可能なリスト（枠冁E��リチE�E�E�E
    BeginScissorMode(static_cast<int>(fieldX), static_cast<int>(listY),
                     static_cast<int>(fieldW), static_cast<int>(listH));
    std::vector<std::pair<std::string, std::string>> items; // (id, name)

    if (dropdownKind_ == DropdownKind::EquipmentSlot) {
      items.emplace_back("", "なし");
      const auto &allEq = ctx.gameplayDataAPI->GetAllEquipment();
      for (const auto *eq : allEq) {
        if (eq)
          items.emplace_back(eq->id, eq->name);
      }
    } else if (dropdownKind_ == DropdownKind::PassiveSlot) {
      items.emplace_back("", "なし");
      const auto &allPs = ctx.gameplayDataAPI->GetAllPassiveSkills();
      for (const auto *ps : allPs) {
        if (ps)
          items.emplace_back(ps->id, ps->name);
      }
    }

    const int totalItems = static_cast<int>(items.size());
    const int visibleStart =
        std::max(0, static_cast<int>(std::floor(dropdownScrollPx_ / itemH)));
    const int visibleEnd = std::min(totalItems, visibleStart + maxVisible);

    for (int i = visibleStart; i < visibleEnd; ++i) {
      const float itemY = listY + static_cast<float>(i - visibleStart) * itemH -
                          dropdownScrollPx_ +
                          static_cast<float>(visibleStart) * itemH;
      if (itemY < listY || itemY + itemH > listY + listH)
        continue;

      const bool isSelected =
          (dropdownKind_ == DropdownKind::EquipmentSlot &&
           tryOnState_.equipment[dropdownSlotIndex_] == items[i].first) ||
          (dropdownKind_ == DropdownKind::PassiveSlot &&
           tryOnState_.passives[dropdownSlotIndex_].id == items[i].first);

      systemAPI_->Render().DrawRectangle(
          fieldX, itemY, fieldW, itemH,
          isSelected ? ui::OverlayColors::CARD_BG_SELECTED
                     : ui::OverlayColors::CARD_BG_NORMAL);

      Vector2 textSz =
          systemAPI_->Render().MeasureTextDefault(items[i].second, 20.0f);
      systemAPI_->Render().DrawTextDefault(
          items[i].second, fieldX + 8.0f, itemY + (itemH - textSz.y) * 0.5f,
          20.0f, ui::OverlayColors::TEXT_PRIMARY);
    }

    // スクロールバ�E�E�忁E��時�E�E
    if (totalItems > maxVisible) {
      const float scrollBarW = 8.0f;
      const float scrollInset = 2.0f;
      const float scrollBarX = fieldX + fieldW - scrollBarW - scrollInset;
      const float scrollBarH = std::max(0.0f, listH - scrollInset * 2.0f);
      const float thumbH = scrollBarH * (static_cast<float>(maxVisible) /
                                         static_cast<float>(totalItems));
      const float thumbTravel = std::max(0.0f, scrollBarH - thumbH);
      const float thumbY = listY + scrollInset +
                           (dropdownScrollPx_ /
                            (static_cast<float>(totalItems) * itemH - listH)) *
                               thumbTravel;

      systemAPI_->Render().DrawRectangle(scrollBarX, listY + scrollInset,
                                         scrollBarW, scrollBarH,
                                         ui::OverlayColors::CARD_BG_NORMAL);
      systemAPI_->Render().DrawRectangle(scrollBarX, thumbY, scrollBarW, thumbH,
                                         ui::OverlayColors::BORDER_DEFAULT);
    }

    EndScissorMode();
    systemAPI_->Render().DrawRectangleLines(fieldX, listY, fieldW, listH, 2.0f,
                                            ui::OverlayColors::BORDER_DEFAULT);
  }
}

void CodexOverlay::RenderInfoPanel() {
  const auto *entry = GetSelectedEntry();
  if (!entry)
    return;

  // 背景
  systemAPI_->Render().DrawRectangle(info_panel_.x, info_panel_.y,
                                     info_panel_.width, info_panel_.height,
                                     ui::OverlayColors::PANEL_BG_SECONDARY);

  // 枠緁E
  systemAPI_->Render().DrawRectangleLines(
      info_panel_.x, info_panel_.y, info_panel_.width, info_panel_.height, 2.0f,
      ui::OverlayColors::BORDER_DEFAULT);

  // ヘッダ
  systemAPI_->Render().DrawRectangle(info_panel_.x, info_panel_.y,
                                     info_panel_.width, kPanelHeaderH,
                                     ui::OverlayColors::PANEL_BG_SECONDARY);
  systemAPI_->Render().DrawRectangleLines(
      info_panel_.x, info_panel_.y, info_panel_.width, kPanelHeaderH, 1.0f,
      ui::OverlayColors::BORDER_DEFAULT);
  systemAPI_->Render().DrawTextDefault("説明", info_panel_.x + 10.0f,
                                       info_panel_.y + 6.0f, 20.0f,
                                       ui::OverlayColors::TEXT_PRIMARY);

  float x = info_panel_.x + info_panel_.padding;
  float y = info_panel_.y + info_panel_.padding + kPanelHeaderH;

  // 説明文
  if (!entry->description.empty()) {
    const float fontSize = static_cast<float>(info_panel_.font_size);
    const float maxWidth = info_panel_.width - info_panel_.padding * 2.0f;

    // キャチE��ュキー�E�タチEID/説明長/幁E��E
    const std::string key = std::to_string(static_cast<int>(activeTab_)) + ":" +
                            entry->id + ":" +
                            std::to_string(entry->description.size());
    if (infoCachedKey_ != key ||
        std::abs(infoCachedMaxWidth_ - maxWidth) > 0.5f) {
      infoCachedKey_ = key;
      infoCachedMaxWidth_ = maxWidth;
      infoWrappedLines_.clear();

      // 幁E�Eースで折り返し�E�ETF-8墁E��を維持E��E
      std::string line;
      line.reserve(64);
      size_t i = 0;
      while (i < entry->description.size()) {
        // 改行�E強制折り返し
        if (entry->description[i] == '\n') {
          infoWrappedLines_.push_back(line);
          line.clear();
          ++i;
          continue;
        }
        const size_t next = Utf8Next(entry->description, i);
        const std::string cp = entry->description.substr(i, next - i);

        std::string candidate = line;
        candidate += cp;

        const Vector2 sz =
            systemAPI_->Render().MeasureTextDefault(candidate, fontSize);
        if (sz.x > maxWidth && !line.empty()) {
          infoWrappedLines_.push_back(line);
          line = cp;
        } else {
          line = std::move(candidate);
        }
        i = next;
      }
      if (!line.empty()) {
        infoWrappedLines_.push_back(line);
      }
    }

    // スクロール�E�クランプ！E
    const float availableH =
        info_panel_.height - info_panel_.padding * 2.0f - kPanelHeaderH;
    const float totalH =
        static_cast<float>(infoWrappedLines_.size()) * info_panel_.line_height;
    const float maxScroll = std::max(0.0f, totalH - availableH);
    if (infoScrollPx_ < 0.0f)
      infoScrollPx_ = 0.0f;
    if (infoScrollPx_ > maxScroll)
      infoScrollPx_ = maxScroll;

    BeginScissorMode(static_cast<int>(x), static_cast<int>(y),
                     static_cast<int>(maxWidth), static_cast<int>(availableH));

    float currentY = y - infoScrollPx_;
    for (const auto &ln : infoWrappedLines_) {
      if (currentY + info_panel_.line_height < y) {
        currentY += info_panel_.line_height;
        continue;
      }
      if (currentY > y + availableH) {
        break;
      }
      systemAPI_->Render().DrawTextDefault(ln, x, currentY, fontSize,
                                           ui::OverlayColors::TEXT_PRIMARY);
      currentY += info_panel_.line_height;
    }
    EndScissorMode();

    // スクロールバ�E�E�簡易！E
    if (maxScroll > 0.0f) {
      const float barW = 6.0f;
      const float barX =
          info_panel_.x + info_panel_.width - info_panel_.padding - barW;
      const float barY = y;
      const float barH = availableH;
      const float thumbH = std::max(24.0f, barH * (availableH / totalH));
      const float t = (maxScroll > 0.0f) ? (infoScrollPx_ / maxScroll) : 0.0f;
      const float thumbY = barY + (barH - thumbH) * t;
      systemAPI_->Render().DrawRectangle(barX, barY, barW, barH,
                                         ui::OverlayColors::PANEL_BG_PRIMARY);
      systemAPI_->Render().DrawRectangle(barX, thumbY, barW, thumbH,
                                         ui::OverlayColors::BORDER_BLUE);
    }
  } else {
    systemAPI_->Render().DrawTextDefault(
        "説明文がありません", x, y, static_cast<float>(info_panel_.font_size),
        GRAY);
  }
}

// ========== イベント�E琁E==========

void CodexOverlay::SwitchTab(CodexTab tab) {
  if (activeTab_ == tab)
    return;
  activeTab_ = tab;

  // ビューア状態リセチE��
  character_viewport_.current_animation = AnimationType::Move;
  character_viewport_.animation_timer = 0.0f;
  character_viewport_.animation_frame = 0;
  character_viewport_.has_error = false;
  character_viewport_.error_message.clear();

  infoScrollPx_ = 0.0f;
  infoCachedKey_.clear();

  dropdownKind_ = DropdownKind::None;
  dropdownSlotIndex_ = -1;
  dropdownScrollPx_ = 0.0f;
}

void CodexOverlay::OnListItemClick(int index) {
  const int ti = TabIndex(activeTab_);
  const auto &entries = tabEntries_[ti];
  if (index < 0 || index >= static_cast<int>(entries.size()))
    return;

  tabSelectedIndex_[ti] = index;

  character_viewport_.current_animation = AnimationType::Move;
  character_viewport_.animation_timer = 0.0f;
  character_viewport_.animation_frame = 0;
  character_viewport_.has_error = false;
  character_viewport_.error_message.clear();

  const auto &e = entries[index];
  LOG_INFO("CodexOverlay: Selected entry: {} ({})", e.name, e.id);

  infoScrollPx_ = 0.0f;
  infoCachedKey_.clear();

  dropdownKind_ = DropdownKind::None;
  dropdownSlotIndex_ = -1;
  dropdownScrollPx_ = 0.0f;
}

void CodexOverlay::OnListScroll(int delta) {
  const int ti = TabIndex(activeTab_);
  const auto &entries = tabEntries_[ti];
  const float innerW = list_panel_.width - list_panel_.padding * 2.0f;
  const float innerH =
      list_panel_.height - kPanelHeaderH - list_panel_.padding * 2.0f;
  const int columns =
      std::max(1, static_cast<int>(std::floor(
                      (innerW + list_panel_.card_gap) /
                      (list_panel_.card_width + list_panel_.card_gap))));
  const int visibleRows =
      std::max(1, static_cast<int>(std::floor(
                      (innerH + list_panel_.card_gap) /
                      (list_panel_.card_height + list_panel_.card_gap))));
  const int totalItems = static_cast<int>(entries.size());
  const int totalRows = (totalItems + columns - 1) / columns;
  int maxScroll = std::max(0, totalRows - visibleRows);

  tabScrollOffset_[ti] =
      std::max(0, std::min(maxScroll, tabScrollOffset_[ti] - delta));
}

// ========== ユーチE��リチE�� ==========

int CodexOverlay::TabIndex(CodexTab tab) const {
  switch (tab) {
  case CodexTab::Characters:
    return 0;
  case CodexTab::Equipment:
    return 1;
  case CodexTab::Passives:
    return 2;
  default:
    return 0;
  }
}

CodexOverlay::CodexEntry *CodexOverlay::GetSelectedEntry() {
  const int ti = TabIndex(activeTab_);
  const int idx = tabSelectedIndex_[ti];
  if (idx < 0 || idx >= static_cast<int>(tabEntries_[ti].size()))
    return nullptr;
  return &tabEntries_[ti][idx];
}

const CodexOverlay::CodexEntry *CodexOverlay::GetSelectedEntry() const {
  const int ti = TabIndex(activeTab_);
  const int idx = tabSelectedIndex_[ti];
  if (idx < 0 || idx >= static_cast<int>(tabEntries_[ti].size()))
    return nullptr;
  return &tabEntries_[ti][idx];
}

const entities::Character *CodexOverlay::GetSelectedCharacter() const {
  if (activeTab_ != CodexTab::Characters)
    return nullptr;
  const auto *e = GetSelectedEntry();
  return e ? e->character : nullptr;
}

const entities::Equipment *CodexOverlay::GetSelectedEquipment() const {
  if (activeTab_ != CodexTab::Equipment)
    return nullptr;
  const auto *e = GetSelectedEntry();
  return e ? e->equipment : nullptr;
}

const entities::PassiveSkill *CodexOverlay::GetSelectedPassive() const {
  if (activeTab_ != CodexTab::Passives)
    return nullptr;
  const auto *e = GetSelectedEntry();
  return e ? e->passive : nullptr;
}

int CodexOverlay::ExtractIdNumber(const std::string &id) {
  // IDから数値部刁E��抽出�E�侁E "cat_001" -> 1, "dog_001" -> 1�E�E
  std::regex pattern(R"(_(\d+)$)");
  std::smatch match;
  if (std::regex_search(id, match, pattern)) {
    try {
      return std::stoi(match[1].str());
    } catch (...) {
      return 9999; // パ�Eス失敗時は最後に
    }
  }
  return 9999;
}

void CodexOverlay::SortCharactersById(std::vector<CodexEntry> &entries) {
  std::sort(entries.begin(), entries.end(),
            [](const CodexEntry &a, const CodexEntry &b) {
              const int num_a = ExtractIdNumber(a.id);
              const int num_b = ExtractIdNumber(b.id);
              if (num_a != num_b) {
                return num_a < num_b;
              }
              return a.id < b.id;
            });
}

void CodexOverlay::EnsureEntriesLoaded(SharedContext &ctx) {
  // どれかが埋まってぁE��ばロード済みとみなす（�E回だけ構築！E
  if (!tabEntries_[0].empty() || !tabEntries_[1].empty() ||
      !tabEntries_[2].empty()) {
    return;
  }

  // キャラ
  if (ctx.gameplayDataAPI) {
    const auto &masters = ctx.gameplayDataAPI->GetAllCharacterMasters();
    auto &out = tabEntries_[TabIndex(CodexTab::Characters)];
    out.reserve(masters.size());
    for (const auto &[id, ch] : masters) {
      CodexEntry e;
      e.type = CodexEntry::Type::Character;
      e.id = id;
      e.name = ch.name;
      e.description = ch.description;
      e.is_discovered =
          ctx.gameplayDataAPI
              ? ctx.gameplayDataAPI->GetCharacterState(id).unlocked
              : ch.default_unlocked;
      e.character = &ch;
      out.push_back(std::move(e));
    }
    SortCharactersById(out);
    if (!out.empty()) {
      tabSelectedIndex_[TabIndex(CodexTab::Characters)] = 0;
    }
    LOG_INFO("CodexOverlay: Loaded {} characters", out.size());
  }

  // 裁E��
  if (ctx.gameplayDataAPI) {
    {
      auto all = ctx.gameplayDataAPI->GetAllEquipment();
      auto &out = tabEntries_[TabIndex(CodexTab::Equipment)];
      out.reserve(all.size());
      for (const auto *eq : all) {
        if (!eq)
          continue;
        CodexEntry e;
        e.type = CodexEntry::Type::Equipment;
        e.id = eq->id;
        e.name = eq->name;
        e.description = eq->description;
        e.is_discovered = true;
        e.equipment = eq;
        out.push_back(std::move(e));
      }
      std::sort(
          out.begin(), out.end(),
          [](const CodexEntry &a, const CodexEntry &b) { return a.id < b.id; });
      if (!out.empty())
        tabSelectedIndex_[TabIndex(CodexTab::Equipment)] = 0;
      LOG_INFO("CodexOverlay: Loaded {} equipment", out.size());
    }

    {
      auto all = ctx.gameplayDataAPI->GetAllPassiveSkills();
      auto &out = tabEntries_[TabIndex(CodexTab::Passives)];
      out.reserve(all.size());
      for (const auto *ps : all) {
        if (!ps)
          continue;
        CodexEntry e;
        e.type = CodexEntry::Type::Passive;
        e.id = ps->id;
        e.name = ps->name;
        e.description = ps->description;
        e.is_discovered = true;
        e.passive = ps;
        out.push_back(std::move(e));
      }
      std::sort(
          out.begin(), out.end(),
          [](const CodexEntry &a, const CodexEntry &b) { return a.id < b.id; });
      if (!out.empty())
        tabSelectedIndex_[TabIndex(CodexTab::Passives)] = 0;
      LOG_INFO("CodexOverlay: Loaded {} passives", out.size());
    }
  }
}

bool CodexOverlay::RequestClose() const {
  if (requestClose_) {
    requestClose_ = false;
    return true;
  }
  return false;
}

bool CodexOverlay::RequestTransition(GameState &nextState) const {
  if (hasTransitionRequest_) {
    nextState = requestedNextState_;
    hasTransitionRequest_ = false;
    return true;
  }
  return false;
}

} // namespace core
} // namespace game
