import { useState } from 'react'
import { SoundDef, MusicDef } from '../../../types/sound'

interface SoundEditorProps {
  sound?: SoundDef | MusicDef
  onSave?: (sound: any) => void
  onCancel?: () => void
}

export default function SoundEditor({
  sound,
  onSave,
  onCancel,
}: SoundEditorProps) {
  const [activeTab, setActiveTab] = useState<'sfx' | 'music' | 'events'>('sfx')

  return (
    <div className="h-full overflow-auto p-8">
      <div className="mx-auto max-w-6xl">
        <div className="mb-6 flex items-center justify-between">
          <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
            サウンドエディター
          </h2>
          {onCancel && (
            <button
              onClick={onCancel}
              className="rounded-lg border border-gray-300 px-4 py-2 text-gray-700 hover:bg-gray-50 dark:border-gray-600 dark:text-gray-300 dark:hover:bg-gray-700"
            >
              閉じる
            </button>
          )}
        </div>

        {/* タブナビゲーション */}
        <div className="mb-6 flex gap-2 border-b border-gray-200 dark:border-gray-700">
          {['sfx', 'music', 'events'].map((tab) => (
            <button
              key={tab}
              onClick={() => setActiveTab(tab as any)}
              className={`px-4 py-2 font-medium transition-colors ${
                activeTab === tab
                  ? 'border-b-2 border-blue-600 text-blue-600 dark:border-blue-400 dark:text-blue-400'
                  : 'text-gray-600 hover:text-gray-900 dark:text-gray-400 dark:hover:text-gray-200'
              }`}
            >
              {tab === 'sfx' && 'SFX & ボイス'}
              {tab === 'music' && 'BGM'}
              {tab === 'events' && 'サウンドイベント'}
            </button>
          ))}
        </div>

        {/* SFX & ボイス */}
        {activeTab === 'sfx' && (
          <div className="space-y-6">
            <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
              <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
                SFX・ボイス設定
              </h3>

              <div className="grid grid-cols-2 gap-4">
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    ID
                  </label>
                  <input
                    type="text"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    名前
                  </label>
                  <input
                    type="text"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    タイプ
                  </label>
                  <select className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white">
                    <option>SFX（効果音）</option>
                    <option>Voice（ボイス）</option>
                    <option>Ambient（環境音）</option>
                    <option>UI（UI音）</option>
                  </select>
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    優先度
                  </label>
                  <select className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white">
                    <option>Low（低）</option>
                    <option>Normal（通常）</option>
                    <option>High（高）</option>
                    <option>Critical（重要）</option>
                  </select>
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    ボリューム
                  </label>
                  <input
                    type="range"
                    min="0"
                    max="1"
                    step="0.1"
                    defaultValue="1"
                    className="mt-1 w-full"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    ピッチ
                  </label>
                  <input
                    type="range"
                    min="0.5"
                    max="2"
                    step="0.1"
                    defaultValue="1"
                    className="mt-1 w-full"
                  />
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    同時再生数上限
                  </label>
                  <input
                    type="number"
                    defaultValue="4"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    再生間隔（秒）
                  </label>
                  <input
                    type="number"
                    step="0.1"
                    defaultValue="0"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>
              </div>

              {/* 3D サウンド設定 */}
              <div className="mt-6 space-y-3 border-t border-gray-200 pt-4 dark:border-gray-700">
                <label className="flex items-center gap-2">
                  <input type="checkbox" className="rounded" />
                  <span className="text-sm font-medium text-gray-700 dark:text-gray-300">
                    3D サウンド
                  </span>
                </label>
                <div className="grid grid-cols-2 gap-4 pl-6">
                  <div>
                    <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                      最小距離
                    </label>
                    <input
                      type="number"
                      defaultValue="1"
                      className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                    />
                  </div>
                  <div>
                    <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                      最大距離
                    </label>
                    <input
                      type="number"
                      defaultValue="100"
                      className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                    />
                  </div>
                </div>
              </div>

              {/* ファイルバリエーション */}
              <div className="mt-6 space-y-3 border-t border-gray-200 pt-4 dark:border-gray-700">
                <div className="flex items-center justify-between">
                  <h4 className="font-semibold text-gray-900 dark:text-white">
                    ファイルバリエーション
                  </h4>
                  <button className="rounded-lg bg-green-600 px-3 py-1 text-sm text-white hover:bg-green-700 dark:bg-green-500 dark:hover:bg-green-600">
                    + 追加
                  </button>
                </div>
                <div className="rounded-lg border border-dashed border-gray-300 bg-gray-50 p-4 text-center dark:border-gray-600 dark:bg-gray-900">
                  <p className="text-sm text-gray-600 dark:text-gray-400">
                    ファイルをドラッグ&ドロップ、または選択してください
                  </p>
                </div>
              </div>
            </section>
          </div>
        )}

        {/* BGM */}
        {activeTab === 'music' && (
          <div className="space-y-6">
            <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
              <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
                BGM設定
              </h3>

              <div className="grid grid-cols-2 gap-4">
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    ID
                  </label>
                  <input
                    type="text"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    名前
                  </label>
                  <input
                    type="text"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    BPM
                  </label>
                  <input
                    type="number"
                    defaultValue="120"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    拍子
                  </label>
                  <select className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white">
                    <option>4/4</option>
                    <option>3/4</option>
                    <option>6/8</option>
                  </select>
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    クロスフェード時間
                  </label>
                  <input
                    type="number"
                    step="0.1"
                    defaultValue="2"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>
              </div>

              {/* ループ設定 */}
              <div className="mt-6 space-y-3 border-t border-gray-200 pt-4 dark:border-gray-700">
                <label className="flex items-center gap-2">
                  <input type="checkbox" defaultChecked className="rounded" />
                  <span className="text-sm font-medium text-gray-700 dark:text-gray-300">
                    ループ有効
                  </span>
                </label>
              </div>
            </section>
          </div>
        )}

        {/* サウンドイベント */}
        {activeTab === 'events' && (
          <div className="space-y-6">
            <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
              <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
                サウンドイベント
              </h3>
              <p className="text-sm text-gray-600 dark:text-gray-400 mb-4">
                ゲームイベントに対応するサウンド群を定義します。
              </p>

              <div className="space-y-4">
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    イベントID
                  </label>
                  <input
                    type="text"
                    placeholder="例: on_boss_defeat"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    再生モード
                  </label>
                  <select className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white">
                    <option>All（全て再生）</option>
                    <option>Random（ランダム）</option>
                    <option>Sequence（順番）</option>
                  </select>
                </div>

                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    サウンド
                  </label>
                  <div className="mt-2 rounded-lg border border-dashed border-gray-300 bg-gray-50 p-4 text-center dark:border-gray-600 dark:bg-gray-900">
                    <p className="text-sm text-gray-600 dark:text-gray-400">
                      サウンドを選択してください
                    </p>
                  </div>
                </div>
              </div>
            </section>
          </div>
        )}
      </div>
    </div>
  )
}

