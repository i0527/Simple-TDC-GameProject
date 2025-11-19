#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <raylib.h>

namespace Resources {
    // リソース管理システムの例外クラス
    class ResourceException : public std::exception {
    public:
        explicit ResourceException(const std::string& message) : message_(message) {}
        
        const char* what() const noexcept override {
            return message_.c_str();
        }
        
    private:
        std::string message_;
    };

    // テクスチャリソース管理
    class TextureManager {
    public:
        TextureManager() = default;
        ~TextureManager();
        
        // テクスチャをファイルから読み込み
        void LoadTexture(const std::string& name, const std::string& filePath);
        
        // キー名でテクスチャを取得
        Texture2D GetTexture(const std::string& name) const;
        
        // テクスチャが読み込まれているか確認
        bool HasTexture(const std::string& name) const;
        
        // すべてのテクスチャをアンロード
        void UnloadAll();

        // Imageから生成したTextureを登録する（ImageManagerから呼び出す）
        void AddTexture(const std::string& name, const Texture2D& texture);
        
    private:
        std::unordered_map<std::string, Texture2D> textures_;
    };

    // フォントリソース管理
    class FontManager {
    public:
        FontManager() = default;
        ~FontManager();
        
        // フォントをファイルから読み込み
        void LoadFont(const std::string& name, const std::string& filePath);
        
		// 日本語対応のためのフォント読み込み
		void LoadFontEx(const std::string& name, const std::string& filePath, int fontSize, const int* glyphs, int glyphCount);

        // キー名でフォントを取得
        Font GetFont(const std::string& name) const;
        
        // フォントが読み込まれているか確認
        bool HasFont(const std::string& name) const;
        
        // すべてのフォントをアンロード
        void UnloadAll();
        
    private:
        std::unordered_map<std::string, Font> fonts_;
    };

    // サウンドリソース管理
    class SoundManager {
    public:
        SoundManager() = default;
        ~SoundManager();
        
        // サウンドをファイルから読み込み
        void LoadSound(const std::string& name, const std::string& filePath);
        
        // キー名でサウンドを取得
        Sound GetSound(const std::string& name) const;
        
        // サウンドが読み込まれているか確認
        bool HasSound(const std::string& name) const;
        
        // サウンドを再生
        void PlaySound(const std::string& name);
        
        // すべてのサウンドをアンロード
        void UnloadAll();
        
    private:
        std::unordered_map<std::string, Sound> sounds_;
    };

    // ミュージック（音楽）リソース管理
    class MusicManager {
    public:
        MusicManager() = default;
        ~MusicManager();
        
        // ミュージックをファイルから読み込み
        void LoadMusic(const std::string& name, const std::string& filePath);
        
        // キー名でミュージックを取得
        Music GetMusic(const std::string& name) const;
        
        // ミュージックが読み込まれているか確認
        bool HasMusic(const std::string& name) const;
        
        // ミュージックを再生
        void PlayMusic(const std::string& name);
        
        // ミュージックを一時停止
        void PauseMusic(const std::string& name);
        
        // ミュージックを再開
        void ResumeMusic(const std::string& name);
        
        // ミュージックを停止
        void StopMusic(const std::string& name);
        
        // ミュージックの音量を設定（0.0f - 1.0f）
        void SetMusicVolume(const std::string& name, float volume);
        
        // すべてのミュージックをアンロード
        void UnloadAll();
        
    private:
        std::unordered_map<std::string, Music> music_;
    };

    // シェーダーリソース管理
    class ShaderManager {
    public:
        ShaderManager() = default;
        ~ShaderManager();
        
        // シェーダーを読み込み（頂点シェーダー、フラグメントシェーダー）
        void LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath);
        
        // デフォルトシェーダーを読み込み（fsPathのみ）
        void LoadShaderFromFile(const std::string& name, const std::string& fsPath);
        
        // キー名でシェーダーを取得
        Shader GetShader(const std::string& name) const;
        
