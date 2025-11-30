/**
 * @file AnimationPlayer.h
 * @brief アニメーション再生制御
 * 
 * Phase 4C: データ駆動アニメーションシステム
 * AnimationDefを使用してアニメーションを再生
 * テクスチャが無い場合はフォールバック描画
 */
#pragma once

#include "Data/AnimationDef.h"
#include "Core/Platform.h"
#include <functional>
#include <string>
#include <vector>
#include <set>
#include <iostream>

namespace Core {

/**
 * @brief アニメーションイベントコールバック
 */
using AnimEventCallback = std::function<void(const Data::FrameEventDef& event)>;

/**
 * @brief アニメーション終了コールバック
 */
using AnimCompleteCallback = std::function<void(const std::string& clipId)>;

/**
 * @brief アニメーション再生状態
 */
enum class AnimPlayState {
    Stopped,
    Playing,
    Paused
};

/**
 * @brief アニメーション再生プレイヤー
 * 
 * 使用例:
 * @code
 * AnimationPlayer player;
 * player.SetAnimation(myAnimDef);
 * player.Play("walk");
 * 
 * // ゲームループ
 * player.Update(deltaTime);
 * player.Draw(x, y, facingRight);
 * @endcode
 */
class AnimationPlayer {
public:
    AnimationPlayer() = default;
    
    /**
     * @brief アニメーション定義を設定
     */
    void SetAnimation(const Data::SpriteAnimationDef* animDef) {
        animDef_ = animDef;
        if (animDef_) {
            Play(animDef_->defaultClip);
        }
    }
    
    /**
     * @brief テクスチャを設定（外部で管理）
     */
    void SetTexture(Texture2D texture) {
        texture_ = texture;
        hasValidTexture_ = (texture.id != 0);
    }
    
    /**
     * @brief フォールバック設定を有効化
     * @param enable フォールバック描画を有効にするか
     */
    void SetFallbackEnabled(bool enable) {
        fallbackEnabled_ = enable;
    }
    
    /**
     * @brief フォールバック色を設定
     */
    void SetFallbackColor(Color color) {
        fallbackColor_ = color;
    }
    
    /**
     * @brief クリップを再生
     * @param clipId クリップID
     * @param restart 既に再生中でも最初から開始するか
     */
    void Play(const std::string& clipId, bool restart = false) {
        if (!animDef_) return;
        
        // 既に同じクリップを再生中で、restartでなければスキップ
        if (!restart && currentClipId_ == clipId && state_ == AnimPlayState::Playing) {
            return;
        }
        
        // 現在のクリップが割り込み不可なら無視
        if (currentClip_ && !currentClip_->canInterrupt && state_ == AnimPlayState::Playing) {
            // 次に再生するクリップを予約
            nextClipId_ = clipId;
            return;
        }
        
        const Data::AnimClipDef* clip = animDef_->GetClip(clipId);
        if (!clip) {
            std::cerr << "AnimationPlayer: Clip not found: " << clipId << std::endl;
            return;
        }
        
        currentClipId_ = clipId;
        currentClip_ = clip;
        currentFrame_ = 0;
        frameTime_ = 0.0f;
        loopCount_ = 0;
        pingPongReverse_ = false;
        state_ = AnimPlayState::Playing;
        firedEvents_.clear();
    }
    
    /**
     * @brief 一時停止
     */
    void Pause() {
        if (state_ == AnimPlayState::Playing) {
            state_ = AnimPlayState::Paused;
        }
    }
    
    /**
     * @brief 再開
     */
    void Resume() {
        if (state_ == AnimPlayState::Paused) {
            state_ = AnimPlayState::Playing;
        }
    }
    
    /**
     * @brief 停止
     */
    void Stop() {
        state_ = AnimPlayState::Stopped;
        currentFrame_ = 0;
        frameTime_ = 0.0f;
    }
    
    /**
     * @brief 更新
     */
    void Update(float deltaTime) {
        if (!currentClip_ || state_ != AnimPlayState::Playing) return;
        if (currentClip_->frames.empty()) return;
        
        // 速度適用
        float adjustedDelta = deltaTime * currentClip_->speed * speedMultiplier_;
        frameTime_ += adjustedDelta;
        
        // 現在フレームの表示時間を取得
        float frameDuration = currentClip_->frames[currentFrame_].duration;
        
        // イベント発火チェック
        CheckFrameEvents();
        
        // フレーム進行
        while (frameTime_ >= frameDuration) {
            frameTime_ -= frameDuration;
            AdvanceFrame();
            
            if (currentFrame_ < currentClip_->frames.size()) {
                frameDuration = currentClip_->frames[currentFrame_].duration;
            } else {
                break;
            }
        }
    }
    
