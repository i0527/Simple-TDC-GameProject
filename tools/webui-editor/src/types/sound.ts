// サウンド定義型

export interface SoundVariation {
  filePath: string
  weight: number
  pitchOffset: number
  volumeOffset: number
}

export interface SoundDef {
  id: string
  name: string
  type: 'sfx' | 'voice' | 'ambient' | 'ui' | 'music'
  priority: 'low' | 'normal' | 'high' | 'critical'

  // ファイル
  variations: SoundVariation[]

  // 再生設定
  volume: number
  pitch: number
  pitchVariation: number
  volumeVariation: number
  loop: boolean

  // 3Dサウンド
  is3D: boolean
  minDistance: number
  maxDistance: number
  rolloffFactor: number

  // 再生制限
  maxInstances: number
  cooldown: number
  stopOldest: boolean

  // フェード
  fadeInTime: number
  fadeOutTime: number
  fadeType: 'none' | 'linear' | 'easeIn' | 'easeOut' | 'easeInOut'

  // グループ
  group: string
  tags: string[]
}

export interface MusicDef {
  id: string
  name: string
  filePath: string

  // 基本設定
  volume: number
  bpm: number
  beatsPerBar: number

  // ループ設定
  loopEnabled: boolean
  loopStart: number
  loopEnd: number

  // イントロ/アウトロ
  introFilePath?: string
  outroFilePath?: string

  // トランジション
  crossfadeDuration: number
  crossfadeType: 'none' | 'linear' | 'easeIn' | 'easeOut' | 'easeInOut'

  // グループ
  group: string
  tags: string[]
}

export interface SoundCue {
  id: string
  soundId: string
  delay: number
  probability: number
  condition?: string
}

export interface SoundEvent {
  id: string
  name: string
  cues: SoundCue[]
  playMode: 'all' | 'random' | 'sequence'
  cooldown: number
}

export interface SoundBankDef {
  id: string
  name: string

  // 含まれるサウンド
  soundIds: string[]
  musicIds: string[]

  // サウンドイベント
  events: Record<string, SoundEvent>

  // バンク設定
  preload: boolean
  persistent: boolean

  tags: string[]
}

