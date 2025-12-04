import { useEffect, useState, useRef } from 'react'
import { useApiClient } from '../../hooks/useApiClient'

export default function ServerEventListener() {
  const { apiClient } = useApiClient()
  const eventSourceRef = useRef<EventSource | null>(null)
  const [lastUpdate, setLastUpdate] = useState<{ type: string; timestamp: number } | null>(null)

  useEffect(() => {
    connectToEventStream()

    return () => {
      if (eventSourceRef.current) {
        eventSourceRef.current.close()
      }
    }
  }, [])

  const connectToEventStream = () => {
    try {
      eventSourceRef.current = new EventSource('http://localhost:8080/events')

      eventSourceRef.current.addEventListener('file_changed', (event) => {
        const data = JSON.parse(event.data)
        console.log('File changed:', data)
        setLastUpdate({ type: 'file_changed', timestamp: Date.now() })
        // ここでページのリロードやキャッシュクリアを実行
        window.location.reload()
      })

      eventSourceRef.current.addEventListener('character_reloaded', (event) => {
        const data = JSON.parse(event.data)
        console.log('Character reloaded:', data)
        setLastUpdate({ type: 'character_reloaded', timestamp: Date.now() })
      })

      eventSourceRef.current.addEventListener('stage_reloaded', (event) => {
        const data = JSON.parse(event.data)
        console.log('Stage reloaded:', data)
        setLastUpdate({ type: 'stage_reloaded', timestamp: Date.now() })
      })

      eventSourceRef.current.addEventListener('ui_reloaded', (event) => {
        const data = JSON.parse(event.data)
        console.log('UI reloaded:', data)
        setLastUpdate({ type: 'ui_reloaded', timestamp: Date.now() })
      })

      eventSourceRef.current.onerror = () => {
        if (eventSourceRef.current) {
          eventSourceRef.current.close()
        }
        // 3秒後に再接続を試みる
        setTimeout(connectToEventStream, 3000)
      }
    } catch (error) {
      console.error('Failed to connect to event stream:', error)
    }
  }

  if (!lastUpdate) {
    return null
  }

  return (
    <div className="fixed bottom-4 right-4 rounded-lg bg-green-500 px-4 py-2 text-white shadow-lg">
      <p className="text-sm">{lastUpdate.type} - {new Date(lastUpdate.timestamp).toLocaleTimeString()}</p>
    </div>
  )
}