    /**
     * @brief 描画
     * @param x X座標
     * @param y Y座標
     * @param facingRight 右向きか
     * @param scale スケール
     * @param tint 色調
     */
    void Draw(float x, float y, bool facingRight = true, float scale = 1.0f, Color tint = WHITE) {
        if (!animDef_ || !currentClip_) return;
        if (currentClip_->frames.empty()) return;
        if (currentFrame_ >= currentClip_->frames.size()) return;
        
        const auto& sheet = animDef_->spriteSheet;
        const auto& frame = currentClip_->frames[currentFrame_];
        
        // テクスチャが無い場合はフォールバック描画
        if (!hasValidTexture_ || texture_.id == 0) {
            if (fallbackEnabled_) {
                DrawFallback(x, y, scale);
            }
            return;
        }
        
        // スプライトシートからソース矩形を計算
        int col = frame.spriteIndex % sheet.columns;
        int row = frame.spriteIndex / sheet.columns;
        
        Rectangle sourceRect = {
            static_cast<float>(sheet.offsetX + col * (sheet.frameWidth + sheet.paddingX)),
            static_cast<float>(sheet.offsetY + row * (sheet.frameHeight + sheet.paddingY)),
            static_cast<float>(sheet.frameWidth),
            static_cast<float>(sheet.frameHeight)
        };
        
        // 左右反転
        if (!facingRight) {
            sourceRect.width = -sourceRect.width;
        }
        
        // スケール計算
        float finalScaleX = scale * (frame.scaleX.value_or(1.0f));
        float finalScaleY = scale * (frame.scaleY.value_or(1.0f));
        
        // オフセット計算
        float offsetX = animDef_->globalOffsetX + frame.offsetX.value_or(0.0f);
        float offsetY = animDef_->globalOffsetY + frame.offsetY.value_or(0.0f);
        
        // ピボット計算
        float pivotOffsetX = sheet.frameWidth * finalScaleX * animDef_->pivotX;
        float pivotOffsetY = sheet.frameHeight * finalScaleY * animDef_->pivotY;
        
        // 描画先矩形
        Rectangle destRect = {
            x + offsetX - pivotOffsetX,
            y + offsetY - pivotOffsetY,
            sheet.frameWidth * finalScaleX,
            sheet.frameHeight * finalScaleY
        };
        
        // 透明度
        Color drawTint = tint;
        if (frame.alpha) {
            drawTint.a = static_cast<unsigned char>(frame.alpha.value() * 255);
        }
        
        // 回転
        float rotation = frame.rotation.value_or(0.0f);
        
        // 描画
        Vector2 origin = {0, 0};
        DrawTexturePro(texture_, sourceRect, destRect, origin, rotation, drawTint);
    }
    
    /**
     * @brief フォールバック描画（テクスチャ不在時）
     */
    void DrawFallback(float x, float y, float scale = 1.0f) {
        if (!animDef_) return;
        
        const auto& sheet = animDef_->spriteSheet;
        
        // アニメーション定義からサイズを取得
        float width = static_cast<float>(sheet.frameWidth) * scale;
        float height = static_cast<float>(sheet.frameHeight) * scale;
        
        // ピボットを考慮した位置調整
        float pivotOffsetX = width * animDef_->pivotX;
        float pivotOffsetY = height * animDef_->pivotY;
        
        float drawX = x - pivotOffsetX + width / 2;
        float drawY = y - pivotOffsetY + height / 2;
        
        // 現在のフレーム数を計算
        int totalFrames = currentClip_ ? static_cast<int>(currentClip_->frames.size()) : 1;
        
        // 基本形状（円）を描画
        float radius = std::min(width, height) / 2;
        DrawCircle(static_cast<int>(drawX), static_cast<int>(drawY), radius, fallbackColor_);
        DrawCircleLines(static_cast<int>(drawX), static_cast<int>(drawY), radius, DARKGRAY);
        
        // フレーム番号表示
        char frameStr[16];
        snprintf(frameStr, sizeof(frameStr), "%d/%d", currentFrame_, totalFrames);
        int fontSize = 12;
        int textW = MeasureText(frameStr, fontSize);
        RDrawText(frameStr,
                 static_cast<int>(drawX - textW / 2),
                 static_cast<int>(drawY - 6),
                 fontSize, WHITE);
        
        // クリップ名表示
        if (!currentClipId_.empty()) {
            int nameWidth = MeasureText(currentClipId_.c_str(), 10);
            RDrawText(currentClipId_.c_str(),
                     static_cast<int>(drawX - nameWidth / 2),
                     static_cast<int>(drawY + radius + 4),
                     10, DARKGRAY);
        }
        
        // 再生インジケータ（回転する点）
        float angle = currentFrame_ * (360.0f / totalFrames) * DEG2RAD;
        float indicatorX = drawX + cosf(angle) * radius * 0.7f;
        float indicatorY = drawY + sinf(angle) * radius * 0.7f;
        Color indicatorColor = ColorFromHSV((currentFrame_ % 8) * 45.0f, 0.6f, 0.9f);
        DrawCircle(static_cast<int>(indicatorX), static_cast<int>(indicatorY), 4, indicatorColor);
    }
    
