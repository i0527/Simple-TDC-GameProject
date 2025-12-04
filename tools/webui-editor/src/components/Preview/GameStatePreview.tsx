import { useGameState } from '../hooks/useGameState'

export default function GameStatePreview() {
  const { gameState, isLoading, error } = useGameState()

  if (isLoading) {
    return (
      <div className="rounded-lg border border-gray-200 bg-white p-4 dark:border-gray-700 dark:bg-gray-800">
        <p className="text-sm text-gray-600 dark:text-gray-400">読み込み中...</p>
      </div>
    )
  }

  if (error) {
    return (
      <div className="rounded-lg border border-red-200 bg-red-50 p-4 dark:border-red-900 dark:bg-red-900/20">
        <p className="text-sm text-red-600 dark:text-red-400">{error}</p>
      </div>
    )
  }

  if (!gameState) {
    return (
      <div className="rounded-lg border border-gray-200 bg-white p-4 dark:border-gray-700 dark:bg-gray-800">
        <p className="text-sm text-gray-600 dark:text-gray-400">ゲーム状態なし</p>
      </div>
    )
  }

  return (
    <div className="rounded-lg border border-gray-200 bg-white p-4 dark:border-gray-700 dark:bg-gray-800">
      <h3 className="mb-3 font-semibold text-gray-900 dark:text-white">ゲーム状態</h3>
      <div className="space-y-2 text-sm text-gray-700 dark:text-gray-300">
        <div className="flex justify-between">
          <span>モード:</span>
          <span className="font-mono text-blue-600 dark:text-blue-400">{gameState.mode}</span>
        </div>
        <div className="flex justify-between">
          <span>シーン:</span>
          <span className="font-mono">{gameState.scene}</span>
        </div>
        <div className="flex justify-between">
          <span>エンティティ数:</span>
          <span className="font-mono">{gameState.entityCount}</span>
        </div>
        <div className="flex justify-between text-xs text-gray-500">
          <span>更新時刻:</span>
          <span>{new Date(gameState.timestamp).toLocaleTimeString()}</span>
        </div>
      </div>
    </div>
  )
}

