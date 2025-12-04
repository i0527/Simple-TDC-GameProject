import { useState, useEffect } from 'react'
import { useApiClient } from '../../hooks/useApiClient'
import StageList from './Stage/StageList'
import StageEditorCanvas from './Stage/StageEditorCanvas'
import { StageDef } from '../../types/stage'

export default function StageEditor() {
  const { apiClient } = useApiClient()
  const [stages, setStages] = useState<StageDef[]>([])
  const [selectedStage, setSelectedStage] = useState<StageDef | null>(null)
  const [isLoading, setIsLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    loadStages()
  }, [])

  const loadStages = async () => {
    try {
      setIsLoading(true)
      setError(null)
      const data = await apiClient.getStages()
      setStages(data)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'ステージの読み込みに失敗しました')
    } finally {
      setIsLoading(false)
    }
  }

  const handleStageSelect = (stage: StageDef) => {
    setSelectedStage(stage)
  }

  const handleStageSave = async (stage: StageDef) => {
    try {
      if (stage.id && stages.some((s) => s.id === stage.id)) {
        await apiClient.updateStage(stage.id, stage)
      } else {
        await apiClient.createStage(stage)
      }
      await loadStages()
    } catch (err) {
      setError(err instanceof Error ? err.message : 'ステージの保存に失敗しました')
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
        <StageList
          stages={stages}
          selectedStage={selectedStage}
          onSelect={handleStageSelect}
        />
      </div>
      <div className="flex-1 overflow-auto">
        {selectedStage ? (
          <StageEditorCanvas stage={selectedStage} onSave={handleStageSave} />
        ) : (
          <div className="flex h-full items-center justify-center text-gray-500 dark:text-gray-400">
            ステージを選択してください
          </div>
        )}
      </div>
    </div>
  )
}

