#pragma once

// 標準ライブラリ
#include <string>
#include <unordered_map>
#include <vector>

// プロジェクト内
#include "TowerAttachment.hpp"

namespace game {
namespace core {
namespace entities {

/// @brief タワーアタッチメントマスターデータ管理
class TowerAttachmentManager {
public:
    TowerAttachmentManager();
    ~TowerAttachmentManager();

    bool Initialize(const std::string& json_path = "");
    const TowerAttachment* GetAttachment(const std::string& id) const;
    std::vector<const TowerAttachment*> GetAllAttachments() const;
    const std::unordered_map<std::string, TowerAttachment>& GetAttachmentMasters() const {
        return attachment_masters_;
    }
    void SetMasters(const std::unordered_map<std::string, TowerAttachment>& masters);
    void Shutdown();

private:
    std::unordered_map<std::string, TowerAttachment> attachment_masters_;
};

} // namespace entities
} // namespace core
} // namespace game
