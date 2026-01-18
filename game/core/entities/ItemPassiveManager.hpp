#pragma once

#include "Character.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>

namespace game {
namespace core {
namespace entities {

/**
 * @brief 装備アイテムとパッシブスキルのマスターデータを管理するクラス
 */
class ItemPassiveManager {
public:
    ItemPassiveManager();
    ~ItemPassiveManager();

    /**
     * @brief 初期化
     * @param json_path マスターデータのJSONパス
     * @return 成功した場合はtrue
     */
    bool Initialize(const std::string& json_path = "");

    /**
     * @brief 指定したIDのパッシブスキルを取得
     * @param id スキルID
     * @return パッシブスキル定義のポインタ（見つからない場合はnullptr）
     */
    const PassiveSkill* GetPassiveSkill(const std::string& id) const;

    /**
     * @brief 全パッシブスキル定義を取得
     * @return パッシブスキル定義ポインタのリスト
     */
    std::vector<const PassiveSkill*> GetAllPassiveSkills() const;

    /**
     * @brief 指定したIDの装備アイテムを取得
     * @param id 装備ID
     * @return 装備定義のポインタ（見つからない場合はnullptr）
     */
    const Equipment* GetEquipment(const std::string& id) const;

    /**
     * @brief 全装備アイテム定義を取得
     * @return 装備定義ポインタのリスト
     */
    std::vector<const Equipment*> GetAllEquipment() const;

    /**
     * @brief 終了処理
     */
    void Shutdown();

private:
    std::unordered_map<std::string, PassiveSkill> passive_masters_;
    std::unordered_map<std::string, Equipment> equipment_masters_;

    bool LoadFromJSON(const std::string& json_path);
    void InitializeHardcodedData();
};

} // namespace entities
} // namespace core
} // namespace game
