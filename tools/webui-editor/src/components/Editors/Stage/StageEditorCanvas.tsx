import { useState } from 'react'
import { StageDef } from '../../../types/stage'

interface StageEditorCanvasProps {
  stage: StageDef
  onSave: (stage: StageDef) => void
}

export default function StageEditorCanvas({
  stage,
  onSave,
}: StageEditorCanvasProps) {
  const [editedStage, setEditedStage] = useState<StageDef>(stage)

  return (
    <div className="h-full overflow-auto p-8">
      <div className="mx-auto max-w-6xl">
        <div className="mb-6 flex items-center justify-between">
          <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
            ステージ編集: {editedStage.name}
          </h2>
          <button
            onClick={() => onSave(editedStage)}
            className="rounded-lg bg-blue-600 px-4 py-2 text-white hover:bg-blue-700 dark:bg-blue-500 dark:hover:bg-blue-600"
          >
            保存
          </button>
        </div>

        <div className="space-y-6">
          {/* ステージ設定 */}
          <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
            <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
              ステージ設定
            </h3>
            <div className="grid grid-cols-2 gap-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  レーン数
                </label>
                <input
                  type="number"
                  value={editedStage.lanes}
                  onChange={(e) =>
                    setEditedStage({
                      ...editedStage,
                      lanes: parseInt(e.target.value) || 0,
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  拠点HP
                </label>
                <input
                  type="number"
                  value={editedStage.baseHealth}
                  onChange={(e) =>
                    setEditedStage({
                      ...editedStage,
                      baseHealth: parseInt(e.target.value) || 0,
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:bg-gray-700 dark:text-white"
                />
              </div>
            </div>
          </section>

          {/* Wave編集（簡易版） */}
          <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
            <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
              Waves
            </h3>
            <p className="text-sm text-gray-600 dark:text-gray-400">
              ノードベースのWave編集は今後実装予定です
            </p>
            <div className="mt-4 space-y-2">
              {editedStage.waves.map((wave, index) => (
                <div
                  key={wave.id}
                  className="rounded-lg border border-gray-200 p-4 dark:border-gray-600"
                >
                  <h4 className="font-semibold text-gray-900 dark:text-white">
                    Wave {index + 1}
                  </h4>
                  <p className="text-sm text-gray-600 dark:text-gray-400">
                    {wave.enemies.length} enemies
                  </p>
                </div>
              ))}
            </div>
          </section>
        </div>
      </div>
    </div>
  )
}

