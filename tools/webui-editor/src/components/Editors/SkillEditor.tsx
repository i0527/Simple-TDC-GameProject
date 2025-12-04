import { useState } from 'react'
import { SkillDef, SkillEffect } from '../../../types/skill'

interface SkillEditorProps {
  skill: SkillDef
  onSave: (skill: SkillDef) => void
  onCancel: () => void
}

export default function SkillEditor({
  skill,
  onSave,
  onCancel,
}: SkillEditorProps) {
  const [editedSkill, setEditedSkill] = useState<SkillDef>(skill)

  const handleAddEffect = () => {
    const newEffect: SkillEffect = {
      type: 'Damage',
      value: 10,
      isPercentage: false,
    }
    setEditedSkill({
      ...editedSkill,
      effects: [...editedSkill.effects, newEffect],
    })
  }

  const handleRemoveEffect = (index: number) => {
    setEditedSkill({
      ...editedSkill,
      effects: editedSkill.effects.filter((_, i) => i !== index),
    })
  }

  const handleUpdateEffect = (index: number, key: string, value: any) => {
    const updatedEffects = [...editedSkill.effects]
    updatedEffects[index] = { ...updatedEffects[index], [key]: value }
    setEditedSkill({
      ...editedSkill,
      effects: updatedEffects,
    })
  }

  return (
    <div className="h-full overflow-auto p-8">
      <div className="mx-auto max-w-4xl">
        <div className="mb-6 flex items-center justify-between">
          <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
            スキル編集: {editedSkill.name}
          </h2>
          <div className="flex gap-2">
            <button
              onClick={onCancel}
              className="rounded-lg border border-gray-300 px-4 py-2 text-gray-700 hover:bg-gray-50 dark:border-gray-600 dark:text-gray-300 dark:hover:bg-gray-700"
            >
              キャンセル
            </button>
            <button
              onClick={() => onSave(editedSkill)}
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
                  value={editedSkill.id}
                  onChange={(e) =>
                    setEditedSkill({ ...editedSkill, id: e.target.value })
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
                  value={editedSkill.name}
                  onChange={(e) =>
                    setEditedSkill({ ...editedSkill, name: e.target.value })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div className="col-span-2">
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  説明
                </label>
                <textarea
                  value={editedSkill.description}
                  onChange={(e) =>
                    setEditedSkill({ ...editedSkill, description: e.target.value })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  rows={3}
                />
              </div>
            </div>
          </section>

          {/* 発動条件 */}
          <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
            <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
              発動条件
            </h3>
            <div className="grid grid-cols-2 gap-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  クールダウン（秒）
                </label>
                <input
                  type="number"
                  step="0.1"
                  value={editedSkill.cooldown}
                  onChange={(e) =>
                    setEditedSkill({
                      ...editedSkill,
                      cooldown: parseFloat(e.target.value) || 0,
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  発動確率
                </label>
                <input
                  type="number"
                  min="0"
                  max="1"
                  step="0.1"
                  value={editedSkill.activationChance}
                  onChange={(e) =>
                    setEditedSkill({
                      ...editedSkill,
                      activationChance: parseFloat(e.target.value) || 0,
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div className="col-span-2 space-y-2">
                <label className="flex items-center gap-2">
                  <input
                    type="checkbox"
                    checked={editedSkill.activateOnAttack}
                    onChange={(e) =>
                      setEditedSkill({
                        ...editedSkill,
                        activateOnAttack: e.target.checked,
                      })
                    }
                    className="rounded"
                  />
                  <span className="text-sm text-gray-700 dark:text-gray-300">
                    攻撃時に発動
                  </span>
                </label>
                <label className="flex items-center gap-2">
                  <input
                    type="checkbox"
                    checked={editedSkill.activateOnDamaged}
                    onChange={(e) =>
                      setEditedSkill({
                        ...editedSkill,
                        activateOnDamaged: e.target.checked,
                      })
                    }
                    className="rounded"
                  />
                  <span className="text-sm text-gray-700 dark:text-gray-300">
                    被ダメージ時に発動
                  </span>
                </label>
                <label className="flex items-center gap-2">
                  <input
                    type="checkbox"
                    checked={editedSkill.activateOnDeath}
                    onChange={(e) =>
                      setEditedSkill({
                        ...editedSkill,
                        activateOnDeath: e.target.checked,
                      })
                    }
                    className="rounded"
                  />
                  <span className="text-sm text-gray-700 dark:text-gray-300">
                    死亡時に発動
                  </span>
                </label>
              </div>
            </div>
          </section>

          {/* ターゲティング */}
          <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
            <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
              ターゲティング
            </h3>
            <div className="grid grid-cols-2 gap-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  ターゲットタイプ
                </label>
                <select
                  value={editedSkill.targetType}
                  onChange={(e) =>
                    setEditedSkill({
                      ...editedSkill,
                      targetType: e.target.value as any,
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                >
                  <option value="self">自分</option>
                  <option value="single_enemy">敵単体</option>
                  <option value="single_ally">味方単体</option>
                  <option value="all_enemies">敵全体</option>
                  <option value="all_allies">味方全体</option>
                  <option value="area">範囲</option>
                </select>
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  最大ターゲット数
                </label>
                <input
                  type="number"
                  value={editedSkill.maxTargets}
                  onChange={(e) =>
                    setEditedSkill({
                      ...editedSkill,
                      maxTargets: parseInt(e.target.value) || 1,
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
            </div>
          </section>

          {/* スキル効果 */}
          <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
            <div className="mb-4 flex items-center justify-between">
              <h3 className="text-lg font-semibold text-gray-900 dark:text-white">
                スキル効果
              </h3>
              <button
                onClick={handleAddEffect}
                className="rounded-lg bg-green-600 px-3 py-1 text-sm text-white hover:bg-green-700 dark:bg-green-500 dark:hover:bg-green-600"
              >
                + 効果追加
              </button>
            </div>

            <div className="space-y-4">
              {editedSkill.effects.map((effect, index) => (
                <div
                  key={index}
                  className="rounded-lg border border-gray-300 p-4 dark:border-gray-600"
                >
                  <div className="mb-3 flex items-center justify-between">
                    <select
                      value={effect.type}
                      onChange={(e) =>
                        handleUpdateEffect(index, 'type', e.target.value)
                      }
                      className="rounded-lg border border-gray-300 px-3 py-1 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                    >
                      <option value="Damage">ダメージ</option>
                      <option value="Heal">回復</option>
                      <option value="StatusApply">ステータス付与</option>
                      <option value="Summon">召喚</option>
                      <option value="Knockback">ノックバック</option>
                      <option value="Pull">引き寄せ</option>
                    </select>
                    <button
                      onClick={() => handleRemoveEffect(index)}
                      className="rounded px-2 py-1 text-red-600 hover:bg-red-100 dark:text-red-400 dark:hover:bg-red-900/20"
                    >
                      削除
                    </button>
                  </div>

                  <div className="grid grid-cols-2 gap-3">
                    <div>
                      <label className="block text-xs text-gray-700 dark:text-gray-300">
                        値
                      </label>
                      <input
                        type="number"
                        value={effect.value}
                        onChange={(e) =>
                          handleUpdateEffect(index, 'value', parseFloat(e.target.value) || 0)
                        }
                        className="mt-1 w-full rounded-lg border border-gray-300 px-2 py-1 text-sm dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                      />
                    </div>
                    <div>
                      <label className="flex items-center gap-2 pt-6 text-xs text-gray-700 dark:text-gray-300">
                        <input
                          type="checkbox"
                          checked={effect.isPercentage}
                          onChange={(e) =>
                            handleUpdateEffect(index, 'isPercentage', e.target.checked)
                          }
                          className="rounded"
                        />
                        <span>%</span>
                      </label>
                    </div>
                  </div>
                </div>
              ))}
            </div>
          </section>

          {/* アニメーション設定 */}
          <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
            <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
              アニメーション設定
            </h3>
            <div className="grid grid-cols-2 gap-4">
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  アニメーション名
                </label>
                <input
                  type="text"
                  value={editedSkill.animationName}
                  onChange={(e) =>
                    setEditedSkill({
                      ...editedSkill,
                      animationName: e.target.value,
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div>
                <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                  エフェクトスプライトパス
                </label>
                <input
                  type="text"
                  value={editedSkill.effectSpritePath}
                  onChange={(e) =>
                    setEditedSkill({
                      ...editedSkill,
                      effectSpritePath: e.target.value,
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
            </div>
          </section>
        </div>
      </div>
    </div>
  )
}

