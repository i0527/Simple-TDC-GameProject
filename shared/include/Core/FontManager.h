#pragma once

#include <imgui.h>
#include <raylib.h>
#include <string>
#include <vector>


namespace Shared::Core {

class FontManager {
public:
  FontManager() = default;
  ~FontManager() = default;

  // Raylib 用フォントロード
  Font LoadJapaneseFont(const char *filePath, int fontSize);

  // ImGui 用フォントロード
  ImFont *LoadImGuiJapaneseFont(const char *filePath, float fontSize);

private:
  std::vector<int> GetJapaneseCodepoints() const;
};

} // namespace Shared::Core
