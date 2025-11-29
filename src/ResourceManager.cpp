#include "ResourceManager.h"
#include <iostream>
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace Resources {
    // ==================== TextureManager ====================
    
    TextureManager::~TextureManager() {
        UnloadAll();
    }
    
    void TextureManager::LoadTexture(const std::string& name, const std::string& filePath) {
        // 既に読み込まれていないか確認
        if (HasTexture(name)) {
            std::cerr << "Texture already loaded: " << name << std::endl;
            return;
        }
        
        try {
            Texture2D texture = ::LoadTexture(filePath.c_str());
            
            // テクスチャが正常に読み込まれたか確認
            if (texture.id == 0) {
                throw ResourceException("Failed to load texture: " + filePath);
            }
            
            textures_[name] = texture;
            std::cout << "Texture loaded successfully: " << name << " (" << filePath << ")" << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    Texture2D TextureManager::GetTexture(const std::string& name) const {
        auto it = textures_.find(name);
        if (it == textures_.end()) {
            throw ResourceException("Texture not found: " + name);
        }
        return it->second;
    }
    
    bool TextureManager::HasTexture(const std::string& name) const {
        return textures_.find(name) != textures_.end();
    }
    
    void TextureManager::UnloadAll() {
        for (auto& pair : textures_) {
            ::UnloadTexture(pair.second);
        }
        textures_.clear();
        std::cout << "All textures unloaded" << std::endl;
    }

    void TextureManager::AddTexture(const std::string& name, const Texture2D& texture) {
        // 既に同名のテクスチャがあればアンロードして差し替え
        auto it = textures_.find(name);
        if (it != textures_.end()) {
            ::UnloadTexture(it->second);
            textures_.erase(it);
            std::cout << "Replaced existing texture: " << name << std::endl;
        }
        textures_[name] = texture;
        std::cout << "Texture added from image: " << name << std::endl;
    }
    
    // ==================== FontManager ====================
    
    FontManager::~FontManager() {
        UnloadAll();
    }
    
    void FontManager::LoadFont(const std::string& name, const std::string& filePath) {
        // 既に読み込まれていないか確認
        if (HasFont(name)) {
            std::cerr << "Font already loaded: " << name << std::endl;
            return;
        }
        
        try {
            Font font = ::LoadFont(filePath.c_str());
            
            // フォントが正常に読み込まれたか確認
            if (font.texture.id == 0) {
                throw ResourceException("Failed to load font: " + filePath);
            }
            
            fonts_[name] = font;
            std::cout << "Font loaded successfully: " << name << " (" << filePath << ")" << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    void FontManager::LoadFontEx(const std::string& name, const std::string& filePath, int fontSize, const int* glyphs, int glyphCount)
    {
        // 既に読み込まれていないか確認
        if (HasFont(name)) {
            std::cerr << "Font already loaded: " << name << std::endl;
            return;
        }
        
        try {
            // Raylib の LoadFontEx は const int* を受け取らないため、const_cast が必要
            Font font = ::LoadFontEx(filePath.c_str(), fontSize, const_cast<int*>(glyphs), glyphCount);
            
            // フォントが正常に読み込まれたか確認
            if (font.texture.id == 0) {
                throw ResourceException("Failed to load font: " + filePath);
            }
            
            fonts_[name] = font;
            std::cout << "Font loaded successfully: " << name << " (" << filePath << ")" << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
		}
    }
    
    Font FontManager::GetFont(const std::string& name) const {
        auto it = fonts_.find(name);
        if (it == fonts_.end()) {
            throw ResourceException("Font not found: " + name);
        }
        return it->second;
    }
    
    bool FontManager::HasFont(const std::string& name) const {
        return fonts_.find(name) != fonts_.end();
    }
    
    void FontManager::UnloadAll() {
        for (auto& pair : fonts_) {
            ::UnloadFont(pair.second);
        }
        fonts_.clear();
        std::cout << "All fonts unloaded" << std::endl;
    }
    
    // ==================== SoundManager ====================
    
    SoundManager::~SoundManager() {
        UnloadAll();
    }
    
    void SoundManager::LoadSound(const std::string& name, const std::string& filePath) {
        // 既に読み込まれていないか確認
        if (HasSound(name)) {
            std::cerr << "Sound already loaded: " << name << std::endl;
            return;
        }
        
        try {
            Sound sound = ::LoadSound(filePath.c_str());
            
            // サウンドが正常に読み込まれたか確認
            if (sound.frameCount == 0) {
                throw ResourceException("Failed to load sound: " + filePath);
            }
            
            sounds_[name] = sound;
            std::cout << "Sound loaded successfully: " << name << " (" << filePath << ")" << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    Sound SoundManager::GetSound(const std::string& name) const {
        auto it = sounds_.find(name);
        if (it == sounds_.end()) {
            throw ResourceException("Sound not found: " + name);
        }
        return it->second;
    }
    
    bool SoundManager::HasSound(const std::string& name) const {
        return sounds_.find(name) != sounds_.end();
    }
    
    void SoundManager::PlaySound(const std::string& name) {
        try {
            Sound sound = GetSound(name);
            ::PlaySound(sound);
            std::cout << "Sound playing: " << name << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void SoundManager::UnloadAll() {
        for (auto& pair : sounds_) {
            ::UnloadSound(pair.second);
        }
        sounds_.clear();
        std::cout << "All sounds unloaded" << std::endl;
    }
    
    // ==================== MusicManager ====================
    
    MusicManager::~MusicManager() {
        UnloadAll();
    }
    
    void MusicManager::LoadMusic(const std::string& name, const std::string& filePath) {
        // 既に読み込まれていないか確認
        if (HasMusic(name)) {
            std::cerr << "Music already loaded: " << name << std::endl;
            return;
        }
        
        try {
            Music music = ::LoadMusicStream(filePath.c_str());
            
            // ミュージックが正常に読み込まれたか確認
            if (music.frameCount == 0) {
                throw ResourceException("Failed to load music: " + filePath);
            }
            
            music_[name] = music;
            std::cout << "Music loaded successfully: " << name << " (" << filePath << ")" << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    Music MusicManager::GetMusic(const std::string& name) const {
        auto it = music_.find(name);
        if (it == music_.end()) {
            throw ResourceException("Music not found: " + name);
        }
        return it->second;
    }
    
    bool MusicManager::HasMusic(const std::string& name) const {
        return music_.find(name) != music_.end();
    }
    
    void MusicManager::PlayMusic(const std::string& name) {
        try {
            Music& music = music_.at(name);
            ::PlayMusicStream(music);
            std::cout << "Music playing: " << name << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Resource error: Music not found: " << name << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void MusicManager::PauseMusic(const std::string& name) {
        try {
            Music& music = music_.at(name);
            ::PauseMusicStream(music);
            std::cout << "Music paused: " << name << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Resource error: Music not found: " << name << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void MusicManager::ResumeMusic(const std::string& name) {
        try {
            Music& music = music_.at(name);
            ::ResumeMusicStream(music);
            std::cout << "Music resumed: " << name << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Resource error: Music not found: " << name << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void MusicManager::StopMusic(const std::string& name) {
        try {
            Music& music = music_.at(name);
            ::StopMusicStream(music);
            std::cout << "Music stopped: " << name << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Resource error: Music not found: " << name << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void MusicManager::SetMusicVolume(const std::string& name, float volume) {
        try {
            Music& music = music_.at(name);
            ::SetMusicVolume(music, volume);
            std::cout << "Music volume set: " << name << " (" << volume << ")" << std::endl;
        } catch (const std::out_of_range& e) {
            std::cerr << "Resource error: Music not found: " << name << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void MusicManager::UnloadAll() {
        for (auto& pair : music_) {
            ::UnloadMusicStream(pair.second);
        }
        music_.clear();
        std::cout << "All music unloaded" << std::endl;
    }

    // ==================== ShaderManager ====================
    
    ShaderManager::~ShaderManager() {
        UnloadAll();
    }
    
    void ShaderManager::LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath) {
        // 既に読み込まれていないか確認
        if (HasShader(name)) {
            std::cerr << "Shader already loaded: " << name << std::endl;
            return;
        }
        
        try {
            // vsPathが空の場合はNULLポインタを使用
            const char* vsPathPtr = vsPath.empty() ? nullptr : vsPath.c_str();
            const char* fsPathPtr = fsPath.empty() ? nullptr : fsPath.c_str();
            
            Shader shader = ::LoadShader(vsPathPtr, fsPathPtr);
            
            // シェーダーが正常に読み込まれたか確認
            if (shader.id == 0) {
                throw ResourceException("Failed to load shader: " + name);
            }
            
            shaders_[name] = shader;
            std::cout << "Shader loaded successfully: " << name << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Shader error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void ShaderManager::LoadShaderFromFile(const std::string& name, const std::string& fsPath) {
        LoadShader(name, "", fsPath);
    }
    
    Shader ShaderManager::GetShader(const std::string& name) const {
        auto it = shaders_.find(name);
        if (it == shaders_.end()) {
            throw ResourceException("Shader not found: " + name);
        }
        return it->second;
    }
    
    bool ShaderManager::HasShader(const std::string& name) const {
        return shaders_.find(name) != shaders_.end();
    }
    
    int ShaderManager::GetShaderLocation(const std::string& name, const std::string& uniformName) {
        try {
            Shader shader = GetShader(name);
            
            // ロケーションがキャッシュされているか確認
            auto locationIt = shaderLocations_.find(name);
            if (locationIt != shaderLocations_.end()) {
                auto uniformIt = locationIt->second.find(uniformName);
                if (uniformIt != locationIt->second.end()) {
                    return uniformIt->second;
                }
            }
            
            // ロケーションを取得してキャッシュ
            int location = ::GetShaderLocation(shader, uniformName.c_str());
            shaderLocations_[name][uniformName] = location;
            
            return location;
        } catch (const ResourceException& e) {
            std::cerr << "Shader error: " << e.what() << std::endl;
            return -1;
        }
    }
    
    void ShaderManager::SetShaderValue(const std::string& name, const std::string& uniformName, float value) {
        try {
            Shader shader = GetShader(name);
            int location = GetShaderLocation(name, uniformName);
            
            if (location >= 0) {
                ::SetShaderValue(shader, location, &value, SHADER_UNIFORM_FLOAT);
            }
        } catch (const ResourceException& e) {
            std::cerr << "Shader error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void ShaderManager::SetShaderValueI(const std::string& name, const std::string& uniformName, int value) {
        try {
            Shader shader = GetShader(name);
            int location = GetShaderLocation(name, uniformName);
            
            if (location >= 0) {
                ::SetShaderValue(shader, location, &value, SHADER_UNIFORM_INT);
            }
        } catch (const ResourceException& e) {
            std::cerr << "Shader error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void ShaderManager::UnloadAll() {
        for (auto& pair : shaders_) {
            ::UnloadShader(pair.second);
        }
        shaders_.clear();
        shaderLocations_.clear();
        std::cout << "All shaders unloaded" << std::endl;
    }

    // ==================== ImageManager ====================
    
    ImageManager::~ImageManager() {
        UnloadAll();
    }
    
    void ImageManager::LoadImage(const std::string& name, const std::string& filePath) {
        // 既に読み込まれていないか確認
        if (HasImage(name)) {
            std::cerr << "Image already loaded: " << name << std::endl;
            return;
        }
        
        try {
            Image image = ::LoadImage(filePath.c_str());
            
            // 画像が正常に読み込まれたか確認
            if (image.data == nullptr) {
                throw ResourceException("Failed to load image: " + filePath);
            }
            
            images_[name] = image;
            std::cout << "Image loaded successfully: " << name << " (" << filePath << ")" << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void ImageManager::LoadSpriteSheet(const std::string& name, const std::string& jsonPath, const std::string& imagePath) {
        try {
            // JSON ファイルを読み込み
            std::ifstream jsonFile(jsonPath);
            if (!jsonFile.is_open()) {
                throw ResourceException("Failed to open JSON file: " + jsonPath);
            }
            
            json spriteData = json::parse(jsonFile);
            
            // メタデータからテクスチャ情報を取得
            if (!spriteData.contains("meta") || !spriteData.contains("frames")) {
                throw ResourceException("Invalid sprite sheet JSON format: " + jsonPath);
            }
            
            // 画像を読み込み
            LoadImage(name, imagePath);
            
            // フレーム情報を解析
            const auto& frames = spriteData["frames"];
            std::vector<std::string> frameNames;
            
            // フレーム数をチェック
            int frameCount = frames.size();
            std::cout << "LoadSpriteSheet: Detected " << frameCount << " frame(s) in JSON" << std::endl;
            
            // 複数フレーム：通常解析
            for (auto it = frames.begin(); it != frames.end(); ++it) {
                const std::string& frameName = it.key();
                const auto& frameData = it.value();
                
                // フレーム情報を抽出
                int x = frameData["frame"]["x"];
                int y = frameData["frame"]["y"];
                int w = frameData["frame"]["w"];
                int h = frameData["frame"]["h"];
                int duration = frameData.value("duration", 100);  // デフォルト100ms
                
                FrameInfo frameInfo;
                frameInfo.rect = { static_cast<float>(x), static_cast<float>(y), 
                                  static_cast<float>(w), static_cast<float>(h) };
                frameInfo.duration = duration;
                frameInfo.textureName = name;
                
                frames_[frameName] = frameInfo;
                frameNames.push_back(frameName);
                
                std::cout << "Frame loaded: " << frameName 
                         << " [" << x << ", " << y << ", " << w << ", " << h << "]"
                         << " duration: " << duration << "ms" << std::endl;
            }
            
            // スプライトシート情報を保存
            spriteSheets_[name] = frameNames;
            
            // 画像からテクスチャを生成して TextureManager に登録
            ImageToTexture(name, name);
            
            std::cout << "Sprite sheet loaded: " << name << " with " << frameNames.size() << " frames" << std::endl;
            
        } catch (const json::parse_error& e) {
            std::cerr << "JSON parse error: " << e.what() << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    Image ImageManager::GetImage(const std::string& name) const {
        auto it = images_.find(name);
        if (it == images_.end()) {
            throw ResourceException("Image not found: " + name);
        }
        return it->second;
    }
    
    bool ImageManager::HasImage(const std::string& name) const {
        return images_.find(name) != images_.end();
    }
    
    FrameInfo ImageManager::GetFrameInfo(const std::string& frameName) const {
        auto it = frames_.find(frameName);
        if (it == frames_.end()) {
            throw ResourceException("Frame not found: " + frameName);
        }
        return it->second;
    }
    
    bool ImageManager::HasFrame(const std::string& frameName) const {
        return frames_.find(frameName) != frames_.end();
    }
    
    std::vector<std::string> ImageManager::GetAllFrameNames(const std::string& spriteName) const {
        auto it = spriteSheets_.find(spriteName);
        if (it == spriteSheets_.end()) {
            std::cerr << "Sprite sheet not found: " << spriteName << std::endl;
            return std::vector<std::string>();
        }
        return it->second;
    }
    
    void ImageManager::LoadAllSpriteSheets(const std::string& jsonDir, const std::string& atlasDir) {
        std::cout << "Loading all sprite sheets from: " << jsonDir << std::endl;
        
        // assets/json/ ディレクトリ内の全 .json ファイルを検索
        // Raylibには DirectoryGetFiles がないため、手動で各ファイルを指定
        // 実際には以下のようなキャラクター名リストを使用
        std::vector<std::string> characterNames = {
            "bard", "cupslime", "kame", "killer_whale", 
            "seahorse", "whale", "yodarehaki"
        };
        
        for (const auto& charName : characterNames) {
            std::string jsonPath = jsonDir + "/" + charName + ".json";
            std::string imagePath = atlasDir + "/" + charName + ".png";
            
            // ファイルが存在するか確認
            if (FileExists(jsonPath.c_str()) && FileExists(imagePath.c_str())) {
                LoadSpriteSheet(charName, jsonPath, imagePath);
            } else {
                std::cout << "Skipping " << charName << " (files not found)" << std::endl;
            }
        }
        
        std::cout << "Finished loading sprite sheets" << std::endl;
    }
    
    std::vector<std::string> ImageManager::GetAllSpriteSheetNames() const {
        std::vector<std::string> names;
        names.reserve(spriteSheets_.size());
        
        for (const auto& pair : spriteSheets_) {
            names.push_back(pair.first);
        }
        
        return names;
    }
    
    void ImageManager::ImageToTexture(const std::string& imageName, const std::string& textureName) {
        try {
            Image image = GetImage(imageName);
            
            // 画像をテクスチャに変換
            Texture2D texture = ::LoadTextureFromImage(image);
            
            if (texture.id == 0) {
                throw ResourceException("Failed to convert image to texture: " + imageName);
            }

            // TextureManager に登録して管理させる
            ResourceManager::GetInstance().GetTextureManager().AddTexture(textureName, texture);
            
            std::cout << "Image converted to texture: " << imageName << " -> " << textureName << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void ImageManager::ResizeImage(const std::string& name, int width, int height) {
        try {
            auto it = images_.find(name);
            if (it == images_.end()) {
                throw ResourceException("Image not found: " + name);
            }
            
            ::ImageResize(&it->second, width, height);
            std::cout << "Image resized: " << name << " to " << width << "x" << height << std::endl;
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void ImageManager::FlipImage(const std::string& name, bool horizontal) {
        try {
            auto it = images_.find(name);
            if (it == images_.end()) {
                throw ResourceException("Image not found: " + name);
            }
            
            if (horizontal) {
                ::ImageFlipHorizontal(&it->second);
                std::cout << "Image flipped horizontally: " << name << std::endl;
            } else {
                ::ImageFlipVertical(&it->second);
                std::cout << "Image flipped vertically: " << name << std::endl;
            }
        } catch (const ResourceException& e) {
            std::cerr << "Resource error: " << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    
    void ImageManager::UnloadAll() {
        for (auto& pair : images_) {
            ::UnloadImage(pair.second);
        }
        images_.clear();
        frames_.clear();
        spriteSheets_.clear();
        std::cout << "All images unloaded" << std::endl;
    }

    // ==================== ResourceManager ====================
    
    ResourceManager& ResourceManager::GetInstance() {
        static ResourceManager instance;
        return instance;
    }
    
    ResourceManager::~ResourceManager() {
        UnloadAll();
    }
    
    TextureManager& ResourceManager::GetTextureManager() {
        return textureManager_;
    }
    
    const TextureManager& ResourceManager::GetTextureManager() const {
        return textureManager_;
    }
    
    FontManager& ResourceManager::GetFontManager() {
        return fontManager_;
    }
    
    const FontManager& ResourceManager::GetFontManager() const {
        return fontManager_;
    }
    
    SoundManager& ResourceManager::GetSoundManager() {
        return soundManager_;
    }
    
    const SoundManager& ResourceManager::GetSoundManager() const {
        return soundManager_;
    }
    
    MusicManager& ResourceManager::GetMusicManager() {
        return musicManager_;
    }
    
    const MusicManager& ResourceManager::GetMusicManager() const {
        return musicManager_;
    }
    
    ShaderManager& ResourceManager::GetShaderManager() {
        return shaderManager_;
    }
    
    const ShaderManager& ResourceManager::GetShaderManager() const {
        return shaderManager_;
    }
    
    ImageManager& ResourceManager::GetImageManager() {
        return imageManager_;
    }
    
    const ImageManager& ResourceManager::GetImageManager() const {
        return imageManager_;
    }
    
    void ResourceManager::UnloadAll() {
        textureManager_.UnloadAll();
        fontManager_.UnloadAll();
        soundManager_.UnloadAll();
        musicManager_.UnloadAll();
        shaderManager_.UnloadAll();
        imageManager_.UnloadAll();
    }
}
