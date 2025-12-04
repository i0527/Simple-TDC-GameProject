import { StageDef } from '../../../types/stage'

interface StageListProps {
  stages: StageDef[]
  selectedStage: StageDef | null
  onSelect: (stage: StageDef) => void
}

export default function StageList({
  stages,
  selectedStage,
  onSelect,
}: StageListProps) {
  return (
    <div className="flex h-full flex-col">
      <div className="border-b border-gray-200 p-4 dark:border-gray-700">
        <h2 className="font-semibold text-gray-900 dark:text-white">ステージ一覧</h2>
      </div>
      <div className="flex-1 overflow-auto">
        <ul className="divide-y divide-gray-200 dark:divide-gray-700">
          {stages.map((stage) => (
            <li key={stage.id}>
              <div
                className={`cursor-pointer p-4 transition-colors ${
                  selectedStage?.id === stage.id
                    ? 'bg-blue-50 dark:bg-blue-900/20'
                    : 'hover:bg-gray-50 dark:hover:bg-gray-700'
                }`}
                onClick={() => onSelect(stage)}
              >
                <h3 className="font-semibold text-gray-900 dark:text-white">
                  {stage.name}
                </h3>
                <p className="text-sm text-gray-600 dark:text-gray-400">
                  {stage.waves.length} waves
                </p>
              </div>
            </li>
          ))}
        </ul>
      </div>
    </div>
  )
}