        // シェーダーが読み込まれているか確認
        bool HasShader(const std::string& name) const;
        
        // シェーダーのロケーション取得（uniform変数など）
        int GetShaderLocation(const std::string& name, const std::string& uniformName);
        
        // シェーダーの浮動小数点値を設定
        void SetShaderValue(const std::string& name, const std::string& uniformName, float value);
        
        // シェーダーの整数値を設定
        void SetShaderValueI(const std::string& name, const std::string& uniformName, int value);
        
        // すべてのシェーダーをアンロード
        void UnloadAll();
        
    private:
        std::unordered_map<std::string, Shader> shaders_;
        std::unordered_map<std::string, std::unordered_map<std::string, int>> shaderLocations_;
    };

    // フレーム情報構造体（スプライトシート用）
    struct FrameInfo {
        Rectangle rect;       // フレームの位置とサイズ
        int duration;         // フレーム持続時間（ミリ秒）
        std::string textureName;  // 関連するテクスチャ名
    };

    // 画像リソース管理（スプライトシート対応）
    class ImageManager {
    public:
        ImageManager() = default;
        ~ImageManager();
        
        // 画像を読み込み（CPU側）
        void LoadImage(const std::string& name, const std::string& filePath);
        
        // Aseprite JSON形式のスプライトシートを読み込み
        void LoadSpriteSheet(const std::string& name, const std::string& jsonPath, const std::string& imagePath);
        
        // キー名で画像を取得
        Image GetImage(const std::string& name) const;
        
        // 画像が読み込まれているか確認
        bool HasImage(const std::string& name) const;
        
        // フレーム情報を取得（スプライトシート用）
        FrameInfo GetFrameInfo(const std::string& frameName) const;
        
        // フレームが存在するか確認
        bool HasFrame(const std::string& frameName) const;
        
        // すべてのフレーム名を取得
        std::vector<std::string> GetAllFrameNames(const std::string& spriteName) const;
        
        // 画像をテクスチャに変換（GPU側へ）
        void ImageToTexture(const std::string& imageName, const std::string& textureName);
        
        // 画像をリサイズ
        void ResizeImage(const std::string& name, int width, int height);
        
        // 画像を反転
        void FlipImage(const std::string& name, bool horizontal);
        
        // すべての画像をアンロード
        void UnloadAll();
        
    private:
        std::unordered_map<std::string, Image> images_;
        std::unordered_map<std::string, FrameInfo> frames_;  // フレーム名 → フレーム情報
        std::unordered_map<std::string, std::vector<std::string>> spriteSheets_;  // スプライト名 → フレーム名リスト
    };

    // 統合リソース管理クラス
    class ResourceManager {
    public:
        static ResourceManager& GetInstance();
        
        // インスタンスのコピーを禁止
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        
        // テクスチャマネージャーへのアクセス
        TextureManager& GetTextureManager();
        const TextureManager& GetTextureManager() const;
        
        // フォントマネージャーへのアクセス
        FontManager& GetFontManager();
        const FontManager& GetFontManager() const;
        
        // サウンドマネージャーへのアクセス
        SoundManager& GetSoundManager();
        const SoundManager& GetSoundManager() const;
        
        // ミュージックマネージャーへのアクセス
        MusicManager& GetMusicManager();
        const MusicManager& GetMusicManager() const;
        
        // シェーダーマネージャーへのアクセス
        ShaderManager& GetShaderManager();
        const ShaderManager& GetShaderManager() const;
        
        // イメージマネージャーへのアクセス
        ImageManager& GetImageManager();
        const ImageManager& GetImageManager() const;
        
        // すべてのリソースをアンロード
        void UnloadAll();
        
    private:
        ResourceManager() = default;
        ~ResourceManager();
        
        TextureManager textureManager_;
        FontManager fontManager_;
        SoundManager soundManager_;
        MusicManager musicManager_;
        ShaderManager shaderManager_;
        ImageManager imageManager_;
    };
}
