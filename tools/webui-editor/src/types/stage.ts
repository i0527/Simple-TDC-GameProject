export interface StageDef {
  id: string
  name: string
  description: string
  lanes: number
  baseHealth: number
  waves: Array<{
    id: string
    delay: number
    enemies: Array<{
      characterId: string
      lane: number
      delay: number
      count: number
      interval: number
    }>
  }>
}

