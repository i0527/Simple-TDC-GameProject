import { useState } from 'react'
import { UILayoutDef } from '../../../types/ui'

interface UILayoutEditorProps {
  layout: UILayoutDef
  onSave: (layout: UILayoutDef) => void
}

export default function UILayoutEditor({
  layout,
  onSave,
}: UILayoutEditorProps) {
  const [editedLayout, setEditedLayout] = useState<UILayoutDef>(layout)

  return (
    <div className="h-full overflow-auto p-8">
      <div className="mx-auto max-w-6xl">
        <div className="mb-6 flex items-center justify-between">
          <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
            UIレイアウト編集: {editedLayout.name}
          </h2>
          <button
            onClick={() => onSave(editedLayout)}
            className="rounded-lg bg-blue-600 px-4 py-2 text-white hover:bg-blue-700 dark:bg-blue-500 dark:hover:bg-blue-600"
          >
            保存
          </button>
        </div>

        <div className="space-y-6">
          {/* UI要素編集（簡易版） */}
          <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
            <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
              UI要素
            </h3>
            <p className="text-sm text-gray-600 dark:text-gray-400">
              ドラッグ&ドロップでのWYSIWYG編集は今後実装予定です
            </p>
            <div className="mt-4 space-y-2">
              {editedLayout.elements.map((element) => (
                <div
                  key={element.id}
                  className="rounded-lg border border-gray-200 p-4 dark:border-gray-600"
                >
                  <h4 className="font-semibold text-gray-900 dark:text-white">
                    {element.type}: {element.id}
                  </h4>
                  <p className="text-sm text-gray-600 dark:text-gray-400">
                    Position: ({element.position.x}, {element.position.y})
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

