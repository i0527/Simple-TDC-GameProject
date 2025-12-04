import { CharacterDef } from '../../../types/character'

interface CharacterPreviewProps {
  character: CharacterDef
}

export default function CharacterPreview({
  character,
}: CharacterPreviewProps) {
  return (
    <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
      <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
        プレビュー
      </h3>

      {/* スプライトプレビュー（仮） */}
      <div className="mb-4 flex items-center justify-center rounded-lg bg-gray-100 p-8 dark:bg-gray-900">
        {character.visual.spriteSheet ? (
          <img
            src={character.visual.spriteSheet}
            alt={character.name}
            style={{
              width: character.visual.spriteSize.width * character.visual.scale,
              height: character.visual.spriteSize.height * character.visual.scale,
            }}
            className="rounded-lg"
          />
        ) : (
          <div className="text-center text-gray-500 dark:text-gray-400">
            <p>スプライトシート: {character.visual.spriteSheet || 'なし'}</p>
            <p className="mt-2 text-sm">
              {character.visual.spriteSize.width}x{character.visual.spriteSize.height}
            </p>
          </div>
        )}
      </div>

      {/* ステータス表示 */}
      <div className="grid grid-cols-2 gap-2 text-sm">
        <div>
          <span className="text-gray-600 dark:text-gray-400">HP:</span>
          <span className="ml-2 font-semibold text-gray-900 dark:text-white">
            {character.stats.maxHealth}
          </span>
        </div>
        <div>
          <span className="text-gray-600 dark:text-gray-400">攻撃:</span>
          <span className="ml-2 font-semibold text-gray-900 dark:text-white">
            {character.stats.attack}
          </span>
        </div>
        <div>
          <span className="text-gray-600 dark:text-gray-400">防御:</span>
          <span className="ml-2 font-semibold text-gray-900 dark:text-white">
            {character.stats.defense}
          </span>
        </div>
        <div>
          <span className="text-gray-600 dark:text-gray-400">速度:</span>
          <span className="ml-2 font-semibold text-gray-900 dark:text-white">
            {character.stats.speed.toFixed(1)}
          </span>
        </div>
        <div>
          <span className="text-gray-600 dark:text-gray-400">射程:</span>
          <span className="ml-2 font-semibold text-gray-900 dark:text-white">
            {character.combat.attackRange}
          </span>
        </div>
        <div>
          <span className="text-gray-600 dark:text-gray-400">攻撃クール:</span>
          <span className="ml-2 font-semibold text-gray-900 dark:text-white">
            {character.combat.attackCooldown.toFixed(1)}s
          </span>
        </div>
      </div>

      {/* アニメーション一覧 */}
      <div className="mt-4">
        <p className="text-sm font-semibold text-gray-900 dark:text-white">
          アニメーション ({character.animations.length})
        </p>
        <div className="mt-2 space-y-1">
          {character.animations.map((anim, i) => (
            <div key={i} className="text-xs text-gray-600 dark:text-gray-400">
              • {anim.name} ({anim.frames.length} frames, {(anim.duration * 1000).toFixed(0)}ms)
            </div>
          ))}
        </div>
      </div>
    </section>
  )
}

