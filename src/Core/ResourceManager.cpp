#include "Core/ResourceManager.h"
#include "Core/TraceCompat.h"

#include <algorithm>

namespace New::Core {

namespace {

void UnloadIfLoaded(Font &font, bool &flag) {
  if (flag && font.texture.id != 0) {
    UnloadFont(font);
  }
  flag = false;
}

} // namespace

bool ResourceManager::Initialize() {
  if (initialized_) {
    return true;
  }

  initialized_ = true;
  return initialized_;
}

void ResourceManager::Shutdown() {
  if (!initialized_) {
    return;
  }

  UnloadIfLoaded(defaultFont_, defaultFontLoaded_);
  initialized_ = false;
}

bool ResourceManager::LoadDefaultFont(const char *path, int fontSize) {
  if (!initialized_) {
    return false;
  }

  if (!path || fontSize <= 0) {
    TRACELOG(LOG_WARNING, "ResourceManager: invalid font request");
    return false;
  }

  if (!FileExists(path)) {
    TRACELOG(LOG_WARNING, "ResourceManager: font file not found: %s", path);
    return false;
  }

  UnloadIfLoaded(defaultFont_, defaultFontLoaded_);

  std::vector<int> codepoints = BuildJapaneseCodepoints();
  defaultFont_ = LoadFontEx(path, fontSize, codepoints.data(),
                            static_cast<int>(codepoints.size()));

  defaultFontLoaded_ = (defaultFont_.texture.id != 0);
  if (defaultFontLoaded_) {
    SetTextureFilter(defaultFont_.texture, TEXTURE_FILTER_BILINEAR);
    TRACELOG(LOG_INFO, "ResourceManager: font loaded: %s (size=%d)", path,
             fontSize);
  } else {
    TRACELOG(LOG_WARNING, "ResourceManager: font load failed: %s", path);
    defaultFont_ = Font{};
  }

  return defaultFontLoaded_;
}

std::vector<int> ResourceManager::BuildJapaneseCodepoints() {
  std::vector<int> cps;
  auto appendRange = [&cps](int start, int end) {
    cps.reserve(cps.size() + static_cast<size_t>(end - start + 1));
    for (int cp = start; cp <= end; ++cp) {
      cps.push_back(cp);
    }
  };

  // ASCII
  appendRange(0x0020, 0x007E);
  // ひらがな
  appendRange(0x3040, 0x309F);
  // カタカナ
  appendRange(0x30A0, 0x30FF);
  // JIS第一水準（CJK統合漢字の主要範囲）
  appendRange(0x4E00, 0x9FFF);
  // 拡張（JIS第二水準を広くカバー）
  appendRange(0x3400, 0x4DBF); // CJK拡張A
  appendRange(0xF900, 0xFAFF); // 互換漢字
  // 必要に応じてCJK拡張B以降（0x20000-）は別途検討（容量増対策）

  return cps;
}

} // namespace New::Core
