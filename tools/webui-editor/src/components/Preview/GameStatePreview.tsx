import { useState, useEffect } from 'react'
import { useApiClient } from '../../../hooks/useApiClient'

export default function GameStatePreview() {
  const { apiClient } = useApiClient()
  const [gameState, setGameState] = useState<any>(null)
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    const fetchGameState = async () => {
      try {
        setLoading(true)
        setError(null)
        const state = await apiClient.getGameState()
        setGameState(state)
      } catch (err) {
        setError(err instanceof Error ? err.message : 'ゲーム状態の取得に失敗しました')
      } finally {
        setLoading(false)
      }
    }

    fetchGameState()
    const interval = setInterval(fetchGameState, 2000) // 2秒ごとに更新

    return () => clearInterval(interval)
  }, [apiClient])

  if (loading) {
    return (
      <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
        <h3 className="text-lg font-semibold text-gray-900 dark:text-white">
          ゲーム状態プレビュー
        </h3>
        <div className="mt-4 text-center text-gray-600 dark:text-gray-400">読み込み中...</div>
      </section>
    )
  }

  if (error) {
    return (
      <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
        <h3 className="text-lg font-semibold text-gray-900 dark:text-white">
          ゲーム状態プレビュー
        </h3>
        <div className="mt-4 rounded-lg bg-yellow-100 p-2 text-sm text-yellow-800 dark:bg-yellow-900 dark:text-yellow-200">
          {error}
        </div>
      </section>
    )
  }

  return (
    <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
      <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
        ゲーム状態プレビュー
      </h3>

      {gameState && (
        <div className="space-y-2 text-sm">
          <div>
            <span className="text-gray-600 dark:text-gray-400">モード:</span>
            <span className="ml-2 font-semibold text-gray-900 dark:text-white">
              {gameState.mode || 'unknown'}
            </span>
          </div>
          <div>
            <span className="text-gray-600 dark:text-gray-400">シーン:</span>
            <span className="ml-2 font-semibold text-gray-900 dark:text-white">
              {gameState.scene || 'unknown'}
            </span>
          </div>
          <div>
            <span className="text-gray-600 dark:text-gray-400">エンティティ:</span>
            <span className="ml-2 font-semibold text-gray-900 dark:text-white">
              {gameState.entities?.total || 0}
            </span>
          </div>

          {gameState.td && (
            <div className="mt-3 rounded-lg bg-blue-50 p-3 dark:bg-blue-900/20">
              <p className="text-xs font-semibold text-blue-900 dark:text-blue-200">
                TD モード情報
              </p>
              <pre className="mt-2 overflow-auto rounded bg-gray-100 p-2 text-xs dark:bg-gray-800">
                {JSON.stringify(gameState.td, null, 2)}
              </pre>
            </div>
          )}
        </div>
      )}

      <p className="mt-4 text-xs text-gray-600 dark:text-gray-400">
        自動更新: 2秒ごと
      </p>
    </section>
  )
}

