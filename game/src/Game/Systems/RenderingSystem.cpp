#include "Game/Systems/RenderingSystem.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <vector>

namespace {
constexpr float ENTITY_SIZE = 40.0f;
constexpr float HUD_FONT_SIZE = 18.0f;
constexpr float SPRITE_DRAW_SCALE = 1.0f;
struct ActionPlayback {
  bool loop;
  float frameDuration;
};

ActionPlayback DefaultPlayback(const std::string &action) {
  if (action == "walk") {
    return {true, 1.0f / 12.0f};
  }
  if (action == "attack") {
    return {false, 1.0f / 10.0f};
  }
  if (action == "death") {
    return {false, 1.0f / 8.0f};
  }
  return {true, 1.0f / 8.0f}; // idleその他
}
} // namespace

namespace Game::Systems {

void RenderingSystem::UpdateAnimations(entt::registry &registry,
                                       float delta_time) {
  auto anim_view = registry.view<Components::Animation>();
  anim_view.each([delta_time]([[maybe_unused]] entt::entity entity,
                              Components::Animation &anim) {
    if (!anim.playing || anim.useAtlas) {
      return;
    }
    anim.frame_timer += delta_time;
    if (anim.frame_timer >= anim.frame_duration) {
      anim.frame_timer = 0.0f;
      anim.current_frame =
          (anim.current_frame + 1) % std::max(1, anim.frames_per_state);
    }
  });

  auto atlas_view = registry.view<Components::Animation, Components::Sprite>();
  atlas_view.each([this, delta_time](entt::entity /*entity*/,
                                     Components::Animation &anim,
                                     Components::Sprite &sprite) {
    if (!anim.playing || !anim.useAtlas) {
      return;
    }

    const auto *atlas = EnsureAtlas(anim, sprite);
    if (!atlas) {
      return;
    }

    const auto clip_it = atlas->tags.find(anim.currentAction);
    const auto default_it = atlas->tags.find("default");
    const auto *clip =
        (clip_it != atlas->tags.end()) ? &clip_it->second
                                       : (default_it != atlas->tags.end()
                                              ? &default_it->second
                                              : nullptr);
    if (!clip || clip->frameIndices.empty()) {
      return;
    }

    const auto playback = DefaultPlayback(anim.currentAction);
    const int clip_index =
        std::min(static_cast<int>(clip->frameIndices.size() - 1),
                 std::max(0, anim.atlasFrameIndex));
    const int frame_id = clip->frameIndices[clip_index];
    float duration_sec = playback.frameDuration;
    if (frame_id >= 0 &&
        frame_id < static_cast<int>(atlas->frames.size())) {
      int dur_ms = atlas->frames[frame_id].durationMs;
      if (dur_ms > 0) {
        duration_sec = static_cast<float>(dur_ms) / 1000.0f;
      }
    }

    anim.atlasFrameTimer += delta_time;
    if (anim.atlasFrameTimer >= duration_sec) {
      anim.atlasFrameTimer = 0.0f;
      anim.atlasFrameIndex += 1;
      if (anim.atlasFrameIndex >=
          static_cast<int>(clip->frameIndices.size())) {
        const bool loop = playback.loop && clip->loop;
        anim.atlasFrameIndex = loop
                                   ? 0
                                   : static_cast<int>(clip->frameIndices.size()) - 1;
      }
    }
  });

  auto hit_view = registry.view<Components::HitEffect>();
  std::vector<entt::entity> to_remove;
  hit_view.each(
      [delta_time, &to_remove](entt::entity e, Components::HitEffect &hit) {
        hit.timer -= delta_time;
        if (hit.timer <= 0.0f) {
          to_remove.push_back(e);
        }
      });
  for (auto e : to_remove) {
    registry.remove<Components::HitEffect>(e);
  }

  auto dmg_view = registry.view<Components::DamagePopup>();
  std::vector<entt::entity> dmg_remove;
  dmg_view.each(
      [delta_time, &dmg_remove](entt::entity e, Components::DamagePopup &pop) {
        pop.timer += delta_time;
        pop.offset.y -= pop.rise_speed * delta_time;
        if (pop.timer >= pop.duration) {
          dmg_remove.push_back(e);
        }
      });
  for (auto e : dmg_remove) {
    registry.remove<Components::DamagePopup>(e);
  }
}

void RenderingSystem::Draw(entt::registry &registry, const Font &font) const {
  DrawEntities(registry);
  DrawDamagePopups(registry);
  DrawHud(font);
}

void RenderingSystem::Shutdown(entt::registry &registry) {
  // Unload textures held by sprites
  registry.view<Components::Sprite>().each(
      [](entt::entity /*entity*/, auto &sprite) {
    if (sprite.loaded && sprite.texture.id != 0) {
      UnloadTexture(sprite.texture);
      sprite.texture = Texture2D{};
      sprite.loaded = false;
    }
  });
  // Unload cached textures
  for (auto &kv : texture_cache_) {
    if (kv.second.id != 0) {
      UnloadTexture(kv.second);
    }
  }
  texture_cache_.clear();
}

Texture2D *
RenderingSystem::LoadTextureIfNeeded(Components::Sprite &sprite) const {
  if (sprite.failed) {
    return nullptr;
  }

  if (sprite.loaded) {
    return &sprite.texture;
  }

  if (sprite.texturePath.empty() && sprite.atlas &&
      !sprite.atlas->imagePath.empty()) {
    sprite.texturePath = sprite.atlas->imagePath;
    sprite.loaded = false;
    sprite.failed = false;
  }

  if (sprite.texturePath.empty()) {
    sprite.failed = true;
    return nullptr;
  }

  // キャッシュヒット
  auto it = texture_cache_.find(sprite.texturePath);
  if (it != texture_cache_.end()) {
    sprite.texture = it->second;
    sprite.loaded = true;
    return &sprite.texture;
  }

  if (!std::filesystem::exists(sprite.texturePath)) {
    sprite.failed = true;
    std::cerr << "[RenderingSystem] Missing texture: " << sprite.texturePath
              << std::endl;
    return nullptr;
  }

  Texture2D tex = LoadTexture(sprite.texturePath.c_str());
  if (tex.id == 0) {
    sprite.failed = true;
    return nullptr;
  }

  sprite.texture = tex;
  sprite.loaded = true;
  texture_cache_.emplace(sprite.texturePath, tex);
  return &sprite.texture;
}

const Shared::Data::SpriteSheetAtlas *
RenderingSystem::EnsureAtlas(Components::Animation &anim,
                             Components::Sprite &sprite) const {
  if (!anim.useAtlas || anim.actionToJson.empty()) {
    return nullptr;
  }

  auto action_it = anim.actionToJson.find(anim.currentAction);
  if (action_it == anim.actionToJson.end()) {
    anim.currentAction = anim.actionToJson.begin()->first;
    action_it = anim.actionToJson.begin();
  }

  const std::string &json_path = action_it->second;
  if (sprite.atlasJsonPath != json_path || sprite.atlas == nullptr) {
    sprite.atlasJsonPath = json_path;
    sprite.atlas = atlas_cache_.GetOrLoad(json_path);
    if (!sprite.atlas) {
      std::cerr << "[RenderingSystem] Failed to load atlas: " << json_path
                << std::endl;
      return nullptr;
    }
    if (sprite.texturePath.empty() && !sprite.atlas->imagePath.empty()) {
      sprite.texturePath = sprite.atlas->imagePath;
      sprite.loaded = false;
      sprite.failed = false;
    }
  }

  return sprite.atlas;
}

void RenderingSystem::DrawEntities(entt::registry &registry) const {
  auto view =
      registry
          .view<Components::Transform, Components::Stats, Components::Team>();

  for (auto entity : view) {
    if (registry.any_of<Components::BaseMarker>(entity)) {
      continue; // ベースは独自描画のためスキップ
    }
    if (registry.any_of<Components::Dead>(entity)) {
      continue; // 死亡済みは描画しない
    }
    const auto &transform = view.get<Components::Transform>(entity);
    const auto &team = view.get<Components::Team>(entity);
    const auto &stats = view.get<Components::Stats>(entity);

    Rectangle bounds{};
    bool drew_sprite = false;

    if (auto *sprite = registry.try_get<Components::Sprite>(entity)) {
      Components::Animation *anim = registry.try_get<Components::Animation>(entity);
      if (anim && anim->useAtlas) {
        const auto *atlas = EnsureAtlas(*anim, *sprite);
        if (atlas && !atlas->frames.empty()) {
          const auto clip_it = atlas->tags.find(anim->currentAction);
          const auto default_it = atlas->tags.find("default");
          const auto *clip =
              (clip_it != atlas->tags.end()) ? &clip_it->second
                                             : (default_it != atlas->tags.end()
                                                    ? &default_it->second
                                                    : nullptr);
          if (clip && !clip->frameIndices.empty()) {
            int clip_index =
                std::min(static_cast<int>(clip->frameIndices.size() - 1),
                         std::max(0, anim->atlasFrameIndex));
            int frame_id = clip->frameIndices[clip_index];
            if (frame_id >= 0 &&
                frame_id < static_cast<int>(atlas->frames.size())) {
              const auto &fr = atlas->frames[frame_id];
              if (auto *tex = LoadTextureIfNeeded(*sprite)) {
                if (tex->id != 0) {
                  Rectangle src{static_cast<float>(fr.x),
                                static_cast<float>(fr.y),
                                static_cast<float>(fr.w),
                                static_cast<float>(fr.h)};
                      // Combine global flip and per-action mirror
                      bool flipH = transform.flipH;
                      bool flipV = transform.flipV;
                      auto itH = anim->mirrorHByAction.find(anim->currentAction);
                      auto itV = anim->mirrorVByAction.find(anim->currentAction);
                      if (itH != anim->mirrorHByAction.end()) flipH = flipH || itH->second;
                      if (itV != anim->mirrorVByAction.end()) flipV = flipV || itV->second;
                      Rectangle dst{
                      transform.x + static_cast<float>(fr.sourceX),
                      transform.y + static_cast<float>(fr.sourceY),
                        (flipH ? -1.0f : 1.0f) * std::abs(src.width) * SPRITE_DRAW_SCALE,
                        (flipV ? -1.0f : 1.0f) * std::abs(src.height) * SPRITE_DRAW_SCALE};
                      Vector2 origin{std::fabs(dst.width) * 0.5f, std::fabs(dst.height) * 0.5f};
                      DrawTexturePro(*tex, src, dst, origin, transform.rotation,
                                 RAYWHITE);
                  bounds = dst;
                  drew_sprite = true;
                }
              }
            }
          }
        }
      } else if (auto *tex = LoadTextureIfNeeded(*sprite)) {
        if (tex->id != 0) {
          Rectangle src{0.0f, 0.0f, static_cast<float>(tex->width),
                        static_cast<float>(tex->height)};
          if (anim) {
            int cols = std::max(1, anim->columns);
            int rows = std::max(1, anim->rows);
            int frame_w = tex->width / cols;
            int frame_h = tex->height / rows;
            int row_index = 0;
            switch (anim->state) {
            case Components::Animation::State::Walk:
              row_index = 1;
              break;
            case Components::Animation::State::Attack:
              row_index = 2;
              break;
            case Components::Animation::State::Death:
              row_index = 3;
              break;
            case Components::Animation::State::Idle:
            default:
              row_index = 0;
              break;
            }
            row_index = std::min(row_index, rows - 1);
            int frame_count = std::max(1, anim->frames_per_state);
            int frame_index = std::min(anim->current_frame, frame_count - 1);
            src = {static_cast<float>(frame_w * frame_index),
                   static_cast<float>(frame_h * row_index),
                   static_cast<float>(frame_w), static_cast<float>(frame_h)};
          }

          bool flipH = transform.flipH;
          bool flipV = transform.flipV;
          Rectangle dst{transform.x, transform.y,
                        (flipH ? -1.0f : 1.0f) * std::abs(src.width) * SPRITE_DRAW_SCALE,
                        (flipV ? -1.0f : 1.0f) * std::abs(src.height) * SPRITE_DRAW_SCALE};
          Vector2 origin{std::fabs(dst.width) * 0.5f, std::fabs(dst.height) * 0.5f};
          DrawTexturePro(*tex, src, dst, origin, transform.rotation, RAYWHITE);
          bounds = dst;
          drew_sprite = true;
        }
      }
    }

    if (!drew_sprite) {
      Color body = (team.type == Components::Team::Type::Player) ? BLUE : RED;
      bounds = {transform.x, transform.y, ENTITY_SIZE, ENTITY_SIZE};
      DrawRectangleRec(bounds, body);
    }

    // HPバー
    float hp_ratio =
        std::clamp(static_cast<float>(stats.current_hp) /
                       static_cast<float>(std::max(1, stats.max_hp)),
                   0.0f, 1.0f);
    Rectangle hp_bg{bounds.x, bounds.y - 8.0f, bounds.width, 6.0f};
    Rectangle hp_fg{bounds.x, bounds.y - 8.0f, bounds.width * hp_ratio, 6.0f};
    DrawRectangleRec(hp_bg, DARKGRAY);
    DrawRectangleRec(hp_fg, GREEN);

    if (auto *hit = registry.try_get<Components::HitEffect>(entity)) {
      DrawRectangleLinesEx({bounds.x - 4.0f, bounds.y - 4.0f,
                            bounds.width + 8.0f, bounds.height + 8.0f},
                           2.0f, Color{255, 200, 100, 180});
    }
  }
}

void RenderingSystem::DrawDamagePopups(entt::registry &registry) const {
  auto view = registry.view<Components::DamagePopup, Components::Transform>();
  for (auto entity : view) {
    const auto &pop = view.get<Components::DamagePopup>(entity);
    const auto &tr = view.get<Components::Transform>(entity);
    float t =
        std::clamp(pop.timer / std::max(0.001f, pop.duration), 0.0f, 1.0f);
    unsigned char alpha =
        static_cast<unsigned char>(255 * std::max(0.0f, 1.0f - t));
    std::string txt = std::to_string(pop.value);
    Vector2 pos{tr.x + pop.offset.x, tr.y + pop.offset.y};
    Vector2 size = MeasureTextEx(GetFontDefault(), txt.c_str(), 18.0f, 2.0f);
    DrawTextEx(GetFontDefault(), txt.c_str(),
               {pos.x - size.x * 0.5f, pos.y - size.y * 0.5f}, 18.0f, 2.0f,
               Color{255, 220, 150, alpha});
  }
}

void RenderingSystem::DrawHud(const Font &font) const {
  const float size = HUD_FONT_SIZE;
  Vector2 pos{20.0f, 20.0f};
  DrawTextEx(font, "HUD: Placeholder", pos, size, 2.0f, RAYWHITE);
}

} // namespace Game::Systems
