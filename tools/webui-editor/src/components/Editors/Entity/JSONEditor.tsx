import { useState } from 'react'
import { CharacterDef } from '../../../types/character'

interface JSONEditorProps {
  character: CharacterDef
  onUpdate: (character: CharacterDef) => void
}

export default function JSONEditor({
  character,
  onUpdate,
}: JSONEditorProps) {
  const [jsonText, setJsonText] = useState(JSON.stringify(character, null, 2))
  const [error, setError] = useState<string | null>(null)

  const handleApply = () => {
    try {
      const parsed = JSON.parse(jsonText)
      setError(null)
      onUpdate(parsed as CharacterDef)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'JSON解析エラー')
    }
  }

  const handleFormatJSON = () => {
    try {
      const parsed = JSON.parse(jsonText)
      setJsonText(JSON.stringify(parsed, null, 2))
      setError(null)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'JSON解析エラー')
    }
  }

  return (
    <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
      <div className="mb-4 flex items-center justify-between">
        <h3 className="text-lg font-semibold text-gray-900 dark:text-white">
          JSON直接編集
        </h3>
        <div className="flex gap-2">
          <button
            onClick={handleFormatJSON}
            className="rounded-lg border border-gray-300 px-3 py-1 text-sm text-gray-700 hover:bg-gray-100 dark:border-gray-600 dark:text-gray-300 dark:hover:bg-gray-700"
          >
            フォーマット
          </button>
          <button
            onClick={handleApply}
            className="rounded-lg bg-blue-600 px-3 py-1 text-sm text-white hover:bg-blue-700 dark:bg-blue-500 dark:hover:bg-blue-600"
          >
            適用
          </button>
        </div>
      </div>

      {error && (
        <div className="mb-4 rounded-lg bg-red-100 p-3 text-sm text-red-800 dark:bg-red-900 dark:text-red-200">
          エラー: {error}
        </div>
      )}

      <textarea
        value={jsonText}
        onChange={(e) => setJsonText(e.target.value)}
        className="w-full rounded-lg border border-gray-300 bg-gray-50 p-3 font-mono text-sm dark:border-gray-600 dark:bg-gray-900 dark:text-gray-100"
        rows={20}
      />

      <p className="mt-2 text-xs text-gray-600 dark:text-gray-400">
        JSONを直接編集して「適用」を押すと、キャラクター定義が更新されます。
      </p>
    </section>
  )
}

