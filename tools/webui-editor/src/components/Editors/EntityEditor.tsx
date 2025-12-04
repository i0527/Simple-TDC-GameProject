import { useState, useEffect } from 'react'
import { useApiClient } from '../../hooks/useApiClient'
import CharacterList from './Entity/CharacterList'
import CharacterEditor from './Entity/CharacterEditor'
import { CharacterDef } from '../../types/character'

export default function EntityEditor() {
  const { apiClient } = useApiClient()
  const [characters, setCharacters] = useState<CharacterDef[]>([])
  const [selectedCharacter, setSelectedCharacter] = useState<CharacterDef | null>(null)
  const [isLoading, setIsLoading] = useState(true)
  const [error, setError] = useState<string | null>(null)

  useEffect(() => {
    loadCharacters()
  }, [])

  const loadCharacters = async () => {
    try {
      setIsLoading(true)
      setError(null)
      const data = await apiClient.getCharacters()
      setCharacters(data)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'キャラクターの読み込みに失敗しました')
    } finally {
      setIsLoading(false)
    }
  }

  const handleCharacterSelect = (character: CharacterDef) => {
    setSelectedCharacter(character)
  }

  const handleCharacterCreate = () => {
    const newCharacter: CharacterDef = {
      id: `character_${Date.now()}`,
      name: '新しいキャラクター',
      description: '',
      rarity: 'common',
      gameMode: 'td',
      visual: {
        spriteSheet: '',
        spriteSize: { width: 64, height: 64 },
        scale: 1.0,
      },
      stats: {
        maxHealth: 100,
        currentHealth: 100,
        attack: 10,
        defense: 5,
        speed: 1.0,
      },
      combat: {
        attackRange: 100,
        attackCooldown: 1.0,
        attackDamage: 10,
      },
      animations: [],
    }
    setSelectedCharacter(newCharacter)
  }

  const handleCharacterSave = async (character: CharacterDef) => {
    try {
      if (character.id && characters.some((c) => c.id === character.id)) {
        await apiClient.updateCharacter(character.id, character)
      } else {
        await apiClient.createCharacter(character)
      }
      await loadCharacters()
      setSelectedCharacter(null)
    } catch (err) {
      setError(err instanceof Error ? err.message : 'キャラクターの保存に失敗しました')
    }
  }

  const handleCharacterDelete = async (id: string) => {
    try {
      await apiClient.deleteCharacter(id)
      await loadCharacters()
      if (selectedCharacter?.id === id) {
        setSelectedCharacter(null)
      }
    } catch (err) {
      setError(err instanceof Error ? err.message : 'キャラクターの削除に失敗しました')
    }
  }

  if (isLoading) {
    return (
      <div className="flex h-full items-center justify-center">
        <div className="text-gray-600 dark:text-gray-400">読み込み中...</div>
      </div>
    )
  }

  if (error) {
    return (
      <div className="p-8">
        <div className="rounded-lg bg-red-100 p-4 text-red-800 dark:bg-red-900 dark:text-red-200">
          {error}
        </div>
      </div>
    )
  }

  return (
    <div className="flex h-full">
      <div className="w-80 border-r border-gray-200 dark:border-gray-700">
        <CharacterList
          characters={characters}
          selectedCharacter={selectedCharacter}
          onSelect={handleCharacterSelect}
          onCreate={handleCharacterCreate}
          onDelete={handleCharacterDelete}
        />
      </div>
      <div className="flex-1 overflow-auto">
        {selectedCharacter ? (
          <CharacterEditor
            character={selectedCharacter}
            onSave={handleCharacterSave}
            onCancel={() => setSelectedCharacter(null)}
          />
        ) : (
          <div className="flex h-full items-center justify-center text-gray-500 dark:text-gray-400">
            キャラクターを選択するか、新規作成してください
          </div>
        )}
      </div>
    </div>
  )
}

