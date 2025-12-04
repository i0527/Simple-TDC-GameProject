// エフェクト定義型

export interface ParticleEmitterDef {
  id: string
  name: string

  // スプライト設定
  textureId: string
  spriteIndex: number
  animated: boolean
  frameCount: number
  frameRate: number

  // エミッター設定
  shape: 'point' | 'circle' | 'rectangle' | 'line' | 'ring' | 'cone'
  shapeRadius?: number
  shapeWidth?: number
  shapeHeight?: number
  emissionMode: 'continuous' | 'burst' | 'distance'
  emissionRate: number

  // 初期値
  lifetimeMin: number
  lifetimeMax: number
  speedMin: number
  speedMax: number
  angleMin: number
  angleMax: number

  // レンダリング
  blendMode: 'alpha' | 'additive' | 'multiply' | 'screen'
  maxParticles: number
}

export interface ScreenEffectDef {
  id: string
  name: string
  type: 'shake' | 'flash' | 'fadeIn' | 'fadeOut' | 'vignette' | 'zoom' | 'blur' | 'slowMotion'

  duration: number
  easing: 'linear' | 'easeIn' | 'easeOut' | 'easeInOut'

  // Shake用
  shakeIntensity?: number
  shakeFrequency?: number

  // Zoom用
  zoomAmount?: number

  // Blur用
  blurRadius?: number

  // SlowMotion用
  timeScale?: number
}

export interface ParticleEffectDef {
  id: string
  name: string

  emitters: ParticleEmitterDef[]

  duration: number
  loop: boolean
  autoDestroy: boolean
  scale: number

  // サウンド連携
  startSoundId?: string
  endSoundId?: string

  tags: string[]
}

export interface CompositeEffectDef {
  id: string
  name: string

  // 各種エフェクト
  particleEffects: {
    effectId: string
    startTime: number
    offsetX: number
    offsetY: number
    scale: number
  }[]

  screenEffects: {
    effectId: string
    startTime: number
  }[]

  // サウンド
  sounds: {
    soundId: string
    startTime: number
  }[]

  duration: number
  loop: boolean

  tags: string[]
}

