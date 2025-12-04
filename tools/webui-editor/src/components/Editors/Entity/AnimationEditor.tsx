import { CharacterDef } from '../../../types/character'

interface AnimationEditorProps {
  character: CharacterDef
  onChange: (character: CharacterDef) => void
}

export default function AnimationEditor({
  character,
  onChange,
}: AnimationEditorProps) {
  const handleAddAnimation = () => {
    const newAnimation = {
      name: `animation_${character.animations.length}`,
      frames: [0, 1, 2],
      duration: 0.1,
      loop: true,
    }
    onChange({
      ...character,
      animations: [...character.animations, newAnimation],
    })
  }

  const handleRemoveAnimation = (index: number) => {
    onChange({
      ...character,
      animations: character.animations.filter((_, i) => i !== index),
    })
  }

  const handleUpdateAnimation = (index: number, key: string, value: any) => {
    const updatedAnimations = [...character.animations]
    updatedAnimations[index] = { ...updatedAnimations[index], [key]: value }
    onChange({
      ...character,
      animations: updatedAnimations,
    })
  }

  return (
    <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
      <div className="mb-4 flex items-center justify-between">
        <h3 className="text-lg font-semibold text-gray-900 dark:text-white">
          アニメーション
        </h3>
        <button
          onClick={handleAddAnimation}
          className="rounded-lg bg-green-600 px-3 py-1 text-sm text-white hover:bg-green-700 dark:bg-green-500 dark:hover:bg-green-600"
        >
          + アニメーション追加
        </button>
      </div>

      <div className="space-y-4">
        {character.animations.map((anim, index) => (
          <div
            key={index}
            className="rounded-lg border border-gray-300 p-4 dark:border-gray-600"
          >
            <div className="mb-3 flex items-center justify-between">
              <input
                type="text"
                value={anim.name}
                onChange={(e) => handleUpdateAnimation(index, 'name', e.target.value)}
                className="rounded-lg border border-gray-300 px-3 py-1 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
              />
              <button
                onClick={() => handleRemoveAnimation(index)}
                className="rounded px-2 py-1 text-red-600 hover:bg-red-100 dark:text-red-400 dark:hover:bg-red-900/20"
              >
                削除
              </button>
            </div>

            <div className="grid grid-cols-2 gap-3">
              <div>
                <label className="block text-sm text-gray-700 dark:text-gray-300">
                  フレーム
                </label>
                <input
                  type="text"
                  value={anim.frames.join(',')}
                  onChange={(e) =>
                    handleUpdateAnimation(
                      index,
                      'frames',
                      e.target.value.split(',').map((f) => parseInt(f) || 0)
                    )
                  }
                  placeholder="0,1,2,3"
                  className="mt-1 w-full rounded-lg border border-gray-300 px-2 py-1 text-sm dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
              <div>
                <label className="block text-sm text-gray-700 dark:text-gray-300">
                  フレーム時間(秒)
                </label>
                <input
                  type="number"
                  step="0.01"
                  value={anim.duration}
                  onChange={(e) =>
                    handleUpdateAnimation(index, 'duration', parseFloat(e.target.value) || 0)
                  }
                  className="mt-1 w-full rounded-lg border border-gray-300 px-2 py-1 text-sm dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                />
              </div>
            </div>

            <div className="mt-3">
              <label className="flex items-center gap-2">
                <input
                  type="checkbox"
                  checked={anim.loop}
                  onChange={(e) => handleUpdateAnimation(index, 'loop', e.target.checked)}
                  className="rounded"
                />
                <span className="text-sm text-gray-700 dark:text-gray-300">ループ再生</span>
              </label>
            </div>
          </div>
        ))}
      </div>
    </section>
  )
}

