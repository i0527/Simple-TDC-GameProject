#pragma once

// 標準ライブラリ
#include <functional>
#include <string>
#include <vector>

// プロジェクト内
#include "../config/RenderTypes.hpp"

namespace game {
namespace core {

class BaseSystemAPI;

/// @brief リソース読み込み進捗情報
struct LoadProgress {
  int current;
  int total;
  std::string message;
};

/// @brief 進捗通知コールバック型
using ProgressCallback = std::function<void(const LoadProgress&)>;

/// @brief リソースタイプ
enum class ResourceType {
  Font,
  Texture,
  Sound,
  Json
};

/// @brief リソースファイル情報
struct ResourceFileInfo {
  ResourceType type;
  std::string path;
  std::string name;
};

/// @brief アセットライセンス情報
struct AssetLicenseEntry {
  std::string packName;
  std::string licenseText;
  std::string sourcePath;
};

/// @brief リソースAPI
class ResourceSystemAPI {
public:
  struct TextureCacheEntry {
    std::string key;
    unsigned int id = 0;
    int width = 0;
    int height = 0;
  };

  explicit ResourceSystemAPI(BaseSystemAPI* owner);
  ~ResourceSystemAPI() = default;

  void InitializeResources();
  void InitializeResources(ProgressCallback callback);
  bool IsResourcesInitialized() const;

  void* GetTexture(const std::string& name);
  Texture2D* GetTexturePtr(const std::string& name);
  bool HasTexture(const std::string& name) const;
  bool TextureExists(const std::string& name) const;
  size_t GetTextureCacheCount() const;
  std::vector<TextureCacheEntry> GetTextureCacheEntries() const;
  bool IsTextureKeyRegistered(const std::string& name) const;
  const std::vector<AssetLicenseEntry>& GetAssetLicenses() const;

  void* GetSound(const std::string& name);
  void* GetMusic(const std::string& name);

  void* GetFont(const std::string& name);
  void SetDefaultFont(const std::string& name, int fontSize);
  void* GetDefaultFont() const;

  int ScanResourceFiles();
  bool LoadNextResource(ProgressCallback callback = nullptr);
  bool HasMoreResources() const;
  LoadProgress GetCurrentProgress() const;
  void ResetLoadingState();

private:
  BaseSystemAPI* owner_;

  void ScanDirectory(const std::string& dirPath, ResourceType type,
                     const std::vector<std::string>& extensions);
  void ScanDirectoryRecursive(const std::string& dirPath, ResourceType type,
                              const std::vector<std::string>& extensions);
  void ScanAssetLicenses();
  void LoadFont(const std::string& path, const std::string& name);
  void LoadTexture(const std::string& path, const std::string& name);
  void LoadSound(const std::string& path, const std::string& name);
  void LoadJson(const std::string& path, const std::string& name);
  Texture2D CreatePlaceholderTexture(const std::string& name);
};

} // namespace core
} // namespace game