    /**
     * @brief イベントコールバックを設定
     */
    void SetEventCallback(AnimEventCallback callback) {
        eventCallback_ = std::move(callback);
    }
    
    /**
     * @brief 完了コールバックを設定
     */
    void SetCompleteCallback(AnimCompleteCallback callback) {
        completeCallback_ = std::move(callback);
    }
    
    /**
     * @brief 再生速度倍率を設定
     */
    void SetSpeedMultiplier(float multiplier) {
        speedMultiplier_ = multiplier;
    }
    
    // ===== ゲッター =====
    
    AnimPlayState GetState() const { return state_; }
    const std::string& GetCurrentClipId() const { return currentClipId_; }
    int GetCurrentFrame() const { return currentFrame_; }
    float GetProgress() const {
        if (!currentClip_ || currentClip_->frames.empty()) return 0.0f;
        return static_cast<float>(currentFrame_) / currentClip_->frames.size();
    }
    bool IsPlaying() const { return state_ == AnimPlayState::Playing; }
    bool IsPlaying(const std::string& clipId) const {
        return state_ == AnimPlayState::Playing && currentClipId_ == clipId;
    }
    
private:
    void AdvanceFrame() {
        if (!currentClip_) return;
        
        int frameCount = static_cast<int>(currentClip_->frames.size());
        
        switch (currentClip_->loopMode) {
            case Data::AnimLoopMode::Once:
                currentFrame_++;
                if (currentFrame_ >= frameCount) {
                    currentFrame_ = frameCount - 1;
                    OnClipComplete();
                }
                break;
                
            case Data::AnimLoopMode::Loop:
                currentFrame_ = (currentFrame_ + 1) % frameCount;
                if (currentFrame_ == 0) {
                    firedEvents_.clear();
                }
                break;
                
            case Data::AnimLoopMode::PingPong:
                if (pingPongReverse_) {
                    currentFrame_--;
                    if (currentFrame_ <= 0) {
                        currentFrame_ = 0;
                        pingPongReverse_ = false;
                        firedEvents_.clear();
                    }
                } else {
                    currentFrame_++;
                    if (currentFrame_ >= frameCount - 1) {
                        currentFrame_ = frameCount - 1;
                        pingPongReverse_ = true;
                    }
                }
                break;
                
            case Data::AnimLoopMode::LoopCount:
                currentFrame_++;
                if (currentFrame_ >= frameCount) {
                    loopCount_++;
                    if (loopCount_ >= currentClip_->loopCount) {
                        currentFrame_ = frameCount - 1;
                        OnClipComplete();
                    } else {
                        currentFrame_ = 0;
                        firedEvents_.clear();
                    }
                }
                break;
        }
    }
    
    void CheckFrameEvents() {
        if (!currentClip_ || !eventCallback_) return;
        
        for (const auto& event : currentClip_->events) {
            if (event.frame == currentFrame_) {
                // 同じフレームで複数回発火しないようにチェック
                std::string eventKey = event.eventName + "_" + std::to_string(event.frame);
                if (firedEvents_.find(eventKey) == firedEvents_.end()) {
                    firedEvents_.insert(eventKey);
                    eventCallback_(event);
                }
            }
        }
    }
    
    void OnClipComplete() {
        // 完了コールバック
        if (completeCallback_) {
            completeCallback_(currentClipId_);
        }
        
        // 次のクリップに自動遷移
        if (!currentClip_->nextClip.empty()) {
            Play(currentClip_->nextClip, true);
        } else if (!nextClipId_.empty()) {
            std::string next = nextClipId_;
            nextClipId_.clear();
            Play(next, true);
        } else {
            state_ = AnimPlayState::Stopped;
        }
    }
    
    const Data::SpriteAnimationDef* animDef_ = nullptr;
    const Data::AnimClipDef* currentClip_ = nullptr;
    Texture2D texture_{};
    bool hasValidTexture_ = false;
    
    // フォールバック設定
    bool fallbackEnabled_ = true;
    Color fallbackColor_{100, 150, 255, 200};  // 青系
    
    std::string currentClipId_;
    std::string nextClipId_;
    int currentFrame_ = 0;
    float frameTime_ = 0.0f;
    int loopCount_ = 0;
    bool pingPongReverse_ = false;
    float speedMultiplier_ = 1.0f;
    
    AnimPlayState state_ = AnimPlayState::Stopped;
    
    AnimEventCallback eventCallback_;
    AnimCompleteCallback completeCallback_;
    
    std::set<std::string> firedEvents_;
};

} // namespace Core
