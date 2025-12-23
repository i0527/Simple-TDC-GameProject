#pragma once

#include <entt/entt.hpp>
#include <raylib.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace Shared::Data {
struct SpriteSheetAtlas;
}

namespace Shared::Data::Graphics {
class IFrameProvider;
}

namespace Game::Components {

/// @brief Transformコンポーネント（スケール・フリップ対応 + 後方互換性）
struct Transform {
  float x = 0.0f;        // ワールド座標X（足元基準）
  float y = 0.0f;        // ワールド座標Y（足元基準）
  float scaleX = 1.0f;   // X軸スケール（新）
  float scaleY = 1.0f;   // Y軸スケール（新）
  float rotation = 0.0f; // 回転角度（度）
  bool flipH = false;    // 水平反転（新）
  bool flipV = false;    // 垂直反転（新）
};

/// @brief Animationコンポーネント（新旧両方のフィールドをサポート）
struct Animation {
  // 新しい定義（クリップベース）
  std::string currentClip = "idle"; // 現在のクリップ名
  int frameIndex = 0;               // 現在のフレームインデックス
  float elapsedTime = 0.0f;         // 経過時間（秒）
  bool isPlaying = true;            // 再生中フラグ
  
  // 古い定義（後方互換性のため保持）
  enum class State { Idle, Walk, Attack, Death };
  State state = State::Idle;
  int columns = 4;
  int rows = 1;
  int frames_per_state = 4;
  int current_frame = 0;
  float frame_timer = 0.0f;
  float frame_duration = 0.15f;
  bool playing = true;

  // Atlas-driven (Aseprite)
  bool useAtlas = false;
  std::string currentAction = "idle";
  int atlasFrameIndex = 0;
  float atlasFrameTimer = 0.0f;
  float atlasDefaultFps = 10.0f;
  bool atlasLoop = true;
  std::unordered_map<std::string, std::string> actionToJson;
  // Per-action mirror defaults (from JSON meta or entity display overrides)
  std::unordered_map<std::string, bool> mirrorHByAction;
  std::unordered_map<std::string, bool> mirrorVByAction;
};

/// @brief Spriteコンポーネント（新旧両方のフィールドをサポート）
struct Sprite {
  // 新しい定義（Providerポインタベース）
  Shared::Data::Graphics::IFrameProvider *provider = nullptr; // FrameRef参照提供
  
  // 古い定義（後方互換性のため保持）
  std::string texturePath;
  std::string atlasJsonPath;
  const Shared::Data::SpriteSheetAtlas *atlas = nullptr;
  Texture2D texture{};
  bool loaded = false;
  bool failed = false;
};

/// @brief エンティティのタグ（味方/敵）
struct Team {
  enum class Type { Player, Enemy };
  Type type = Type::Player;
};

/// @brief エンティティのステータス
struct Stats {
  int max_hp = 100;
  int current_hp = 100;
  int attack = 10;
  float attack_speed = 1.0f;
  int range = 100;
  float move_speed = 50.0f;
  int knockback = 0;
};

/// @brief エンティティの移動状態
struct Velocity {
  float x = 0.0f;
  float y = 0.0f;
};

/// @brief エンティティの定義ID
struct EntityDefId {
  std::string id;
};

/// @brief 攻撃クールダウン
struct AttackCooldown {
  float remaining = 0.0f;
};

/// @brief スキル保持
struct SkillHolder {
  std::vector<std::string> skill_ids;
};

/// @brief アビリティ保持
struct AbilityHolder {
  std::vector<std::string> ability_ids;
};

/// @brief スキルクールダウン（全スキル共有の単一タイマー暫定）
struct SkillCooldown {
  float remaining = 0.0f;
};

/// @brief ヒットエフェクト
struct HitEffect {
  float timer = 0.2f;
};

/// @brief 死亡マーカー
struct Dead {
  float death_timer = 0.0f;
  float death_duration = 0.6f;
};

/// @brief ダメージポップ表示
struct DamagePopup {
  int value = 0;
  float timer = 0.0f;
  float duration = 0.8f;
  Vector2 offset{0.0f, -20.0f};
  float rise_speed = 30.0f;
};

/// @brief ベース（城）など描画除外用のタグ
struct BaseMarker {};

} // namespace Game::Components
