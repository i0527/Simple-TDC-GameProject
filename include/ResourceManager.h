#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <raylib.h>

namespace Resources {
    // ���\�[�X�Ǘ��V�X�e���̗�O�N���X
    class ResourceException : public std::exception {
    public:
        explicit ResourceException(const std::string& message) : message_(message) {}
        
        const char* what() const noexcept override {
            return message_.c_str();
        }
        
    private:
        std::string message_;
    };

    // �e�N�X�`�����\�[�X�Ǘ�
    class TextureManager {
    public:
        TextureManager() = default;
        ~TextureManager();
        
        // �e�N�X�`�����t�@�C������ǂݍ���
        void LoadTexture(const std::string& name, const std::string& filePath);
        
        // �L�[���Ńe�N�X�`�����擾
        Texture2D GetTexture(const std::string& name) const;
        
        // �e�N�X�`�����ǂݍ��܂�Ă��邩�m�F
        bool HasTexture(const std::string& name) const;
        
        // ���ׂẴe�N�X�`�����A�����[�h
        void UnloadAll();

        // Image���琶������Texture��o�^����iImageManager����Ăяo���j
        void AddTexture(const std::string& name, const Texture2D& texture);
        
    private:
        std::unordered_map<std::string, Texture2D> textures_;
    };

    // �t�H���g���\�[�X�Ǘ�
    class FontManager {
    public:
        FontManager() = default;
        ~FontManager();
        
        // �t�H���g���t�@�C������ǂݍ���
        void LoadFont(const std::string& name, const std::string& filePath);
        
		// ���{��Ή��̂��߂̃t�H���g�ǂݍ���
		void LoadFontEx(const std::string& name, const std::string& filePath, int fontSize, const int* glyphs, int glyphCount);

        // �L�[���Ńt�H���g���擾
        Font GetFont(const std::string& name) const;
        
        // �t�H���g���ǂݍ��܂�Ă��邩�m�F
        bool HasFont(const std::string& name) const;
        
        // ���ׂẴt�H���g���A�����[�h
        void UnloadAll();
        
    private:
        std::unordered_map<std::string, Font> fonts_;
    };

    // �T�E���h���\�[�X�Ǘ�
    class SoundManager {
    public:
        SoundManager() = default;
        ~SoundManager();
        
        // �T�E���h���t�@�C������ǂݍ���
        void LoadSound(const std::string& name, const std::string& filePath);
        
        // �L�[���ŃT�E���h���擾
        Sound GetSound(const std::string& name) const;
        
        // �T�E���h���ǂݍ��܂�Ă��邩�m�F
        bool HasSound(const std::string& name) const;
        
        // �T�E���h���Đ�
        void PlaySound(const std::string& name);
        
        // ���ׂẴT�E���h���A�����[�h
        void UnloadAll();
        
    private:
        std::unordered_map<std::string, Sound> sounds_;
    };

    // �~���[�W�b�N�i���y�j���\�[�X�Ǘ�
    class MusicManager {
    public:
        MusicManager() = default;
        ~MusicManager();
        
        // �~���[�W�b�N���t�@�C������ǂݍ���
        void LoadMusic(const std::string& name, const std::string& filePath);
        
        // �L�[���Ń~���[�W�b�N���擾
        Music GetMusic(const std::string& name) const;
        
        // �~���[�W�b�N���ǂݍ��܂�Ă��邩�m�F
        bool HasMusic(const std::string& name) const;
        
        // �~���[�W�b�N���Đ�
        void PlayMusic(const std::string& name);
        
        // �~���[�W�b�N���ꎞ��~
        void PauseMusic(const std::string& name);
        
        // �~���[�W�b�N���ĊJ
        void ResumeMusic(const std::string& name);
        
        // �~���[�W�b�N���~
        void StopMusic(const std::string& name);
        
        // �~���[�W�b�N�̉��ʂ�ݒ�i0.0f - 1.0f�j
        void SetMusicVolume(const std::string& name, float volume);
        
        // ���ׂẴ~���[�W�b�N���A�����[�h
        void UnloadAll();
        
    private:
        std::unordered_map<std::string, Music> music_;
    };

    // �V�F�[�_�[���\�[�X�Ǘ�
    class ShaderManager {
    public:
        ShaderManager() = default;
        ~ShaderManager();
        
        // �V�F�[�_�[��ǂݍ��݁i���_�V�F�[�_�[�A�t���O�����g�V�F�[�_�[�j
        void LoadShader(const std::string& name, const std::string& vsPath, const std::string& fsPath);
        
        // �f�t�H���g�V�F�[�_�[��ǂݍ��݁ifsPath�̂݁j
        void LoadShaderFromFile(const std::string& name, const std::string& fsPath);
        
        // �L�[���ŃV�F�[�_�[���擾
        Shader GetShader(const std::string& name) const;
        
