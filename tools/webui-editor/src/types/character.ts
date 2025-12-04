export interface CharacterDef {
  id: string
  name: string
  description: string
  rarity: 'common' | 'rare' | 'epic' | 'legendary'
  gameMode: 'td' | 'roguelike' | 'both'
  visual: {
    spriteSheet: string
    spriteSize: { width: number; height: number }
    scale: number
  }
  stats: {
    maxHealth: number
    currentHealth: number
    attack: number
    defense: number
    speed: number
  }
  combat: {
    attackRange: number
    attackCooldown: number
    attackDamage: number
  }
  animations: Array<{
    name: string
    frames: number[]
    duration: number
    loop: boolean
  }>
}

