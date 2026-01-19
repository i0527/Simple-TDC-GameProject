#pragma once

#include "../api/BaseSystemAPI.hpp"
#include "../config/RenderPrimitives.hpp"
#include <string>
#include <vector>

namespace game {
namespace core {
namespace ui {

/// @brief 繝ｦ繝九ャ繝医Μ繧ｹ繝医い繧､繝・Β
struct UnitListItem {
    std::string unitId;
    std::string displayName;
    int currentCount;
    int maxCount;
    int costGold;
    bool isSelected;
};

/// @brief 繧ｲ繝ｼ繝繧ｷ繝ｼ繝ｳHUD謠冗判繧ｯ繝ｩ繧ｹ
///
/// 雋ｬ蜍・
/// - Top HUD・・ave縲？P縲；old縲√ご繝ｼ繝騾溷ｺｦ縲√・繧ｿ繝ｳ・・
/// - Left Panel・医Α繝九・繝・・繝ｻ閭梧勹陦ｨ遉ｺ・・
/// - Right Panel・医Θ繝九ャ繝磯∈謚槭ヱ繝阪Ν・・
/// - Field UI・磯∈謚槭・繝帙ヰ繝ｼ陦ｨ遉ｺ・・
class HUDRenderer {
public:
    /// @brief 繧ｳ繝ｳ繧ｹ繝医Λ繧ｯ繧ｿ
    /// @param sysAPI BaseSystemAPI繝昴う繝ｳ繧ｿ
    explicit HUDRenderer(BaseSystemAPI* sysAPI);
    
    ~HUDRenderer() = default;

    // ========== Top HUD ==========

    /// @brief Top HUD繧呈緒逕ｻ
    /// @param wave 迴ｾ蝨ｨ縺ｮ繧ｦ繧ｧ繝ｼ繝也分蜿ｷ
    /// @param totalWaves 邱上え繧ｧ繝ｼ繝匁焚
    /// @param hp 迴ｾ蝨ｨ縺ｮHP
    /// @param maxHp 譛螟ｧHP
    /// @param gold 迴ｾ蝨ｨ縺ｮ繧ｴ繝ｼ繝ｫ繝・
    /// @param gameSpeed 繧ｲ繝ｼ繝騾溷ｺｦ・・.5, 1.0, 2.0・・
    /// @param isPaused 繝昴・繧ｺ荳ｭ縺九←縺・°
    /// @param gameStateText 繧ｲ繝ｼ繝迥ｶ諷九ユ繧ｭ繧ｹ繝茨ｼ井ｾ具ｼ・貅門ｙ荳ｭ..."・・
    void RenderTopHUD(int wave, int totalWaves, int hp, int maxHp,
                     int gold, float gameSpeed, bool isPaused,
                     const std::string& gameStateText);

    // ========== Left Panel ==========

    /// @brief Left Panel・医Α繝九・繝・・・峨ｒ謠冗判
    /// @param fieldOriginX 繝輔ぅ繝ｼ繝ｫ繝牙次轤ｹX蠎ｧ讓・
    /// @param fieldOriginY 繝輔ぅ繝ｼ繝ｫ繝牙次轤ｹY蠎ｧ讓・
    /// @param fieldWidth 繝輔ぅ繝ｼ繝ｫ繝牙ｹ・ｼ医ヴ繧ｯ繧ｻ繝ｫ・・
    /// @param fieldHeight 繝輔ぅ繝ｼ繝ｫ繝蛾ｫ倥＆・医ヴ繧ｯ繧ｻ繝ｫ・・
    void RenderLeftPanel(float fieldOriginX, float fieldOriginY,
                        float fieldWidth, float fieldHeight);

    // ========== Right Panel ==========

    /// @brief Right Panel・医Θ繝九ャ繝磯∈謚橸ｼ峨ｒ謠冗判
    /// @param units 繝ｦ繝九ャ繝医Μ繧ｹ繝・
    /// @param selectedUnitId 驕ｸ謚樔ｸｭ縺ｮ繝ｦ繝九ャ繝・D・育ｩｺ譁・ｭ怜・縺ｪ繧画悴驕ｸ謚橸ｼ・
    void RenderRightPanel(const std::vector<UnitListItem>& units,
                         const std::string& selectedUnitId);

    // ========== Field UI ==========

