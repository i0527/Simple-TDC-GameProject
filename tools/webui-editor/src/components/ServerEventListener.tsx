import { useEffect } from 'react'
import { useApiClient } from '../../hooks/useApiClient'

export default function ServerEventListener() {
  const { apiClient } = useApiClient()

  useEffect(() => {
    const connectSSE = () => {
      try {
        const eventSource = new EventSource('/events')

        eventSource.addEventListener('character_reloaded', (event) => {
          console.log('Character reloaded:', event.data)
          // キャラクター更新イベント
        })

        eventSource.addEventListener('stage_reloaded', (event) => {
          console.log('Stage reloaded:', event.data)
          // ステージ更新イベント
        })

        eventSource.addEventListener('ui_reloaded', (event) => {
          console.log('UI reloaded:', event.data)
          // UI更新イベント
        })

        eventSource.addEventListener('error', () => {
          console.log('SSE connection closed')
          eventSource.close()
          // 3秒後に再接続
          setTimeout(connectSSE, 3000)
        })

        return () => eventSource.close()
      } catch (error) {
        console.error('Failed to connect to SSE:', error)
        // 3秒後に再接続
        setTimeout(connectSSE, 3000)
      }
    }

    const cleanup = connectSSE()
    return cleanup
  }, [])

  return null // このコンポーネントはUIを描画しない
}

