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

  return (
    <aside className="w-64 border-r border-gray-200 bg-white dark:border-gray-700 dark:bg-gray-800">
      <nav className="p-4">
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
      </nav>
    </aside>
  )
}

