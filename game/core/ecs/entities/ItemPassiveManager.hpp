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
 * @brief 陬・ｙ繧｢繧､繝・Β縺ｨ繝代ャ繧ｷ繝悶せ繧ｭ繝ｫ縺ｮ繝槭せ繧ｿ繝ｼ繝・・繧ｿ繧堤ｮ｡逅・☆繧九け繝ｩ繧ｹ
 */
class ItemPassiveManager {
public:
    ItemPassiveManager();
    ~ItemPassiveManager();

    /**
     * @brief 蛻晄悄蛹・
     * @param json_path 繝槭せ繧ｿ繝ｼ繝・・繧ｿ縺ｮJSON繝代せ
     * @return 謌仙粥縺励◆蝣ｴ蜷医・true
     */
    bool Initialize(const std::string& json_path = "");

    /**
     * @brief 謖・ｮ壹＠縺櫑D縺ｮ繝代ャ繧ｷ繝悶せ繧ｭ繝ｫ繧貞叙蠕・
     * @param id 繧ｹ繧ｭ繝ｫID
     * @return 繝代ャ繧ｷ繝悶せ繧ｭ繝ｫ螳夂ｾｩ縺ｮ繝昴う繝ｳ繧ｿ・郁ｦ九▽縺九ｉ縺ｪ縺・ｴ蜷医・nullptr・・
     */
    const PassiveSkill* GetPassiveSkill(const std::string& id) const;

    /**
     * @brief 蜈ｨ繝代ャ繧ｷ繝悶せ繧ｭ繝ｫ螳夂ｾｩ繧貞叙蠕・
     * @return 繝代ャ繧ｷ繝悶せ繧ｭ繝ｫ螳夂ｾｩ繝昴う繝ｳ繧ｿ縺ｮ繝ｪ繧ｹ繝・
     */
    std::vector<const PassiveSkill*> GetAllPassiveSkills() const;
    const std::unordered_map<std::string, PassiveSkill>& GetPassiveMasters() const {
        return passive_masters_;
    }

    /**
     * @brief 謖・ｮ壹＠縺櫑D縺ｮ陬・ｙ繧｢繧､繝・Β繧貞叙蠕・
     * @param id 陬・ｙID
     * @return 陬・ｙ螳夂ｾｩ縺ｮ繝昴う繝ｳ繧ｿ・郁ｦ九▽縺九ｉ縺ｪ縺・ｴ蜷医・nullptr・・
     */
    const Equipment* GetEquipment(const std::string& id) const;

    /**
     * @brief 蜈ｨ陬・ｙ繧｢繧､繝・Β螳夂ｾｩ繧貞叙蠕・
     * @return 陬・ｙ螳夂ｾｩ繝昴う繝ｳ繧ｿ縺ｮ繝ｪ繧ｹ繝・
     */
    std::vector<const Equipment*> GetAllEquipment() const;
    const std::unordered_map<std::string, Equipment>& GetEquipmentMasters() const {
        return equipment_masters_;
    }

    /**
     * @brief 邨ゆｺ・・逅・
     */
    void Shutdown();
    void SetMasters(const std::unordered_map<std::string, PassiveSkill>& passives,
                    const std::unordered_map<std::string, Equipment>& equipment);

private:
    std::unordered_map<std::string, PassiveSkill> passive_masters_;
    std::unordered_map<std::string, Equipment> equipment_masters_;

    // 繝ｭ繝ｼ繝峨・ ItemPassiveLoader 縺ｫ蟋碑ｭｲ
};

} // namespace entities
} // namespace core
} // namespace game
