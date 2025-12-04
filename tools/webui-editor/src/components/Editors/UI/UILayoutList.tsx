import { UILayoutDef } from '../../../types/ui'

interface UILayoutListProps {
  layouts: UILayoutDef[]
  selectedLayout: UILayoutDef | null
  onSelect: (layout: UILayoutDef) => void
}

export default function UILayoutList({
  layouts,
  selectedLayout,
  onSelect,
}: UILayoutListProps) {
  return (
    <div className="flex h-full flex-col">
      <div className="border-b border-gray-200 p-4 dark:border-gray-700">
        <h2 className="font-semibold text-gray-900 dark:text-white">UIレイアウト一覧</h2>
      </div>
      <div className="flex-1 overflow-auto">
        <ul className="divide-y divide-gray-200 dark:divide-gray-700">
          {layouts.map((layout) => (
            <li key={layout.id}>
              <div
                className={`cursor-pointer p-4 transition-colors ${
                  selectedLayout?.id === layout.id
                    ? 'bg-blue-50 dark:bg-blue-900/20'
                    : 'hover:bg-gray-50 dark:hover:bg-gray-700'
                }`}
                onClick={() => onSelect(layout)}
              >
                <h3 className="font-semibold text-gray-900 dark:text-white">
                  {layout.name}
                </h3>
                <p className="text-sm text-gray-600 dark:text-gray-400">
                  {layout.elements.length} elements
                </p>
              </div>
            </li>
          ))}
        </ul>
      </div>
    </div>
  )
}

