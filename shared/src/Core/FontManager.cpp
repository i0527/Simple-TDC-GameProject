#include "Core/FontManager.h"

#include <iostream>
#include <filesystem>

namespace Shared::Core {

std::string FontManager::ResolveFontPath(const char *requested) const {
  using std::filesystem::path;
  path req(requested ? requested : "");
  path fname = req.filename();

  // 候補リスト（順序優先）
  std::vector<path> candidates;
  if (!req.empty()) candidates.push_back(req);
  candidates.push_back(path("assets/shared/fonts") / fname);
  // よくあるタイポも許容
  candidates.push_back(path("assets/shared/font") / fname);    // fonts → font
  candidates.push_back(path("assets/shareds/fonts") / fname);   // shared → shareds
  candidates.push_back(path("assets/fonts") / fname);
  candidates.push_back(path("fonts") / fname);
  // ユーザレポートのタイプミス（shareds/font）にも対応
  candidates.push_back(path("assets/shareds/font") / fname);

  for (const auto &p : candidates) {
    std::string ps = p.string();
    if (FileExists(ps.c_str())) {
      return ps;
    }
  }
  return requested ? std::string(requested) : std::string();
}

Font FontManager::LoadJapaneseFont(const char *filePath, int fontSize) {
  std::string resolved = FileExists(filePath) ? std::string(filePath)
                                              : ResolveFontPath(filePath);
  if (!FileExists(resolved.c_str())) {
    std::cerr << "Font file not found: " << (filePath ? filePath : "")
              << " (resolved: " << resolved << ")" << std::endl;
    return GetFontDefault();
  }

  auto codepoints = GetJapaneseCodepoints();
  Font font = LoadFontEx(resolved.c_str(), fontSize, codepoints.data(),
                         static_cast<int>(codepoints.size()));

  if (font.texture.id == 0) {
    std::cerr << "Failed to load font: " << resolved << std::endl;
    return GetFontDefault();
  }

  SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
  return font;
}

ImFont *FontManager::LoadImGuiJapaneseFont(const char *filePath,
                                           float fontSize) {
  std::string resolved = FileExists(filePath) ? std::string(filePath)
                                              : ResolveFontPath(filePath);
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
  ImFont *font = io.Fonts->AddFontFromFileTTF(resolved.c_str(), fontSize, &config,
                                              glyphRanges.data());

  if (font == nullptr) {
    std::cerr << "Failed to load ImGui font: " << resolved << std::endl;
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
