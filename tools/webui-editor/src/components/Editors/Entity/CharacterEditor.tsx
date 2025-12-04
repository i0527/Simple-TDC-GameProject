import { useState, useEffect } from 'react'
import { CharacterDef } from '../../../types/character'

interface CharacterEditorProps {
  character: CharacterDef
  onSave: (character: CharacterDef) => void
  onCancel: () => void
}

export default function CharacterEditor({
  character,
  onSave,
  onCancel,
}: CharacterEditorProps) {
  const [editedCharacter, setEditedCharacter] = useState<CharacterDef>(character)

  useEffect(() => {
    setEditedCharacter(character)
  }, [character])

  const handleSave = () => {
    onSave(editedCharacter)
  }

  return (
    <div className="h-full overflow-auto p-8">
      <div className="mx-auto max-w-4xl">
        <div className="mb-6 flex items-center justify-between">
          <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
            キャラクター編集: {editedCharacter.name}
          </h2>
          <div className="flex gap-2">
            <button
              onClick={onCancel}
              className="rounded-lg border border-gray-300 px-4 py-2 text-gray-700 hover:bg-gray-50 dark:border-gray-600 dark:text-gray-300 dark:hover:bg-gray-700"
            >
              キャンセル
            </button>
            <button
              onClick={handleSave}
              className="rounded-lg bg-blue-600 px-4 py-2 text-white hover:bg-blue-700 dark:bg-blue-500 dark:hover:bg-blue-600"
            >
              保存
            </button>
          </div>
        </div>

        <div className="space-y-6">
          {/* 基本情報 */}
          <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
            <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
              基本情報
            </h3>
            <div className="grid grid-cols-2 gap-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  ID
                </label>
                <input
                  type="text"
                  value={editedCharacter.id}
                  onChange={(e) =>
                    setEditedCharacter({ ...editedCharacter, id: e.target.value })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  名前
                </label>
                <input
                  type="text"
                  value={editedCharacter.name}
                  onChange={(e) =>
                    setEditedCharacter({ ...editedCharacter, name: e.target.value })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div className="col-span-2">
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  説明
                </label>
                <textarea
                  value={editedCharacter.description}
                  onChange={(e) =>
                    setEditedCharacter({ ...editedCharacter, description: e.target.value })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  rows={3}
                />
              </div>
            </div>
          </section>

          {/* ステータス */}
          <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
            <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
              ステータス
            </h3>
            <div className="grid grid-cols-2 gap-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  HP
                </label>
                <input
                  type="number"
                  value={editedCharacter.stats.maxHealth}
                  onChange={(e) =>
                    setEditedCharacter({
                      ...editedCharacter,
                      stats: {
                        ...editedCharacter.stats,
                        maxHealth: parseInt(e.target.value) || 0,
                      },
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  攻撃力
                </label>
                <input
                  type="number"
                  value={editedCharacter.stats.attack}
                  onChange={(e) =>
                    setEditedCharacter({
                      ...editedCharacter,
                      stats: {
                        ...editedCharacter.stats,
                        attack: parseInt(e.target.value) || 0,
                      },
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  防御力
                </label>
                <input
                  type="number"
                  value={editedCharacter.stats.defense}
                  onChange={(e) =>
                    setEditedCharacter({
                      ...editedCharacter,
                      stats: {
                        ...editedCharacter.stats,
                        defense: parseInt(e.target.value) || 0,
                      },
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  速度
                </label>
                <input
                  type="number"
                  step="0.1"
                  value={editedCharacter.stats.speed}
                  onChange={(e) =>
                    setEditedCharacter({
                      ...editedCharacter,
                      stats: {
                        ...editedCharacter.stats,
                        speed: parseFloat(e.target.value) || 0,
                      },
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
            </div>
          </section>

          {/* JSON直接編集モード（将来実装） */}
          <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
            <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
              JSON直接編集
            </h3>
            <p className="text-sm text-gray-600 dark:text-gray-400">
              この機能は今後実装予定です
            </p>
          </section>
        </div>
      </div>
    </div>
  )
}

