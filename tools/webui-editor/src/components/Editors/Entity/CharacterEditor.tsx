import { useState, useEffect } from 'react'
import { CharacterDef } from '../../../types/character'
import AnimationEditor from './AnimationEditor'
import JSONEditor from './JSONEditor'
import CharacterPreview from './CharacterPreview'

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
  const [activeTab, setActiveTab] = useState<'basic' | 'animation' | 'json' | 'preview'>('basic')

  useEffect(() => {
    setEditedCharacter(character)
  }, [character])

  const handleSave = () => {
    onSave(editedCharacter)
  }

  const tabs = [
    { id: 'basic', label: '基本情報' },
    { id: 'animation', label: 'アニメーション' },
    { id: 'json', label: 'JSON編集' },
    { id: 'preview', label: 'プレビュー' },
  ] as const

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

        {/* タブナビゲーション */}
        <div className="mb-6 flex gap-2 border-b border-gray-200 dark:border-gray-700">
          {tabs.map((tab) => (
            <button
              key={tab.id}
              onClick={() => setActiveTab(tab.id)}
              className={`px-4 py-2 font-medium transition-colors ${
                activeTab === tab.id
                  ? 'border-b-2 border-blue-600 text-blue-600 dark:border-blue-400 dark:text-blue-400'
                  : 'text-gray-600 hover:text-gray-900 dark:text-gray-400 dark:hover:text-gray-200'
              }`}
            >
              {tab.label}
            </button>
          ))}
        </div>

        {/* コンテンツ */}
        <div className="space-y-6">
          {/* 基本情報タブ */}
          {activeTab === 'basic' && (
            <>
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
                  <div>
                    <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                      レアリティ
                    </label>
                    <select
                      value={editedCharacter.rarity}
                      onChange={(e) =>
                        setEditedCharacter({
                          ...editedCharacter,
                          rarity: e.target.value as any,
                        })
                      }
                      className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                    >
                      <option value="common">通常</option>
                      <option value="rare">レア</option>
                      <option value="epic">エピック</option>
                      <option value="legendary">レジェンダリー</option>
                    </select>
                  </div>
                  <div>
                    <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                      ゲームモード
                    </label>
                    <select
                      value={editedCharacter.gameMode}
                      onChange={(e) =>
                        setEditedCharacter({
                          ...editedCharacter,
                          gameMode: e.target.value as any,
                        })
                      }
                      className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                    >
                      <option value="td">TD</option>
                      <option value="roguelike">ローグライク</option>
                      <option value="both">両方</option>
                    </select>
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
                            currentHealth: parseInt(e.target.value) || 0,
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

              {/* 戦闘設定 */}
              <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
                <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
                  戦闘設定
                </h3>
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                      攻撃範囲
                    </label>
                    <input
                      type="number"
                      value={editedCharacter.combat.attackRange}
                      onChange={(e) =>
                        setEditedCharacter({
                          ...editedCharacter,
                          combat: {
                            ...editedCharacter.combat,
                            attackRange: parseInt(e.target.value) || 0,
                          },
                        })
                      }
                      className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                    />
                  </div>
                  <div>
                    <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                      攻撃クールダウン(秒)
                    </label>
                    <input
                      type="number"
                      step="0.1"
                      value={editedCharacter.combat.attackCooldown}
                      onChange={(e) =>
                        setEditedCharacter({
                          ...editedCharacter,
                          combat: {
                            ...editedCharacter.combat,
                            attackCooldown: parseFloat(e.target.value) || 0,
                          },
                        })
                      }
                      className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                    />
                  </div>
                  <div>
                    <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                      攻撃ダメージ
                    </label>
                    <input
                      type="number"
                      value={editedCharacter.combat.attackDamage}
                      onChange={(e) =>
                        setEditedCharacter({
                          ...editedCharacter,
                          combat: {
                            ...editedCharacter.combat,
                            attackDamage: parseInt(e.target.value) || 0,
                          },
                        })
                      }
                      className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                    />
                  </div>
                </div>
              </section>
            </>
          )}

          {/* アニメーションタブ */}
          {activeTab === 'animation' && (
            <AnimationEditor character={editedCharacter} onChange={setEditedCharacter} />
          )}

          {/* JSON編集タブ */}
          {activeTab === 'json' && (
            <JSONEditor character={editedCharacter} onUpdate={setEditedCharacter} />
          )}

          {/* プレビュータブ */}
          {activeTab === 'preview' && <CharacterPreview character={editedCharacter} />}
        </div>
      </div>
    </div>
  )
}