    /// @brief 繝輔ぅ繝ｼ繝ｫ繝蔚I・医・繝舌・繝ｻ驕ｸ謚櫁｡ｨ遉ｺ・峨ｒ謠冗判
    /// @param hoverGx 繝帙ヰ繝ｼ荳ｭ縺ｮ繧ｰ繝ｪ繝・ラX蠎ｧ讓呻ｼ・1縺ｪ繧臥┌蜉ｹ・・
    /// @param hoverGy 繝帙ヰ繝ｼ荳ｭ縺ｮ繧ｰ繝ｪ繝・ラY蠎ｧ讓呻ｼ・1縺ｪ繧臥┌蜉ｹ・・
    /// @param selectGx 驕ｸ謚樔ｸｭ縺ｮ繧ｰ繝ｪ繝・ラX蠎ｧ讓呻ｼ・1縺ｪ繧臥┌蜉ｹ・・
    /// @param selectGy 驕ｸ謚樔ｸｭ縺ｮ繧ｰ繝ｪ繝・ラY蠎ｧ讓呻ｼ・1縺ｪ繧臥┌蜉ｹ・・
    /// @param isPlaceable 繝帙ヰ繝ｼ菴咲ｽｮ縺碁・鄂ｮ蜿ｯ閭ｽ縺九←縺・°
    /// @param cellSize 繧ｻ繝ｫ繧ｵ繧､繧ｺ・医ヴ繧ｯ繧ｻ繝ｫ・・
    /// @param fieldOriginX 繝輔ぅ繝ｼ繝ｫ繝牙次轤ｹX蠎ｧ讓・
    /// @param fieldOriginY 繝輔ぅ繝ｼ繝ｫ繝牙次轤ｹY蠎ｧ讓・
    void RenderFieldUI(int hoverGx, int hoverGy, int selectGx, int selectGy,
                      bool isPlaceable, int cellSize,
                      float fieldOriginX, float fieldOriginY);

    // ========== 繝懊ち繝ｳ讀懷・ ==========

    /// @brief Top HUD縺ｮ繝懊ち繝ｳ縺後け繝ｪ繝・け縺輔ｌ縺溘°繝√ぉ繝・け
    /// @param mouseX 繝槭え繧ｹX蠎ｧ讓・
    /// @param mouseY 繝槭え繧ｹY蠎ｧ讓・
    /// @return 繧ｯ繝ｪ繝・け縺輔ｌ縺溘・繧ｿ繝ｳ蜷搾ｼ・speed_0.5", "speed_1.0", "speed_2.0", "pause", "exit", ""・・
    std::string CheckTopHUDButtonClick(float mouseX, float mouseY);

private:
    BaseSystemAPI* sysAPI_;

    /// @brief 繝励Ο繧ｰ繝ｬ繧ｹ繝舌・繧呈緒逕ｻ
    void DrawBar(float x, float y, float width, float height,
                         float current, float max, ColorRGBA fillColor, ColorRGBA bgColor);

    /// @brief 繝・く繧ｹ繝医ｒ謠冗判・医ョ繝輔か繝ｫ繝医ヵ繧ｩ繝ｳ繝井ｽｿ逕ｨ・・
    void DrawText(float x, float y, const std::string& text,
                          int fontSize, ColorRGBA color);

    /// @brief 荳ｭ螟ｮ謠・∴繝・く繧ｹ繝医ｒ謠冗判
    void DrawTextCentered(float centerX, float y, const std::string& text,
                                  int fontSize, ColorRGBA color);

    /// @brief 繝懊ち繝ｳ繧呈緒逕ｻ
    void DrawButton(float x, float y, float width, float height,
                            const std::string& label, bool isActive, ColorRGBA baseColor);

    /// @brief 遏ｩ蠖｢蜀・↓繝槭え繧ｹ縺後≠繧九°繝√ぉ繝・け
    bool IsMouseInRect(float mouseX, float mouseY,
                      float rectX, float rectY, float rectW, float rectH);

    // Top HUD繝懊ち繝ｳ遏ｩ蠖｢・医く繝｣繝・す繝･・・
    struct ButtonRect {
        float x, y, width, height;
        std::string id;
    };
    std::vector<ButtonRect> topHudButtons_;
};

} // namespace ui
} // namespace core
} // namespace game
