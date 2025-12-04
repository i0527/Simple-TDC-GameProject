/**
 * @file Components.h
 * @brief 全コンポーネント定義の統合ヘッダー
 * 
 * Core/Game/TDの各コンポーネントをまとめてインクルード
 */
#pragma once

// Core層コンポーネント（基本的なECS用）
#include "Core/Components/CoreComponents.h"

// Game層コンポーネント（汎用ゲーム用）
#include "Game/Components/GameComponents.h"

// TD層コンポーネント（タワーディフェンス固有）
#include "TD/Components/TDComponents.h"
