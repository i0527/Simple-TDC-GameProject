import { useState, useEffect } from 'react'
import { useApiClient } from '../../hooks/useApiClient'

export default function Header() {
  const { isConnected, serverStatus } = useApiClient()
  const [connectionStatus, setConnectionStatus] = useState<'connected' | 'disconnected' | 'connecting'>('disconnected')

  useEffect(() => {
    if (isConnected) {
      setConnectionStatus('connected')
    } else {
      setConnectionStatus('disconnected')
    }
  }, [isConnected])

  return (
    <header className="flex h-16 items-center justify-between border-b border-gray-200 bg-white px-6 dark:border-gray-700 dark:bg-gray-800">
      <div className="flex items-center gap-4">
        <h1 className="text-xl font-bold text-gray-900 dark:text-white">
          Simple TDC Game - WebUI Editor
        </h1>
        <div className="flex items-center gap-2">
          <div
            className={`h-2 w-2 rounded-full ${
              connectionStatus === 'connected'
                ? 'bg-green-500'
                : connectionStatus === 'connecting'
                ? 'bg-yellow-500'
                : 'bg-red-500'
            }`}
          />
          <span className="text-sm text-gray-600 dark:text-gray-400">
            {connectionStatus === 'connected'
              ? 'サーバー接続中'
              : connectionStatus === 'connecting'
              ? '接続中...'
              : 'サーバー未接続'}
          </span>
        </div>
      </div>
      <div className="flex items-center gap-4">
        {serverStatus && (
          <span className="text-sm text-gray-600 dark:text-gray-400">
            API: {serverStatus.version || 'v1.0'}
          </span>
        )}
      </div>
    </header>
  )
}

