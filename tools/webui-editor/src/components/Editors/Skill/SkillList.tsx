import { useState, useEffect } from 'react'
import { SkillDef } from '../../../types/skill'
import SkillEditor from './SkillEditor'
import { useApiClient } from '../../../hooks/useApiClient'

export default function SkillList() {
  const { apiClient } = useApiClient()
  const [skills, setSkills] = useState<SkillDef[]>([])
  const [loading, setLoading] = useState(false)
  const [editingSkill, setEditingSkill] = useState<SkillDef | null>(null)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    loadSkills()
  }, [])

  const loadSkills = async () => {
    try {
      setLoading(true)
      setError(null)
      const data = await apiClient.getSkills?.() || []
      setSkills(data)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'スキルの読み込みに失敗しました')
    } finally {
      setLoading(false)
    }
  }

  const handleCreate = () => {
    const newSkill: SkillDef = {
      id: `skill_${Date.now()}`,
      name: '新規スキル',
      description: '',
      cooldown: 1.0,
      activationChance: 1.0,
      activateOnAttack: false,
      activateOnDamaged: false,
      activateOnDeath: false,
      healthThreshold: 0,
      targetType: 'single_enemy',
      maxTargets: 1,
      effects: [],
      animationName: '',
      effectSpritePath: '',
    }
    setEditingSkill(newSkill)
  }

  const handleSave = async (skill: SkillDef) => {
    try {
      if (skills.find((s) => s.id === skill.id)) {
        await apiClient.updateSkill?.(skill.id, skill)
        setSkills(skills.map((s) => (s.id === skill.id ? skill : s)))
      } else {
        await apiClient.createSkill?.(skill)
        setSkills([...skills, skill])
      }
      setEditingSkill(null)
      setError(null)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'スキルの保存に失敗しました')
    }
  }

  const handleDelete = async (id: string) => {
    if (confirm('このスキルを削除しますか？')) {
      try {
        await apiClient.deleteSkill?.(id)
        setSkills(skills.filter((s) => s.id !== id))
      } catch (err) {
        setError(err instanceof Error ? err.message : 'スキルの削除に失敗しました')
      }
    }
  }

  if (editingSkill) {
    return (
      <SkillEditor
        skill={editingSkill}
        onSave={handleSave}
        onCancel={() => setEditingSkill(null)}
      />
    )
  }

  return (
    <div className="h-full overflow-auto p-8">
      <div className="mx-auto max-w-6xl">
        <div className="mb-6 flex items-center justify-between">
          <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
            スキル一覧
          </h2>
          <button
            onClick={handleCreate}
            className="rounded-lg bg-blue-600 px-4 py-2 text-white hover:bg-blue-700 dark:bg-blue-500 dark:hover:bg-blue-600"
          >
            + 新規スキル
          </button>
        </div>

        {error && (
          <div className="mb-4 rounded-lg bg-red-100 p-3 text-red-800 dark:bg-red-900 dark:text-red-200">
            {error}
          </div>
        )}

        {loading ? (
          <div className="text-center text-gray-600 dark:text-gray-400">
            読み込み中...
          </div>
        ) : skills.length === 0 ? (
          <div className="rounded-lg border border-gray-200 bg-white p-8 text-center dark:border-gray-700 dark:bg-gray-800">
            <p className="text-gray-600 dark:text-gray-400">
              スキルがまだ作成されていません
            </p>
          </div>
        ) : (
          <div className="grid grid-cols-1 gap-4 md:grid-cols-2 lg:grid-cols-3">
            {skills.map((skill) => (
              <div
                key={skill.id}
                className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800"
              >
                <div className="mb-2 flex items-start justify-between">
                  <div>
                    <h3 className="font-semibold text-gray-900 dark:text-white">
                      {skill.name}
                    </h3>
                    <p className="text-xs text-gray-600 dark:text-gray-400">
                      ID: {skill.id}
                    </p>
                  </div>
                  <div className="flex gap-1">
                    <button
                      onClick={() => setEditingSkill(skill)}
                      className="rounded px-2 py-1 text-blue-600 hover:bg-blue-100 dark:text-blue-400 dark:hover:bg-blue-900/20"
                    >
                      編集
                    </button>
                    <button
                      onClick={() => handleDelete(skill.id)}
                      className="rounded px-2 py-1 text-red-600 hover:bg-red-100 dark:text-red-400 dark:hover:bg-red-900/20"
                    >
                      削除
                    </button>
                  </div>
                </div>

                <p className="mb-3 text-sm text-gray-700 dark:text-gray-300">
                  {skill.description}
                </p>

                <div className="space-y-1 border-t border-gray-200 pt-3 text-xs dark:border-gray-700">
                  <div className="flex justify-between">
                    <span className="text-gray-600 dark:text-gray-400">
                      クールダウン:
                    </span>
                    <span className="font-semibold text-gray-900 dark:text-white">
                      {skill.cooldown}s
                    </span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-gray-600 dark:text-gray-400">
                      ターゲット:
                    </span>
                    <span className="font-semibold text-gray-900 dark:text-white">
                      {skill.targetType}
                    </span>
                  </div>
                  <div className="flex justify-between">
                    <span className="text-gray-600 dark:text-gray-400">
                      効果数:
                    </span>
                    <span className="font-semibold text-gray-900 dark:text-white">
                      {skill.effects.length}
                    </span>
                  </div>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  )
}

