#include "Core/FontManager.h"

#include <iostream>

namespace Shared::Core {

Font FontManager::LoadJapaneseFont(const char *filePath, int fontSize) {
  if (!FileExists(filePath)) {
    std::cerr << "Font file not found: " << filePath << std::endl;
    return GetFontDefault();
  }

  auto codepoints = GetJapaneseCodepoints();
  Font font = LoadFontEx(filePath, fontSize, codepoints.data(),
                         static_cast<int>(codepoints.size()));

  if (font.texture.id == 0) {
    std::cerr << "Failed to load font: " << filePath << std::endl;
    return GetFontDefault();
  }

  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  return font;
}

ImFont *FontManager::LoadImGuiJapaneseFont(const char *filePath,
                                           float fontSize) {
  auto codepoints = GetJapaneseCodepoints();
  std::vector<ImWchar> glyphRanges;
  glyphRanges.reserve(codepoints.size() + 1);

  for (int cp : codepoints) {
    glyphRanges.push_back(static_cast<ImWchar>(cp));
  }
  glyphRanges.push_back(0);

  ImFontConfig config;
  config.OversampleH = 2;
  config.OversampleV = 2;
  config.PixelSnapH = true;

  ImGuiIO &io = ImGui::GetIO();
  ImFont *font = io.Fonts->AddFontFromFileTTF(filePath, fontSize, &config,
                                              glyphRanges.data());

  if (font == nullptr) {
    std::cerr << "Failed to load ImGui font: " << filePath << std::endl;
  }

  return font;
}

std::vector<int> FontManager::GetJapaneseCodepoints() const {
  std::vector<int> codepoints;
  codepoints.reserve(6000);

  auto appendRange = [&codepoints](int start, int end) {
    for (int cp = start; cp <= end; ++cp) {
      codepoints.push_back(cp);
    }
  };

  // ASCII
  appendRange(0x0020, 0x007E);
  // ひらがな / カタカナ / 句読点
  appendRange(0x3000, 0x303F);
  appendRange(0x3040, 0x309F);
  appendRange(0x30A0, 0x30FF);
  appendRange(0x31F0, 0x31FF); // カタカナ拡張
  appendRange(0xFF00, 0xFFEF); // 半角・全角フォーム
  // CJK統合漢字（JIS第一・第二水準を広くカバー）
  appendRange(0x4E00, 0x9FAF);

  return codepoints;
}

} // namespace Shared::Core