        // �V�F�[�_�[���ǂݍ��܂�Ă��邩�m�F
        bool HasShader(const std::string& name) const;
        
        // �V�F�[�_�[�̃��P�[�V�����擾�iuniform�ϐ��Ȃǁj
        int GetShaderLocation(const std::string& name, const std::string& uniformName);
        
        // �V�F�[�_�[�̕��������_�l��ݒ�
        void SetShaderValue(const std::string& name, const std::string& uniformName, float value);
        
        // �V�F�[�_�[�̐����l��ݒ�
        void SetShaderValueI(const std::string& name, const std::string& uniformName, int value);
        
        // ���ׂẴV�F�[�_�[���A�����[�h
        void UnloadAll();
        
    private:
        std::unordered_map<std::string, Shader> shaders_;
        std::unordered_map<std::string, std::unordered_map<std::string, int>> shaderLocations_;
    };

    // �t���[�����\���́i�X�v���C�g�V�[�g�p�j
    struct FrameInfo {
        Rectangle rect;       // �t���[���̈ʒu�ƃT�C�Y
        int duration;         // �t���[���������ԁi�~���b�j
        std::string textureName;  // �֘A����e�N�X�`����
    };

    // �摜���\�[�X�Ǘ��i�X�v���C�g�V�[�g�Ή��j
    class ImageManager {
    public:
        ImageManager() = default;
        ~ImageManager();
        
        // �摜��ǂݍ��݁iCPU���j
        void LoadImage(const std::string& name, const std::string& filePath);
        
        // Aseprite JSON�`���̃X�v���C�g�V�[�g��ǂݍ���
        void LoadSpriteSheet(const std::string& name, const std::string& jsonPath, const std::string& imagePath);
        
        // ディレクトリ内の全JSONスプライトシートを読み込む
        void LoadAllSpriteSheets(const std::string& jsonDir, const std::string& atlasDir);
        
        // �L�[���ŉ摜���擾
        Image GetImage(const std::string& name) const;
        
        // �摜���ǂݍ��܂�Ă��邩�m�F
        bool HasImage(const std::string& name) const;
        
        // �t���[�������擾�i�X�v���C�g�V�[�g�p�j
        FrameInfo GetFrameInfo(const std::string& frameName) const;
        
        // �t���[�������݂��邩�m�F
        bool HasFrame(const std::string& frameName) const;
        
        // ���ׂẴt���[�������擾
        std::vector<std::string> GetAllFrameNames(const std::string& spriteName) const;
        
        // 全スプライトシート名を取得
        std::vector<std::string> GetAllSpriteSheetNames() const;
        
        // �摜���e�N�X�`���ɕϊ��iGPU���ցj
        void ImageToTexture(const std::string& imageName, const std::string& textureName);
        
        // �摜�����T�C�Y
        void ResizeImage(const std::string& name, int width, int height);
        
        // �摜�𔽓]
        void FlipImage(const std::string& name, bool horizontal);
        
        // ���ׂẲ摜���A�����[�h
        void UnloadAll();
        
    private:
        std::unordered_map<std::string, Image> images_;
        std::unordered_map<std::string, FrameInfo> frames_;  // �t���[���� �� �t���[�����
        std::unordered_map<std::string, std::vector<std::string>> spriteSheets_;  // �X�v���C�g�� �� �t���[�������X�g
    };

    // �������\�[�X�Ǘ��N���X
    class ResourceManager {
    public:
        static ResourceManager& GetInstance();
        
        // �C���X�^���X�̃R�s�[���֎~
        ResourceManager(const ResourceManager&) = delete;
        ResourceManager& operator=(const ResourceManager&) = delete;
        
        // �e�N�X�`���}�l�[�W���[�ў̃A�N�Z�X
        TextureManager& GetTextureManager();
        const TextureManager& GetTextureManager() const;
        
        // �t�H���g�}�l�[�W���[�ў̃A�N�Z�X
        FontManager& GetFontManager();
        const FontManager& GetFontManager() const;
        
        // �T�E���h�}�l�[�W���[�ў̃A�N�Z�X
        SoundManager& GetSoundManager();
        const SoundManager& GetSoundManager() const;
        
        // �~���[�W�b�N�}�l�[�W���[�ў̃A�N�Z�X
        MusicManager& GetMusicManager();
        const MusicManager& GetMusicManager() const;
        
        // �V�F�[�_�[�}�l�[�W���[�ў̃A�N�Z�X
        ShaderManager& GetShaderManager();
        const ShaderManager& GetShaderManager() const;
        
        // �C���[�W�}�l�[�W���[�ў̃A�N�Z�X
        ImageManager& GetImageManager();
        const ImageManager& GetImageManager() const;
        
        // ���ׂẴ��\�[�X���A�����[�h
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
