import { CharacterDef } from '../../../types/character'

interface CharacterListProps {
  characters: CharacterDef[]
  selectedCharacter: CharacterDef | null
  onSelect: (character: CharacterDef) => void
  onCreate: () => void
  onDelete: (id: string) => void
}

export default function CharacterList({
  characters,
  selectedCharacter,
  onSelect,
  onCreate,
  onDelete,
}: CharacterListProps) {
  return (
    <div className="flex h-full flex-col">
      <div className="border-b border-gray-200 p-4 dark:border-gray-700">
        <button
          onClick={onCreate}
          className="w-full rounded-lg bg-blue-600 px-4 py-2 text-white hover:bg-blue-700 dark:bg-blue-500 dark:hover:bg-blue-600"
        >
          + 新規作成
        </button>
      </div>
      <div className="flex-1 overflow-auto">
        <ul className="divide-y divide-gray-200 dark:divide-gray-700">
          {characters.map((character) => (
            <li key={character.id}>
              <div
                className={`cursor-pointer p-4 transition-colors ${
                  selectedCharacter?.id === character.id
                    ? 'bg-blue-50 dark:bg-blue-900/20'
                    : 'hover:bg-gray-50 dark:hover:bg-gray-700'
                }`}
                onClick={() => onSelect(character)}
              >
                <div className="flex items-center justify-between">
                  <div>
                    <h3 className="font-semibold text-gray-900 dark:text-white">
                      {character.name}
                    </h3>
                    <p className="text-sm text-gray-600 dark:text-gray-400">
                      {character.id}
                    </p>
                  </div>
                  <button
                    onClick={(e) => {
                      e.stopPropagation()
                      if (confirm(`「${character.name}」を削除しますか？`)) {
                        onDelete(character.id)
                      }
                    }}
                    className="rounded px-2 py-1 text-red-600 hover:bg-red-100 dark:text-red-400 dark:hover:bg-red-900/20"
                  >
                    削除
                  </button>
                </div>
              </div>
            </li>
          ))}
        </ul>
      </div>
    </div>
  )
}

