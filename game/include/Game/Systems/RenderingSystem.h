#pragma once

#include <entt/entt.hpp>
#include <raylib.h>
#include <string>
#include <unordered_map>

#include "Game/Components/NewCoreComponents.h"
#include "Data/Loaders/SpriteSheetLoader.h"

namespace Game::Systems {

class RenderingSystem {
public:
  void UpdateAnimations(entt::registry &registry, float delta_time);
  void Draw(entt::registry &registry, const Font &font) const;
  void Shutdown(entt::registry &registry);

private:
  Texture2D *LoadTextureIfNeeded(Components::Sprite &sprite) const;
  const Shared::Data::SpriteSheetAtlas *
  EnsureAtlas(Components::Animation &anim,
              Components::Sprite &sprite) const;
  void DrawEntities(entt::registry &registry) const;
  void DrawDamagePopups(entt::registry &registry) const;
  void DrawHud(const Font &font) const;

  mutable std::unordered_map<std::string, Texture2D> texture_cache_;
  mutable Shared::Data::SpriteAtlasCache atlas_cache_;
};

} // namespace Game::Systems
