#pragma once

#include "raylib.h"

namespace New::Core {

// シンプルなリソース管理インタフェース（現状はフォント用途）
class IResourceManager {
public:
  virtual ~IResourceManager() = default;

  virtual bool Initialize() = 0;
  virtual void Shutdown() = 0;

  virtual bool IsInitialized() const = 0;

  // デフォルトフォントのロードと取得
  virtual bool LoadDefaultFont(const char *path, int fontSize) = 0;
  virtual bool HasDefaultFont() const = 0;
  virtual const Font &GetDefaultFont() const = 0;

  // 予約枠: 後続の具体的なリソースロードAPIをここに追加する
  virtual void Warmup() {}
};

} // namespace New::Core
