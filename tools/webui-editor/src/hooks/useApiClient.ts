import { useState, useEffect, createContext, useContext } from 'react'
import { ApiClient } from '../api/client'

interface ApiContextValue {
  apiClient: ApiClient
  isConnected: boolean
  serverStatus: { version?: string } | null
}

const ApiContext = createContext<ApiContextValue | null>(null)

export function ApiProvider({ children }: { children: React.ReactNode }) {
  const [apiClient] = useState(() => new ApiClient())
  const [isConnected, setIsConnected] = useState(false)
  const [serverStatus, setServerStatus] = useState<{ version?: string } | null>(null)

  useEffect(() => {
    const checkConnection = async () => {
      try {
        const status = await apiClient.healthCheck()
        setIsConnected(true)
        setServerStatus(status)
      } catch (error) {
        setIsConnected(false)
        setServerStatus(null)
      }
    }

    checkConnection()
    const interval = setInterval(checkConnection, 5000) // 5秒ごとにチェック

    return () => clearInterval(interval)
  }, [apiClient])

  return (
    <ApiContext.Provider value={{ apiClient, isConnected, serverStatus }}>
      {children}
    </ApiContext.Provider>
  )
}

export function useApiClient() {
  const context = useContext(ApiContext)
  if (!context) {
    throw new Error('useApiClient must be used within ApiProvider')
  }
  return context
}

