// スキル定義型
export interface SkillDef {
  id: string
  name: string
  description: string

  // 発動条件
  cooldown: number             // クールダウン（秒）
  activationChance: number     // 発動確率（0.0-1.0）
  activateOnAttack: boolean    // 攻撃時に発動
  activateOnDamaged: boolean   // 被ダメージ時に発動
  activateOnDeath: boolean     // 死亡時に発動
  healthThreshold: number      // HP閾値

  // ターゲティング
  targetType: 'self' | 'single_enemy' | 'single_ally' | 'all_enemies' | 'all_allies' | 'area'
  effectArea?: {
    x: number
    y: number
    width: number
    height: number
  }
  maxTargets: number           // 最大ターゲット数

  // 効果
  effects: SkillEffect[]

  // アニメーション
  animationName: string
  effectSpritePath: string
}

export interface SkillEffect {
  type: 'Damage' | 'Heal' | 'StatusApply' | 'Summon' | 'Knockback' | 'Pull'
  value: number
  isPercentage: boolean
  statusEffectId?: string      // StatusApplyの場合
  summonCharacterId?: string   // Summonの場合
  summonCount?: number         // 召喚数
}

export interface AbilityDef {
  id: string
  name: string
  description: string
  type: 'passive' | 'active' | 'special'

  // 能力の効果
  effects: {
    type: string
    value: number
    parameters: Record<string, any>
  }[]

  // 条件
  condition?: {
    type: 'hp_below' | 'hp_above' | 'on_hit' | 'on_damage'
    value: number
  }

  // クールダウン（アクティブ能力用）
  cooldown?: number

  // ゲームモード
  gameMode: 'TD' | 'Roguelike' | 'Both'
}

export interface StatusEffectDef {
  id: string
  name: string
  type: 'Slow' | 'Stun' | 'Poison' | 'Burn' | 'Freeze' | 'AttackUp' | 'AttackDown' | 'DefenseUp' | 'DefenseDown' | 'SpeedUp' | 'Regeneration' | 'Shield' | 'Invincible'

  value: number                // 効果値（%またはフラット値）
  duration: number             // 効果時間（秒）
  tickInterval: number         // 継続ダメージ/回復の間隔
  isPercentage: boolean        // 値が%かどうか
  iconPath: string             // UIアイコン
}

