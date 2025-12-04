import { useEffect, useState } from 'react'
import { ApiClient } from '../api/client'

interface GameStateInfo {
  mode: string
  scene: string
  entityCount: number
  timestamp: number
  td?: {
    waveManager?: any
    gameStateManager?: any
  }
  roguelike?: {
    turnManager?: any
  }
}

export function useGameState() {
  const [gameState, setGameState] = useState<GameStateInfo | null>(null)
  const [isLoading, setIsLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    const fetchGameState = async () => {
      try {
        setIsLoading(true)
        setError(null)
        const apiClient = new ApiClient('http://localhost:8080')
        const state = await apiClient.getGameState()
        setGameState(state)
      } catch (err) {
        setError(err instanceof Error ? err.message : 'ゲーム状態の取得に失敗しました')
      } finally {
        setIsLoading(false)
      }
    }

    // 1秒ごとにゲーム状態を取得
    const interval = setInterval(fetchGameState, 1000)
    fetchGameState()

    return () => clearInterval(interval)
  }, [])

  return { gameState, isLoading, error }
}

