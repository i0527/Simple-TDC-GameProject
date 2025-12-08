#pragma once

#include <string>
#include <vector>

#include "Core/IResourceManager.h"

namespace New::Core {

// フォントなど共通リソースをまとめて管理する（現状フォント専用）
class ResourceManager : public IResourceManager {
public:
  ResourceManager() = default;
  ~ResourceManager() override = default;

  bool Initialize() override;
  void Shutdown() override;
  bool IsInitialized() const override { return initialized_; }

  bool LoadDefaultFont(const char *path, int fontSize) override;
  bool HasDefaultFont() const override { return defaultFontLoaded_; }
  const Font &GetDefaultFont() const override { return defaultFont_; }

private:
  bool initialized_ = false;
  bool defaultFontLoaded_ = false;
  Font defaultFont_{};

  static std::vector<int> BuildJapaneseCodepoints();
};

} // namespace New::Core
