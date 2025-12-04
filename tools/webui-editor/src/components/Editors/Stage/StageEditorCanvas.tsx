import { useState } from 'react'
import { StageDef } from '../../../types/stage'
import WaveNodeEditor from './WaveNodeEditor'
import GameStatePreview from '../../Preview/GameStatePreview'

interface StageEditorCanvasProps {
  stage: StageDef
  onSave: (stage: StageDef) => void
}

export default function StageEditorCanvas({
  stage,
  onSave,
}: StageEditorCanvasProps) {
  const [editedStage, setEditedStage] = useState<StageDef>(stage)
  const [showNodeEditor, setShowNodeEditor] = useState(false)

  return (
    <div className="grid h-full grid-cols-3 gap-4 p-8">
      {/* 左: ステージ設定 */}
      <div className="col-span-1 space-y-4 overflow-auto">
        <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
          <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
            ステージ設定
          </h3>
          <div className="grid gap-4">
            <div>
              <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                ステージ名
              </label>
              <input
                type="text"
                value={editedStage.name}
                onChange={(e) =>
                  setEditedStage({
                    ...editedStage,
                    name: e.target.value,
                  })
                }
                className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
              />
            </div>
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
                className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
              />
            </div>
            <button
              onClick={() => onSave(editedStage)}
              className="rounded-lg bg-blue-600 px-4 py-2 text-white hover:bg-blue-700 dark:bg-blue-500 dark:hover:bg-blue-600"
            >
              保存
            </button>
          </div>
        </section>

        {/* ゲーム状態プレビュー */}
        <GameStatePreview />
      </div>

      {/* 右2列: メインエディター */}
      <div className="col-span-2 flex flex-col">
        <div className="mb-4 flex gap-2">
          <button
            onClick={() => setShowNodeEditor(false)}
            className={`rounded-lg px-4 py-2 ${
              !showNodeEditor
                ? 'bg-blue-600 text-white'
                : 'border border-gray-300 text-gray-700 dark:border-gray-600 dark:text-gray-300'
            }`}
          >
            リスト表示
          </button>
          <button
            onClick={() => setShowNodeEditor(true)}
            className={`rounded-lg px-4 py-2 ${
              showNodeEditor
                ? 'bg-blue-600 text-white'
                : 'border border-gray-300 text-gray-700 dark:border-gray-600 dark:text-gray-300'
            }`}
          >
            ノードエディター
          </button>
        </div>

        <div className="flex-1 rounded-lg border border-gray-200 bg-white dark:border-gray-700 dark:bg-gray-800">
          {showNodeEditor ? (
            <WaveNodeEditor stage={editedStage} onSave={setEditedStage} />
          ) : (
            <div className="p-6">
              <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
                Waves
              </h3>
              <div className="space-y-3">
                {editedStage.waves.map((wave, index) => (
                  <div
                    key={wave.id}
                    className="rounded-lg border border-gray-200 p-4 dark:border-gray-600"
                  >
                    <div className="flex items-center justify-between">
                      <h4 className="font-semibold text-gray-900 dark:text-white">
                        Wave {index + 1}
                      </h4>
                      <span className="text-sm text-gray-600 dark:text-gray-400">
                        {wave.enemies.length} enemies
                      </span>
                    </div>
                    <div className="mt-2 space-y-1 text-sm text-gray-600 dark:text-gray-400">
                      <p>Delay: {wave.delay}s</p>
                      {wave.enemies.slice(0, 3).map((enemy, i) => (
                        <p key={i} className="ml-2">
                          • {enemy.characterId} x{enemy.count}
                        </p>
                      ))}
                      {wave.enemies.length > 3 && (
                        <p className="ml-2 text-gray-500">
                          ... +{wave.enemies.length - 3} more
                        </p>
                      )}
                    </div>
                  </div>
                ))}
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  )
}
