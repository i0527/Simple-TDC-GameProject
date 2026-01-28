#include "TowerAttachmentLoader.hpp"

// 標準ライブラリ
#include <algorithm>
#include <fstream>
#include <vector>

// 外部ライブラリ
#include <nlohmann/json.hpp>

// プロジェクト内
#include "../../../utils/Log.h"

using json = nlohmann::json;

namespace game {
namespace core {
namespace entities {

namespace {

std::string ToLowerCopy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}

TowerAttachmentEffectType ParseEffectType(const json& j) {
    const std::string raw = j.value("effect_type", "percentage");
    const std::string s = ToLowerCopy(raw);
    if (s == "percentage" || s == "percent" || s == "ratio") {
        return TowerAttachmentEffectType::Percentage;
    }
    if (s == "flat" || s == "add") {
        return TowerAttachmentEffectType::Flat;
    }
    LOG_WARN("TowerAttachmentLoader: Unknown effect_type '{}', fallback to 'percentage'", raw);
    return TowerAttachmentEffectType::Percentage;
}

TowerAttachmentTargetStat ParseTargetStat(const json& j) {
    const std::string raw = j.value("target_stat", "tower_hp");
    const std::string s = ToLowerCopy(raw);
    if (s == "tower_hp") return TowerAttachmentTargetStat::TowerHp;
    if (s == "wallet_growth") return TowerAttachmentTargetStat::WalletGrowth;
    if (s == "cost_regen") return TowerAttachmentTargetStat::CostRegen;
    if (s == "ally_attack") return TowerAttachmentTargetStat::AllyAttack;
    if (s == "ally_hp") return TowerAttachmentTargetStat::AllyHp;
    if (s == "enemy_hp") return TowerAttachmentTargetStat::EnemyHp;
    if (s == "enemy_attack") return TowerAttachmentTargetStat::EnemyAttack;
    if (s == "enemy_move_speed" || s == "enemy_speed") {
        return TowerAttachmentTargetStat::EnemyMoveSpeed;
    }
    LOG_WARN("TowerAttachmentLoader: Unknown target_stat '{}', fallback to 'tower_hp'", raw);
    return TowerAttachmentTargetStat::TowerHp;
}

} // namespace

bool TowerAttachmentLoader::LoadFromJSON(
    const std::string& json_path,
    std::unordered_map<std::string, TowerAttachment>& outMasters) {
    outMasters.clear();
    if (json_path.empty()) {
        return false;
    }

    try {
        std::ifstream file(json_path);
        if (!file.is_open()) {
            LOG_WARN("TowerAttachmentLoader: failed to open json: {}", json_path);
            return false;
        }

        json root;
        file >> root;

        if (!root.contains("tower_attachments") || !root["tower_attachments"].is_array()) {
            LOG_WARN("TowerAttachmentLoader: missing tower_attachments array");
            return false;
        }

        for (const auto& v : root["tower_attachments"]) {
            if (!v.is_object()) continue;
            TowerAttachment attachment;
            attachment.id = v.value("id", "");
            if (attachment.id.empty()) {
                continue;
            }
            attachment.name = v.value("name", attachment.id);
            attachment.description = v.value("description", "");
            attachment.effect_type = ParseEffectType(v);
            attachment.target_stat = ParseTargetStat(v);
            attachment.value_per_level = static_cast<float>(v.value("value_per_level", 0.0));
            attachment.max_level = std::max(1, v.value("max_level", 1));
            // アタッチメントは 1=R, 2=SR, 3=SSR のみ
            int r = v.value("rarity", 1);
            if (v.contains("rarity") && v["rarity"].is_string()) {
                const std::string rs = ToLowerCopy(v["rarity"].get<std::string>());
                if (rs == "r") r = 1;
                else if (rs == "sr") r = 2;
                else if (rs == "ssr") r = 3;
            }
            attachment.rarity = std::max(1, std::min(3, r));
            outMasters[attachment.id] = attachment;
        }

        return !outMasters.empty();
    } catch (const json::parse_error& e) {
        LOG_ERROR("TowerAttachmentLoader: JSON parse error: {}", e.what());
    } catch (const std::exception& e) {
        LOG_ERROR("TowerAttachmentLoader: error: {}", e.what());
    }
    return false;
}

void TowerAttachmentLoader::LoadHardcoded(
    std::unordered_map<std::string, TowerAttachment>& outMasters) {
    outMasters.clear();

    // rarity: 1=R, 2=SR, 3=SSR
    auto add = [&](const std::string& id,
                   const std::string& name,
                   const std::string& desc,
                   TowerAttachmentTargetStat target,
                   float value_per_level,
                   int max_level,
                   int rarity) {
        TowerAttachment a;
        a.id = id;
        a.name = name;
        a.description = desc;
        a.target_stat = target;
        a.value_per_level = value_per_level;
        a.max_level = max_level;
        a.rarity = std::max(1, std::min(3, rarity));
        outMasters[id] = a;
    };

    add("tower_core_hp", "城塞コア", "城HPが増加する。", TowerAttachmentTargetStat::TowerHp, 0.05f, 50, 1);
    add("tower_wallet_gear", "蓄財ギア", "お金成長/秒が増加する。", TowerAttachmentTargetStat::WalletGrowth, 0.05f, 50, 1);
    add("tower_cost_capacitor", "回復コンデンサ", "コスト回復/秒が増加する。", TowerAttachmentTargetStat::CostRegen, 0.05f, 50, 1);
    add("tower_ally_relic_atk", "猛攻レリック", "味方攻撃が増加する。", TowerAttachmentTargetStat::AllyAttack, 0.02f, 50, 2);
    add("tower_ally_relic_hp", "守護レリック", "味方HPが増加する。", TowerAttachmentTargetStat::AllyHp, 0.02f, 50, 2);
    add("tower_enemy_slow", "妨害レンズ", "敵移動速度が低下する。", TowerAttachmentTargetStat::EnemyMoveSpeed, -0.02f, 50, 3);
    // 追加10種（ガチャで入手）
    add("tower_fortress_plate", "要塞プレート", "城HPがさらに増加する。", TowerAttachmentTargetStat::TowerHp, 0.04f, 50, 2);
    add("tower_gold_amplifier", "黄金増幅器", "お金成長/秒がさらに増加する。", TowerAttachmentTargetStat::WalletGrowth, 0.04f, 50, 2);
    add("tower_quick_charge", "高速充填", "コスト回復/秒がさらに増加する。", TowerAttachmentTargetStat::CostRegen, 0.04f, 50, 2);
    add("tower_berserk_core", "狂戦士コア", "味方攻撃が大きく増加する。", TowerAttachmentTargetStat::AllyAttack, 0.03f, 50, 3);
    add("tower_guardian_aura", "守護オーラ", "味方HPが大きく増加する。", TowerAttachmentTargetStat::AllyHp, 0.03f, 50, 3);
    add("tower_enemy_weaken", "敵弱体レンズ", "敵HPが減少する。", TowerAttachmentTargetStat::EnemyHp, -0.02f, 50, 2);
    add("tower_enemy_attack_down", "攻撃弱化フィールド", "敵攻撃が低下する。", TowerAttachmentTargetStat::EnemyAttack, -0.02f, 50, 2);
    add("tower_slow_field", "減速フィールド", "敵移動速度がさらに低下する。", TowerAttachmentTargetStat::EnemyMoveSpeed, -0.03f, 50, 3);
    add("tower_balance_unit", "バランスユニット", "城HPがわずかに増加する。", TowerAttachmentTargetStat::TowerHp, 0.02f, 50, 1);
    add("tower_critical_module", "クリティカルモジュール", "味方攻撃が増加する。", TowerAttachmentTargetStat::AllyAttack, 0.025f, 50, 2);
}

} // namespace entities
} // namespace core
} // namespace game
