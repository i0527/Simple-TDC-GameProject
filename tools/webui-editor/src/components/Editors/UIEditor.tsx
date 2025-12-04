import { useState, useEffect } from 'react'
import { useApiClient } from '../../hooks/useApiClient'
import UILayoutList from './UI/UILayoutList'
import UILayoutEditor from './UI/UILayoutEditor'
import { UILayoutDef } from '../../types/ui'

export default function UIEditor() {
  const { apiClient } = useApiClient()
  const [layouts, setLayouts] = useState<UILayoutDef[]>([])
  const [selectedLayout, setSelectedLayout] = useState<UILayoutDef | null>(null)
  const [isLoading, setIsLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    loadLayouts()
  }, [])

  const loadLayouts = async () => {
    try {
      setIsLoading(true)
      setError(null)
      const data = await apiClient.getUILayouts()
      setLayouts(data)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'UIレイアウトの読み込みに失敗しました')
    } finally {
      setIsLoading(false)
    }
  }

  const handleLayoutSelect = (layout: UILayoutDef) => {
    setSelectedLayout(layout)
  }

  const handleLayoutSave = async (layout: UILayoutDef) => {
    try {
      if (layout.id && layouts.some((l) => l.id === layout.id)) {
        await apiClient.updateUILayout(layout.id, layout)
      } else {
        await apiClient.createUILayout(layout)
      }
      await loadLayouts()
    } catch (err) {
      setError(err instanceof Error ? err.message : 'UIレイアウトの保存に失敗しました')
    }
  }

  if (isLoading) {
    return (
      <div className="flex h-full items-center justify-center">
        <div className="text-gray-600 dark:text-gray-400">読み込み中...</div>
      </div>
    )
  }

  if (error) {
    return (
      <div className="p-8">
        <div className="rounded-lg bg-red-100 p-4 text-red-800 dark:bg-red-900 dark:text-red-200">
          {error}
        </div>
      </div>
    )
  }

  return (
    <div className="flex h-full">
      <div className="w-80 border-r border-gray-200 dark:border-gray-700">
        <UILayoutList
          layouts={layouts}
          selectedLayout={selectedLayout}
          onSelect={handleLayoutSelect}
        />
      </div>
      <div className="flex-1 overflow-auto">
        {selectedLayout ? (
          <UILayoutEditor layout={selectedLayout} onSave={handleLayoutSave} />
        ) : (
          <div className="flex h-full items-center justify-center text-gray-500 dark:text-gray-400">
            UIレイアウトを選択してください
          </div>
        )}
      </div>
    </div>
  )
}

