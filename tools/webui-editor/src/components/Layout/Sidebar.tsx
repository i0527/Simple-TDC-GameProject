import { EditorMode } from '../../types/editor'

interface SidebarProps {
  currentMode: EditorMode
  onModeChange: (mode: EditorMode) => void
}

export default function Sidebar({ currentMode, onModeChange }: SidebarProps) {
  const menuItems: { mode: EditorMode; label: string; icon: string }[] = [
    { mode: 'entity', label: 'エンティティエディター', icon: '👤' },
    { mode: 'stage', label: 'ステージエディター', icon: '🎮' },
    { mode: 'ui', label: 'UIエディター', icon: '🎨' },
  ]

  const advancedItems: { mode: EditorMode; label: string; icon: string }[] = [
    { mode: 'skill', label: 'スキルエディター', icon: '⚔️' },
    { mode: 'effect', label: 'エフェクトエディター', icon: '✨' },
    { mode: 'sound', label: 'サウンドエディター', icon: '🔊' },
  ]

  return (
    <aside className="w-64 border-r border-gray-200 bg-white dark:border-gray-700 dark:bg-gray-800 overflow-y-auto flex flex-col">
      <nav className="p-4 flex-1">
        <div className="mb-6">
          <h3 className="mb-2 text-xs font-semibold uppercase text-gray-600 dark:text-gray-400">
            エディター
          </h3>
          <ul className="space-y-2">
            {menuItems.map((item) => (
              <li key={item.mode}>
                <button
                  onClick={() => onModeChange(item.mode)}
                  className={`w-full rounded-lg px-4 py-3 text-left transition-colors ${
                    currentMode === item.mode
                      ? 'bg-blue-100 text-blue-900 dark:bg-blue-900 dark:text-blue-100'
                      : 'text-gray-700 hover:bg-gray-100 dark:text-gray-300 dark:hover:bg-gray-700'
                  }`}
                >
                  <span className="mr-2">{item.icon}</span>
                  {item.label}
                </button>
              </li>
            ))}
          </ul>
        </div>

        <div>
          <h3 className="mb-2 text-xs font-semibold uppercase text-gray-600 dark:text-gray-400">
            高度なエディター
          </h3>
          <ul className="space-y-2">
            {advancedItems.map((item) => (
              <li key={item.mode}>
                <button
                  onClick={() => onModeChange(item.mode)}
                  className={`w-full rounded-lg px-4 py-3 text-left transition-colors ${
                    currentMode === item.mode
                      ? 'bg-blue-100 text-blue-900 dark:bg-blue-900 dark:text-blue-100'
                      : 'text-gray-700 hover:bg-gray-100 dark:text-gray-300 dark:hover:bg-gray-700'
                  }`}
                >
                  <span className="mr-2">{item.icon}</span>
                  {item.label}
                </button>
              </li>
            ))}
          </ul>
        </div>
      </nav>
    </aside>
  )
}

