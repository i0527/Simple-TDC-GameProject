#include "../ResourceSystemAPI.hpp"
#include "../BaseSystemAPI.hpp"

// Standard library
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <vector>

// External libraries
#include <imgui.h>
#include <rlImGui.h>

// Project
#include "../../../utils/Log.h"

namespace game {
namespace core {
namespace {
std::string NormalizeSlashes(std::string s) {
  std::replace(s.begin(), s.end(), '\\', '/');
  return s;
}

bool StartsWith(const std::string &s, const std::string &prefix) {
  return s.size() >= prefix.size() && s.compare(0, prefix.size(), prefix) == 0;
}

std::string ToLower(std::string value) {
  std::transform(
      value.begin(), value.end(), value.begin(),
      [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  return value;
}

// - '\' -> '/'
std::string NormalizeTextureKey(std::string key) {
  key = NormalizeSlashes(std::move(key));

  if (StartsWith(key, "./")) {
    key = key.substr(2);
  }

  if (StartsWith(key, "data/")) {
    key = key.substr(5);
  }

  if (StartsWith(key, "assets/")) {
    const std::filesystem::path p(key);
    if (!p.has_extension()) {
      key += ".png";
    }
  }

  return key;
}

std::string MakeAssetsRelativeKey(const std::filesystem::path &fullPath) {
  try {
    std::filesystem::path rel =
        std::filesystem::relative(fullPath, std::filesystem::path("data"));
    return rel.generic_string();
  } catch (...) {
    return NormalizeSlashes(fullPath.string());
  }
}
} // namespace

ResourceSystemAPI::ResourceSystemAPI(BaseSystemAPI* owner) : owner_(owner) {}

void ResourceSystemAPI::InitializeResources() {
  LOG_INFO("ResourceSystemAPI resources initialized");
}

void ResourceSystemAPI::InitializeResources(ProgressCallback callback) {
  if (owner_->resourcesInitialized_) {
    LOG_WARN("ResourceSystemAPI::InitializeResources: Already initialized");
    return;
  }

  int totalFiles = ScanResourceFiles();
  if (totalFiles == 0) {
    LOG_WARN("ResourceSystemAPI::InitializeResources: No resource files found");
    owner_->resourcesInitialized_ = true;
    return;
  }

  // 騾ｲ謐鈴夂衍
  if (callback) {
    callback({0, totalFiles, "Scanning resources..."});
  }

  while (HasMoreResources()) {
    if (!LoadNextResource(callback)) {
      break;
    }
  }

  owner_->resourcesInitialized_ = true;
  LOG_INFO("ResourceSystemAPI::InitializeResources: Initialization completed");
}

bool ResourceSystemAPI::IsResourcesInitialized() const {
  return owner_->resourcesInitialized_;
}

void *ResourceSystemAPI::GetTexture(const std::string &name) {
  const std::string key = NormalizeTextureKey(name);

  auto it = owner_->textures_.find(key);
  if (it != owner_->textures_.end()) {
    return it->second.get();
  }

  std::string path;
  if (key.length() >= 7 && key.substr(0, 7) == "assets/") {
    path = "data/" + key;
  } else {
    path = "data/assets/textures/" + key;
  }
  if (path.length() < 4 || path.substr(path.length() - 4) != ".png") {
    path += ".png";
  }

  Texture2D texture = ::LoadTexture(path.c_str());

  if (texture.id == 0) {
    LOG_WARN("Failed to load texture: {}, creating placeholder", path);
    texture = CreatePlaceholderTexture(name);
  } else {
    LOG_INFO("Loaded texture: {}", path);
  }

  auto texturePtr =
      std::shared_ptr<Texture2D>(new Texture2D(texture), [](Texture2D *t) {
        if (t && t->id != 0) {
          UnloadTexture(*t);
        }
        delete t;
      });

  owner_->textures_[key] = texturePtr;
  return texturePtr.get();
}

Texture2D *ResourceSystemAPI::GetTexturePtr(const std::string &name) {
  return static_cast<Texture2D *>(GetTexture(name));
}

bool ResourceSystemAPI::HasTexture(const std::string &name) const {
  const std::string key = NormalizeTextureKey(name);
  return owner_->textures_.find(key) != owner_->textures_.end();
}

bool ResourceSystemAPI::TextureExists(const std::string &name) const {
  const std::string path = owner_->ResolveTexturePath(name);
  if (path.empty()) {
    return false;
  }
  return FileExists(path.c_str());
}

bool ResourceSystemAPI::IsTextureKeyRegistered(const std::string &name) const {
  const std::string key = NormalizeTextureKey(name);
  return owner_->registeredTextureKeys_.find(key) !=
         owner_->registeredTextureKeys_.end();
}

const std::vector<AssetLicenseEntry> &ResourceSystemAPI::GetAssetLicenses()
    const {
  return owner_->assetLicenses_;
}

size_t ResourceSystemAPI::GetTextureCacheCount() const {
  return owner_->textures_.size();
}

std::vector<ResourceSystemAPI::TextureCacheEntry>
ResourceSystemAPI::GetTextureCacheEntries() const {
  std::vector<TextureCacheEntry> entries;
  entries.reserve(owner_->textures_.size());

  for (const auto &[key, texPtr] : owner_->textures_) {
    TextureCacheEntry e;
    e.key = key;
    if (texPtr) {
      e.id = texPtr->id;
      e.width = texPtr->width;
      e.height = texPtr->height;
    }
    entries.push_back(std::move(e));
  }

  return entries;
}

void *ResourceSystemAPI::GetSound(const std::string &name) {
  auto it = owner_->sounds_.find(name);
  if (it != owner_->sounds_.end()) {
    return it->second.get();
  }

  std::vector<std::string> candidatePaths = {
      "data/assets/sounds/" + name + ".wav",
      "data/assets/sounds/" + name + ".ogg"};
  if (name == "button_click") {
    candidatePaths.push_back(
        "data/assets/other/kenney_ui-pack/Sounds/click-a.ogg");
  }
  const std::vector<std::string> kenneyNames = {
      "click-a", "click-b", "switch-a", "switch-b", "tap-a", "tap-b"};
  if (std::find(kenneyNames.begin(), kenneyNames.end(), name) !=
      kenneyNames.end()) {
    candidatePaths.push_back(
        "data/assets/other/kenney_ui-pack/Sounds/" + name + ".ogg");
  }

  Sound sound{};
  std::string loadedPath;
  for (const auto &path : candidatePaths) {
    if (!std::filesystem::exists(path)) {
      continue;
    }

    sound = ::LoadSound(path.c_str());
    if (sound.frameCount != 0) {
      loadedPath = path;
      break;
    }
  }

  if (sound.frameCount == 0) {
    LOG_ERROR("Failed to load sound: {}", name);
    return nullptr;
  }

  LOG_INFO("Loaded sound: {}", loadedPath);

  auto soundPtr = std::shared_ptr<Sound>(new Sound(sound), [](Sound *s) {
    if (s && s->frameCount != 0) {
      UnloadSound(*s);
    }
    delete s;
  });

  owner_->sounds_[name] = soundPtr;
  return soundPtr.get();
}

void *ResourceSystemAPI::GetMusic(const std::string &name) {
  auto it = owner_->musics_.find(name);
  if (it != owner_->musics_.end()) {
    return it->second.get();
  }

  std::string path = "data/assets/music/" + name + ".mp3";

  Music music = ::LoadMusicStream(path.c_str());

  if (music.frameCount == 0) {
    LOG_ERROR("Failed to load music: {}", path);
    return nullptr;
  }

  LOG_INFO("Loaded music: {}", path);

  auto musicPtr = std::shared_ptr<Music>(new Music(music), [](Music *m) {
    if (m && m->frameCount != 0) {
      UnloadMusicStream(*m);
    }
    delete m;
  });

  owner_->musics_[name] = musicPtr;
  return musicPtr.get();
}

void *ResourceSystemAPI::GetFont(const std::string &name) {
  auto it = owner_->fonts_.find(name);
  if (it != owner_->fonts_.end()) {
    return it->second.get();
  }

  std::string path = "data/assets/fonts/" + name;

  Font font = ::LoadFontEx(path.c_str(), 48, owner_->fontCodepoints_.data(),
                           static_cast<int>(owner_->fontCodepoints_.size()));

  if (font.baseSize == 0) {
    LOG_ERROR("Failed to load font: {}", path);
    return nullptr;
  }

  LOG_INFO("Loaded font: {}", path);

  auto fontPtr = std::shared_ptr<Font>(new Font(font), [](Font *f) {
    if (f && f->baseSize != 0) {
      UnloadFont(*f);
    }
    delete f;
  });

  owner_->fonts_[name] = fontPtr;
  return fontPtr.get();
}

void ResourceSystemAPI::SetDefaultFont(const std::string &name, int fontSize) {
  if (owner_->defaultFont_ && owner_->fonts_.find(name) != owner_->fonts_.end() &&
      owner_->defaultFont_ == owner_->fonts_[name]) {
    LOG_DEBUG(
        "ResourceSystemAPI::SetDefaultFont: Font '{}' is already set as default",
        name);
    return;
  }

  auto fontPtr = static_cast<Font *>(GetFont(name));
  if (fontPtr && fontPtr->baseSize != 0) {
    owner_->defaultFont_ = std::static_pointer_cast<Font>(owner_->fonts_[name]);

    SetTextureFilter(fontPtr->texture, TEXTURE_FILTER_BILINEAR);
    LOG_INFO(
        "ResourceSystemAPI::SetDefaultFont: Set default font '{}' with size {}",
        name, fontSize);

    if (!owner_->imGuiInitialized_) {
      try {
        rlImGuiSetup(true);

        ImGuiIO &io = ImGui::GetIO();
        std::string fontPath = "data/assets/fonts/" + name;

        ImFontConfig config;
        config.MergeMode = false; // merge disabled
        config.OversampleH = 2;
        config.OversampleV = 2;
        config.PixelSnapH = true;

        const ImWchar *glyphRanges = io.Fonts->GetGlyphRangesJapanese();

        ImFont *japaneseFont = io.Fonts->AddFontFromFileTTF(
            fontPath.c_str(), static_cast<float>(fontSize), &config,
            glyphRanges);

        if (japaneseFont) {
          io.FontDefault = japaneseFont;

          io.Fonts->Build();

          owner_->imGuiJapaneseFont_ = japaneseFont;
          owner_->imGuiInitialized_ = true;

          LOG_INFO("ResourceSystemAPI::SetDefaultFont: ImGui initialized with "
                   "Japanese font '{}'",
                   name);
          LOG_INFO("ResourceSystemAPI::SetDefaultFont: Font size: {}px", fontSize);
        } else {
          owner_->imGuiInitialized_ = true;
          LOG_ERROR("ResourceSystemAPI::SetDefaultFont: Failed to add Japanese "
                    "font '{}', using default",
                    fontPath);
        }
      } catch (const std::exception &e) {
        LOG_ERROR("ResourceSystemAPI::SetDefaultFont: Error initializing ImGui: {}",
                  e.what());
        if (!owner_->imGuiInitialized_) {
          owner_->imGuiInitialized_ = true;
        }
      }
    }
  } else {
    LOG_WARN("ResourceSystemAPI::SetDefaultFont: Failed to load font '{}', using "
             "Raylib default",
             name);
  }
}

void *ResourceSystemAPI::GetDefaultFont() const {
  return owner_->defaultFont_.get();
}

int ResourceSystemAPI::ScanResourceFiles() {
  owner_->resourceFileList_.clear();
  owner_->currentResourceIndex_ = 0;
  owner_->registeredTextureKeys_.clear();
  owner_->assetLicenses_.clear();

  try {
    ScanDirectory("data/assets/fonts", ResourceType::Font, {".ttf"});

    ScanDirectory("data/assets/textures", ResourceType::Texture, {".png"});
    ScanDirectoryRecursive("data/assets/characters", ResourceType::Texture,
                           {".png"});
    ScanDirectoryRecursive("data/assets/other", ResourceType::Texture,
                           {".png"});

    ScanDirectoryRecursive("data/assets/sounds", ResourceType::Sound,
                           {".wav", ".ogg"});
    ScanDirectoryRecursive("data/assets/other/kenney_ui-pack/Sounds",
                           ResourceType::Sound, {".ogg", ".wav"});

    ScanDirectory("data", ResourceType::Json, {".json"});

    ScanAssetLicenses();

    owner_->scanningCompleted_ = true;
    LOG_INFO("ResourceSystemAPI: Scanned {} resource files",
             owner_->resourceFileList_.size());
    return static_cast<int>(owner_->resourceFileList_.size());
  } catch (const std::exception &e) {
    LOG_ERROR("ResourceSystemAPI: Error scanning resource files: {}", e.what());
    return 0;
  }
}

bool ResourceSystemAPI::LoadNextResource(ProgressCallback callback) {
  if (owner_->currentResourceIndex_ >= owner_->resourceFileList_.size()) {
    return false;
  }

  const auto &fileInfo =
      owner_->resourceFileList_[owner_->currentResourceIndex_];
  std::string message;

  try {
    switch (fileInfo.type) {
    case ResourceType::Font:
      LoadFont(fileInfo.path, fileInfo.name);
      message = "Loading font: " + fileInfo.path;
      break;
    case ResourceType::Texture:
      LoadTexture(fileInfo.path, fileInfo.name);
      message = "Loading texture: " + fileInfo.path;
      break;
    case ResourceType::Sound:
      LoadSound(fileInfo.path, fileInfo.name);
      message = "Loading sound: " + fileInfo.path;
      break;
    case ResourceType::Json:
      LoadJson(fileInfo.path, fileInfo.name);
      message = "Loading json: " + fileInfo.path;
      break;
    }

    owner_->currentResourceIndex_++;

    if (callback) {
      LoadProgress progress;
      progress.current = static_cast<int>(owner_->currentResourceIndex_);
      progress.total = static_cast<int>(owner_->resourceFileList_.size());
      progress.message = message;
      callback(progress);
    }

    if (owner_->currentResourceIndex_ >= owner_->resourceFileList_.size()) {
      LOG_INFO("ResourceSystemAPI: Resource loading completed. textures={}, "
               "sounds={}, musics={}, fonts={}",
               owner_->textures_.size(), owner_->sounds_.size(),
               owner_->musics_.size(), owner_->fonts_.size());
    }

    return true;
  } catch (const std::exception &e) {
    LOG_WARN("ResourceSystemAPI: Failed to load resource {}: {}", fileInfo.path,
             e.what());
    owner_->currentResourceIndex_++;
    return true; // error, skipped
  }
}

bool ResourceSystemAPI::HasMoreResources() const {
  return owner_->currentResourceIndex_ < owner_->resourceFileList_.size();
}

LoadProgress ResourceSystemAPI::GetCurrentProgress() const {
  LoadProgress progress;
  progress.current = static_cast<int>(owner_->currentResourceIndex_);
  progress.total = static_cast<int>(owner_->resourceFileList_.size());
  std::string message;

  if (owner_->currentResourceIndex_ < owner_->resourceFileList_.size()) {
    const auto &fileInfo =
        owner_->resourceFileList_[owner_->currentResourceIndex_];
    switch (fileInfo.type) {
    case ResourceType::Font:
      message = "Loading font: " + fileInfo.path;
      break;
    case ResourceType::Texture:
      message = "Loading texture: " + fileInfo.path;
      break;
    case ResourceType::Sound:
      message = "Loading sound: " + fileInfo.path;
      break;
    case ResourceType::Json:
      message = "Loading json: " + fileInfo.path;
      break;
    }
    progress.message = message;
  } else {
    progress.message = "Resource loading completed";
  }

  return progress;
}

void ResourceSystemAPI::ResetLoadingState() {
  owner_->resourceFileList_.clear();
  owner_->currentResourceIndex_ = 0;
  owner_->scanningCompleted_ = false;
}

void ResourceSystemAPI::ScanDirectory(const std::string &dirPath, ResourceType type,
                                  const std::vector<std::string> &extensions) {
  if (!std::filesystem::exists(dirPath)) {
    return;
  }

  try {
    for (const auto &entry : std::filesystem::directory_iterator(dirPath)) {
      if (entry.is_regular_file()) {
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (std::find(extensions.begin(), extensions.end(), ext) !=
            extensions.end()) {
          ResourceFileInfo info;
          info.type = type;
          info.path = entry.path().generic_string();

          if (type == ResourceType::Texture) {
            info.name = MakeAssetsRelativeKey(entry.path());
            info.name = NormalizeTextureKey(info.name);
            owner_->registeredTextureKeys_.insert(info.name);
          } else {
            info.name = entry.path().stem().string();
          }
          owner_->resourceFileList_.push_back(info);
        }
      }
    }
  } catch (const std::exception &e) {
    LOG_WARN("ResourceSystemAPI: Error scanning directory {}: {}", dirPath,
             e.what());
  }
}

void ResourceSystemAPI::ScanDirectoryRecursive(
    const std::string &dirPath, ResourceType type,
    const std::vector<std::string> &extensions) {
  if (!std::filesystem::exists(dirPath)) {
    return;
  }

  try {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(dirPath)) {
      if (entry.is_regular_file()) {
        std::string ext = entry.path().extension().string();
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (std::find(extensions.begin(), extensions.end(), ext) !=
            extensions.end()) {
          ResourceFileInfo info;
          info.type = type;
          info.path = entry.path().generic_string();

          if (type == ResourceType::Texture) {
            info.name = MakeAssetsRelativeKey(entry.path());
            info.name = NormalizeTextureKey(info.name);
            owner_->registeredTextureKeys_.insert(info.name);
          } else {
            info.name = entry.path().stem().string();
          }
          owner_->resourceFileList_.push_back(info);
        }
      }
    }
  } catch (const std::exception &e) {
    LOG_WARN("ResourceSystemAPI: Error scanning directory recursively {}: {}",
             dirPath, e.what());
  }
}

void ResourceSystemAPI::LoadFont(const std::string &path, const std::string &name) {
  LOG_DEBUG("Font loaded: {}", path);
}

void ResourceSystemAPI::ScanAssetLicenses() {
  const std::filesystem::path basePath("data/assets/other");
  if (!std::filesystem::exists(basePath)) {
    return;
  }

  try {
    for (const auto &entry :
         std::filesystem::recursive_directory_iterator(basePath)) {
      if (!entry.is_regular_file()) {
        continue;
      }

      const std::string filename = ToLower(entry.path().filename().string());
      if (filename != "license.txt") {
        continue;
      }

      std::ifstream input(entry.path());
      if (!input.is_open()) {
        LOG_WARN("ResourceSystemAPI: Failed to open license file {}",
                 entry.path().generic_string());
        continue;
      }

      std::string content((std::istreambuf_iterator<char>(input)),
                          std::istreambuf_iterator<char>());
      if (content.empty()) {
        LOG_WARN("ResourceSystemAPI: License file is empty {}",
                 entry.path().generic_string());
        continue;
      }

      AssetLicenseEntry license;
      license.licenseText = content;
      license.sourcePath = entry.path().generic_string();

      std::string packName;
      try {
        const std::filesystem::path rel =
            std::filesystem::relative(entry.path(), basePath);
        if (rel.begin() != rel.end()) {
          packName = (*rel.begin()).string();
        }
      } catch (...) {
        packName.clear();
      }
      if (packName.empty()) {
        packName = entry.path().parent_path().filename().string();
      }
      license.packName = packName;

      owner_->assetLicenses_.push_back(license);
    }

    std::sort(owner_->assetLicenses_.begin(), owner_->assetLicenses_.end(),
              [](const AssetLicenseEntry &a, const AssetLicenseEntry &b) {
                return a.packName < b.packName;
              });
  } catch (const std::exception &e) {
    LOG_WARN("ResourceSystemAPI: Failed to scan asset licenses: {}", e.what());
  }
}

void ResourceSystemAPI::LoadTexture(const std::string &path,
                                const std::string &name) {
  const std::string key = NormalizeTextureKey(name);

  if (owner_->textures_.find(key) != owner_->textures_.end()) {
    return;
  }

  Texture2D texture = ::LoadTexture(path.c_str());

  if (texture.id == 0) {
    LOG_WARN("Failed to load texture: {}, creating placeholder", path);
    texture = CreatePlaceholderTexture(name);
  }

  auto texturePtr =
      std::shared_ptr<Texture2D>(new Texture2D(texture), [](Texture2D *t) {
        if (t && t->id != 0) {
          UnloadTexture(*t);
        }
        delete t;
      });

  owner_->textures_[key] = texturePtr;

  if (StartsWith(key, "assets/textures/")) {
    const std::filesystem::path p(key);
    const std::string filename = p.filename().generic_string(); // e.g. foo.png
    const std::string stem = p.stem().string();                 // e.g. foo

    if (!stem.empty() && owner_->textures_.find(stem) == owner_->textures_.end()) {
      owner_->textures_[stem] = texturePtr;
    } else if (!stem.empty()) {
      LOG_DEBUG("ResourceSystemAPI: texture alias collision (stem): {}", stem);
    }

    if (!filename.empty() &&
        owner_->textures_.find(filename) == owner_->textures_.end()) {
      owner_->textures_[filename] = texturePtr;
    } else if (!filename.empty()) {
      LOG_DEBUG("ResourceSystemAPI: texture alias collision (filename): {}",
                filename);
    }
  }
}

void ResourceSystemAPI::LoadSound(const std::string &path,
                              const std::string &name) {
  std::string ext = std::filesystem::path(path).extension().string();
  std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

  if (ext == ".mp3") {
    if (owner_->musics_.find(name) != owner_->musics_.end()) {
      return;
    }

    Music music = ::LoadMusicStream(path.c_str());
    if (music.frameCount == 0) {
      LOG_WARN("Failed to load music: {}", path);
      return;
    }

    auto musicPtr = std::shared_ptr<Music>(new Music(music), [](Music *m) {
      if (m && m->frameCount != 0) {
        UnloadMusicStream(*m);
      }
      delete m;
    });

    owner_->musics_[name] = musicPtr;
  } else if (ext == ".wav" || ext == ".ogg") {
    if (owner_->sounds_.find(name) != owner_->sounds_.end()) {
      return;
    }

    Sound sound = ::LoadSound(path.c_str());
    if (sound.frameCount == 0) {
      LOG_WARN("Failed to load sound: {}", path);
      return;
    }

    auto soundPtr = std::shared_ptr<Sound>(new Sound(sound), [](Sound *s) {
      if (s && s->frameCount != 0) {
        UnloadSound(*s);
      }
      delete s;
    });

    owner_->sounds_[name] = soundPtr;
    if (path.find("kenney_ui-pack/Sounds") != std::string::npos &&
        name == "click-a") {
      const std::string aliasName = "button_click";
      if (owner_->sounds_.find(aliasName) == owner_->sounds_.end()) {
        owner_->sounds_[aliasName] = soundPtr;
      }
    }
  }
}

void ResourceSystemAPI::LoadJson(const std::string &path, const std::string &name) {
  LOG_DEBUG("JSON loaded: {}", path);
}

void BaseSystemAPI::GenerateFontCodepoints() {
  if (!fontCodepoints_.empty())
    return;

  // ASCII (0x20 - 0x7E)
  for (int i = 0x20; i <= 0x7E; ++i)
    fontCodepoints_.push_back(i);

  // Latin-1 Supplement (U+00A0-U+00FF) - includes ´
  for (int i = 0x00A0; i <= 0x00FF; ++i)
    fontCodepoints_.push_back(i);

  // Greek and Coptic (U+0370-U+03FF) - includes ω
  for (int i = 0x0370; i <= 0x03FF; ++i)
    fontCodepoints_.push_back(i);

  for (int i = 0x3000; i <= 0x30FF; ++i)
    fontCodepoints_.push_back(i);

  for (int i = 0xFF00; i <= 0xFFEF; ++i)
    fontCodepoints_.push_back(i);

  for (int i = 0x4E00; i <= 0x9FAF; ++i)
    fontCodepoints_.push_back(i);

  // Arrows (U+2190-U+21FF)
  for (int i = 0x2190; i <= 0x21FF; ++i)
    fontCodepoints_.push_back(i);

  // General Punctuation (U+2000-U+206F)
  for (int i = 0x2000; i <= 0x206F; ++i)
    fontCodepoints_.push_back(i);

  // Miscellaneous Symbols (U+2600-U+26FF)
  for (int i = 0x2600; i <= 0x26FF; ++i)
    fontCodepoints_.push_back(i);

  // Dingbats (U+2700-U+27BF)
  for (int i = 0x2700; i <= 0x27BF; ++i)
    fontCodepoints_.push_back(i);

  // Miscellaneous Symbols and Pictographs (U+1F300-U+1F9FF)
  for (int i = 0x1F300; i <= 0x1F9FF; ++i)
    fontCodepoints_.push_back(i);

  // Supplemental Symbols and Pictographs (U+1FA00-U+1FAFF)
  for (int i = 0x1FA00; i <= 0x1FAFF; ++i)
    fontCodepoints_.push_back(i);

  LOG_INFO("Generated font codepoints: {} characters (including emoji ranges)",
           fontCodepoints_.size());
}

Texture2D ResourceSystemAPI::CreatePlaceholderTexture(const std::string &name) {
  const int size = 64;
  Image image = GenImageColor(size, size, MAGENTA);

  for (int y = 0; y < size; ++y) {
    for (int x = 0; x < size; ++x) {
      Color color = ((x / 8 + y / 8) % 2 == 0) ? MAGENTA : YELLOW;
      ImageDrawPixel(&image, x, y, color);
    }
  }

  Texture2D texture = LoadTextureFromImage(image);
  UnloadImage(image);

  LOG_INFO("Created placeholder texture for: {}", name);
  return texture;
}

Color RenderSystemAPI::GetReadableTextColor(const std::string &textureKey,
                                            float luminanceThreshold) {
  const std::string key = NormalizeTextureKey(textureKey);
  auto cached = owner_->textureTextColorCache_.find(key);
  if (cached != owner_->textureTextColorCache_.end()) {
    return cached->second;
  }

  float luminance = 0.0f;
  auto lumIt = owner_->textureLuminanceCache_.find(key);
  if (lumIt != owner_->textureLuminanceCache_.end()) {
    luminance = lumIt->second;
  } else {
    luminance = owner_->CalculateTextureLuminance(key);
    owner_->textureLuminanceCache_[key] = luminance;
  }

  Color color = (luminance >= luminanceThreshold) ? BLACK : WHITE;
  owner_->textureTextColorCache_[key] = color;
  return color;
}

float BaseSystemAPI::CalculateTextureLuminance(const std::string &textureKey) {
  const std::string path = ResolveTexturePath(textureKey);
  if (path.empty() || !FileExists(path.c_str())) {
    LOG_WARN("RenderSystemAPI: Texture not found for luminance {}", textureKey);
    return 0.0f;
  }

  Image image = LoadImage(path.c_str());
  if (!image.data) {
    LOG_WARN("RenderSystemAPI: Failed to load image for luminance {}", path);
    return 0.0f;
  }

  Color *pixels = LoadImageColors(image);
  if (!pixels) {
    UnloadImage(image);
    return 0.0f;
  }

  const int count = image.width * image.height;
  double sum = 0.0;
  int samples = 0;
  for (int i = 0; i < count; ++i) {
    const Color c = pixels[i];
    if (c.a == 0) {
      continue;
    }
    const double r = static_cast<double>(c.r) / 255.0;
    const double g = static_cast<double>(c.g) / 255.0;
    const double b = static_cast<double>(c.b) / 255.0;
    const double lum = 0.2126 * r + 0.7152 * g + 0.0722 * b;
    sum += lum;
    samples++;
  }

  UnloadImageColors(pixels);
  UnloadImage(image);

  if (samples == 0) {
    return 0.0f;
  }
  return static_cast<float>(sum / static_cast<double>(samples));
}

std::string
BaseSystemAPI::ResolveTexturePath(const std::string &textureKey) const {
  const std::string key = NormalizeTextureKey(textureKey);
  std::string path;
  if (key.length() >= 7 && key.substr(0, 7) == "assets/") {
    path = "data/" + key;
  } else {
    path = "data/assets/textures/" + key;
  }
  if (path.length() < 4 || path.substr(path.length() - 4) != ".png") {
    path += ".png";
  }
  return path;
}

} // namespace core
} // namespace game

