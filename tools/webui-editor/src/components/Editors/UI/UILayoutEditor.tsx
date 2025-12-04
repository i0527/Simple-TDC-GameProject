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
  const [selectedElement, setSelectedElement] = useState<string | null>(null)
  const [previewScale, setPreviewScale] = useState(0.25)

  const handleAddElement = (type: string) => {
    const newElement = {
      id: `${type}_${Date.now()}`,
      type: type as any,
      position: { x: 100, y: 100 },
      size: { width: 100, height: 50 },
      properties: {
        text: 'New ' + type,
        color: '#FFFFFF',
      },
    }
    setEditedLayout({
      ...editedLayout,
      elements: [...editedLayout.elements, newElement],
    })
  }

  const handleUpdateElement = (elementId: string, key: string, value: any) => {
    const updatedElements = editedLayout.elements.map((el) =>
      el.id === elementId
        ? { ...el, [key]: value }
        : el
    )
    setEditedLayout({
      ...editedLayout,
      elements: updatedElements,
    })
  }

  const handleDeleteElement = (elementId: string) => {
    setEditedLayout({
      ...editedLayout,
      elements: editedLayout.elements.filter((el) => el.id !== elementId),
    })
  }

  const selectedElem = editedLayout.elements.find((el) => el.id === selectedElement)

  return (
    <div className="grid h-full grid-cols-4 gap-4 p-8">
      {/* 左: ツールパレット */}
      <div className="col-span-1 space-y-4 overflow-auto">
        <section className="rounded-lg border border-gray-200 bg-white p-4 dark:border-gray-700 dark:bg-gray-800">
          <h3 className="mb-3 font-semibold text-gray-900 dark:text-white">
            UI要素
          </h3>
          <div className="grid gap-2">
            {['panel', 'button', 'text', 'progressBar', 'image'].map((type) => (
              <button
                key={type}
                onClick={() => handleAddElement(type)}
                className="rounded-lg bg-blue-600 px-3 py-2 text-sm text-white hover:bg-blue-700 dark:bg-blue-500 dark:hover:bg-blue-600"
              >
                {type}を追加
              </button>
            ))}
          </div>
        </section>

        {/* プロパティパネル */}
        {selectedElem && (
          <section className="rounded-lg border border-gray-200 bg-white p-4 dark:border-gray-700 dark:bg-gray-800">
            <div className="mb-3 flex items-center justify-between">
              <h3 className="font-semibold text-gray-900 dark:text-white">
                プロパティ
              </h3>
              <button
                onClick={() => handleDeleteElement(selectedElem.id)}
                className="rounded px-2 py-1 text-red-600 hover:bg-red-100 dark:text-red-400 dark:hover:bg-red-900/20"
              >
                削除
              </button>
            </div>
            <div className="space-y-3 text-sm">
              <div>
                <label className="block text-gray-700 dark:text-gray-300">
                  X
                </label>
                <input
                  type="number"
                  value={selectedElem.position.x}
                  onChange={(e) =>
                    handleUpdateElement(selectedElem.id, 'position', {
                      ...selectedElem.position,
                      x: parseInt(e.target.value) || 0,
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-2 py-1 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div>
                <label className="block text-gray-700 dark:text-gray-300">
                  Y
                </label>
                <input
                  type="number"
                  value={selectedElem.position.y}
                  onChange={(e) =>
                    handleUpdateElement(selectedElem.id, 'position', {
                      ...selectedElem.position,
                      y: parseInt(e.target.value) || 0,
                    })
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-2 py-1 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              {selectedElem.size && (
                <>
                  <div>
                    <label className="block text-gray-700 dark:text-gray-300">
                      幅
                    </label>
                    <input
                      type="number"
                      value={selectedElem.size.width}
                      onChange={(e) =>
                        handleUpdateElement(selectedElem.id, 'size', {
                          ...selectedElem.size,
                          width: parseInt(e.target.value) || 0,
                        })
                      }
                      className="mt-1 w-full rounded-lg border border-gray-300 px-2 py-1 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                    />
                  </div>
                  <div>
                    <label className="block text-gray-700 dark:text-gray-300">
                      高さ
                    </label>
                    <input
                      type="number"
                      value={selectedElem.size.height}
                      onChange={(e) =>
                        handleUpdateElement(selectedElem.id, 'size', {
                          ...selectedElem.size,
                          height: parseInt(e.target.value) || 0,
                        })
                      }
                      className="mt-1 w-full rounded-lg border border-gray-300 px-2 py-1 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                    />
                  </div>
                </>
              )}
            </div>
          </section>
        )}

        {/* スケールコントロール */}
        <section className="rounded-lg border border-gray-200 bg-white p-4 dark:border-gray-700 dark:bg-gray-800">
          <label className="text-sm font-semibold text-gray-900 dark:text-white">
            プレビュースケール
          </label>
          <input
            type="range"
            min="0.1"
            max="1"
            step="0.05"
            value={previewScale}
            onChange={(e) => setPreviewScale(parseFloat(e.target.value))}
            className="mt-2 w-full"
          />
          <p className="mt-2 text-xs text-gray-600 dark:text-gray-400">
            {(previewScale * 100).toFixed(0)}%
          </p>
        </section>
      </div>

      {/* 中央: WYSIWYG エディター */}
      <div className="col-span-2">
        <section className="h-full rounded-lg border border-gray-200 bg-white dark:border-gray-700 dark:bg-gray-800">
          <div className="flex h-full flex-col">
            <div className="border-b border-gray-200 p-4 dark:border-gray-700">
              <h3 className="font-semibold text-gray-900 dark:text-white">
                WYSIWYG エディター
              </h3>
            </div>
            <div className="flex-1 overflow-auto p-4">
              {/* FHD (1920x1080) キャンバス */}
              <div
                className="relative border-2 border-gray-300 bg-gray-900 dark:border-gray-600"
                style={{
                  width: 1920 * previewScale,
                  height: 1080 * previewScale,
                }}
              >
                {/* UI要素のレンダリング */}
                {editedLayout.elements.map((element) => (
                  <div
                    key={element.id}
                    onClick={() => setSelectedElement(element.id)}
                    className={`absolute cursor-move rounded border-2 ${
                      selectedElement === element.id
                        ? 'border-blue-500 bg-blue-100/20'
                        : 'border-gray-500 hover:border-gray-400'
                    }`}
                    style={{
                      left: element.position.x * previewScale,
                      top: element.position.y * previewScale,
                      width: element.size?.width ? element.size.width * previewScale : 'auto',
                      height: element.size?.height ? element.size.height * previewScale : 'auto',
                      backgroundColor: element.properties?.color || 'transparent',
                    }}
                  >
                    <div className="text-xs text-white">
                      {element.properties?.text || element.type}
                    </div>
                  </div>
                ))}
              </div>
            </div>
          </div>
        </section>
      </div>

      {/* 右: 要素リスト */}
      <div className="col-span-1 space-y-4 overflow-auto">
        <section className="rounded-lg border border-gray-200 bg-white p-4 dark:border-gray-700 dark:bg-gray-800">
          <h3 className="mb-3 font-semibold text-gray-900 dark:text-white">
            要素一覧
          </h3>
          <div className="space-y-2">
            {editedLayout.elements.map((element) => (
              <div
                key={element.id}
                onClick={() => setSelectedElement(element.id)}
                className={`cursor-pointer rounded-lg p-2 text-sm ${
                  selectedElement === element.id
                    ? 'bg-blue-100 text-blue-900 dark:bg-blue-900/30 dark:text-blue-200'
                    : 'hover:bg-gray-100 dark:hover:bg-gray-700'
                }`}
              >
                <div className="font-semibold">{element.type}</div>
                <div className="text-xs text-gray-600 dark:text-gray-400">
                  {element.id}
                </div>
              </div>
            ))}
          </div>
        </section>

        {/* 保存ボタン */}
        <button
          onClick={() => onSave(editedLayout)}
          className="w-full rounded-lg bg-blue-600 px-4 py-2 text-white hover:bg-blue-700 dark:bg-blue-500 dark:hover:bg-blue-600"
        >
          保存
        </button>
      </div>
    </div>
  )
}
