#pragma once

#include "Character.hpp"
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

namespace game {
namespace core {
namespace entities {

// キャラクターマスターデータ管理
class CharacterManager {
public:
    CharacterManager();
    ~CharacterManager();

    // 初期化（JSON / ハードコードからデータロード）
    bool Initialize(const std::string& json_path = "");

    // マスターデータからキャラクターを取得
    // （フレッシュなインスタンスを返す）
    std::shared_ptr<Character> GetCharacterTemplate(const std::string& character_id);

    // 全キャラクターIDを取得
    std::vector<std::string> GetAllCharacterIds() const;

    // キャラクター存在確認
    bool HasCharacter(const std::string& character_id) const;

    // キャラクター数
    size_t GetCharacterCount() const { return masters_.size(); }

    // 全マスターデータ取得（デバッグ用）
    const std::unordered_map<std::string, Character>& GetAllMasters() const {
        return masters_;
    }

    // 終了処理
    void Shutdown();

private:
    // マスターデータ（ID -> Character）
    std::unordered_map<std::string, Character> masters_;

    // JSONからデータロード
    bool LoadFromJSON(const std::string& json_path);

    // ハードコードデータを初期化（JSON不要な場合）
    void InitializeHardcodedData();
};

} // namespace entities
} // namespace core
} // namespace game
