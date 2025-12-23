#pragma once

#include <string>
#include <vector>
#include <unordered_map>

#include <raylib.h>

#include "Data/DefinitionRegistry.h"
#include "Game/Managers/FormationManager.h"
#include "Game/Scenes/IScene.h"
#include "Game/Graphics/GridSheetProvider.h"
#include "Game/Graphics/SpriteRenderer.h"
#include "Game/Graphics/AsepriteJsonAtlasProvider.h"

namespace Game::Scenes {

class FormationScene : public IScene {
public:
  FormationScene(Font font, int screen_width, int screen_height,
                 Shared::Data::DefinitionRegistry &definitions,
                 Game::Managers::FormationManager &formation_manager);
  ~FormationScene() override;

  void Update(float delta_time) override;
  void Draw() override;

  bool ConsumeReturnHome();

private:
  struct LayoutInfo {
    Rectangle candidatePanel{};
    Rectangle slotPanel{};
    Rectangle statusPanel{};
    Rectangle previewPanel{};
    int candidatesPerRow = 1;
    int candidateRows = 1;
    int slotCols = 1;
    int slotRows = 1;
  };

  struct SpriteSheet {
    Texture2D texture{};
    int frameWidth = 64;
    int frameHeight = 64;
    int frames = 1;
    bool loaded = false;
  };

  struct DragState {
    bool active = false;
    int candidateIndex = -1;
    Vector2 startPos{0.0f, 0.0f};
    Vector2 currentPos{0.0f, 0.0f};
  };

  struct PreviewState {
    float animTimer = 0.0f;
    int currentFrame = 0;
    bool playingAttack = false;
  };

  struct EnhancementState {
    int level = 1;
    int maxLevel = 5;
    int currency = 500;
    int upgradeCost = 120;
    std::string message = "強化ポイントを消費してレベルを上げられます(仮)";
  };

  Font font_;
  int screen_width_;
  int screen_height_;
  Shared::Data::DefinitionRegistry &definitions_;
  Game::Managers::FormationManager &formation_manager_;

  std::vector<std::string> candidates_;
  std::vector<std::string> slots_;
  int selected_candidate_ = 0;
  int selected_slot_ = 0;
  bool apply_requested_ = false;
  bool return_home_requested_ = false;
  DragState drag_;
  PreviewState preview_;
  EnhancementState enhancement_;
  float candidate_scroll_ = 0.0f;

  mutable std::unordered_map<std::string, std::unique_ptr<Game::Graphics::AsepriteJsonAtlasProvider>> provider_cache_;
  Game::Graphics::SpriteRenderer sprite_renderer_;
  mutable std::unordered_map<std::string, Texture2D> icon_cache_;

  void RefreshData();
  LayoutInfo BuildLayout() const;
  void HandleInput();
  void DrawCandidates(const Rectangle &panel, int candidates_per_row,
                      int candidate_rows) const;
  void DrawSlots(const Rectangle &panel, int slot_cols) const;
  void AssignSelectedToSlot(int slot_index);
  void DrawIcon(const Rectangle &rect, const std::string &entity_id) const;
  const Shared::Data::EntityDef *GetSelectedDef() const;
  const Game::Graphics::AsepriteJsonAtlasProvider *GetProvider(const std::string &entity_id) const;
  void DrawStatusPanel(const Rectangle &panel, const Shared::Data::EntityDef *def);
  void DrawCharacterPreview(const Rectangle &panel,
                            const Shared::Data::EntityDef *def);
  void UpdatePreview(float delta_time);
  void StartAttackPreview();
  void DrawSpriteAnim(const Rectangle &area, Game::Graphics::AsepriteJsonAtlasProvider &provider,
                      const std::string &clipName, float elapsed, int current_frame) const;
  bool HasPreviewAnimation() const;
  void TryMockUpgrade();
  Rectangle GetEnhancementArea(const Rectangle &status_panel) const;
};

} // namespace Game::Scenes
