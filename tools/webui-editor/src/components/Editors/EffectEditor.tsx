import { useState } from 'react'
import { ParticleEffectDef, ScreenEffectDef } from '../../../types/effect'

interface EffectEditorProps {
  effect?: ParticleEffectDef | ScreenEffectDef
  onSave?: (effect: any) => void
  onCancel?: () => void
}

export default function EffectEditor({
  effect,
  onSave,
  onCancel,
}: EffectEditorProps) {
  const [activeTab, setActiveTab] = useState<'particle' | 'screen' | 'composite'>('particle')

  return (
    <div className="h-full overflow-auto p-8">
      <div className="mx-auto max-w-6xl">
        <div className="mb-6 flex items-center justify-between">
          <h2 className="text-2xl font-bold text-gray-900 dark:text-white">
            エフェクトエディター
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
          {['particle', 'screen', 'composite'].map((tab) => (
            <button
              key={tab}
              onClick={() => setActiveTab(tab as any)}
              className={`px-4 py-2 font-medium transition-colors ${
                activeTab === tab
                  ? 'border-b-2 border-blue-600 text-blue-600 dark:border-blue-400 dark:text-blue-400'
                  : 'text-gray-600 hover:text-gray-900 dark:text-gray-400 dark:hover:text-gray-200'
              }`}
            >
              {tab === 'particle' && 'パーティクルエフェクト'}
              {tab === 'screen' && 'スクリーンエフェクト'}
              {tab === 'composite' && '複合エフェクト'}
            </button>
          ))}
        </div>

        {/* パーティクルエフェクト */}
        {activeTab === 'particle' && (
          <div className="space-y-6">
            <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
              <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
                パーティクルエフェクト作成
              </h3>
              <p className="text-sm text-gray-600 dark:text-gray-400 mb-4">
                エミッター形状、発生モード、アニメーションを設定します。
              </p>

              <div className="grid grid-cols-2 gap-4">
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    エミッター形状
                  </label>
                  <select className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white">
                    <option>Point（点）</option>
                    <option>Circle（円）</option>
                    <option>Rectangle（矩形）</option>
                    <option>Line（線）</option>
                    <option>Ring（リング）</option>
                    <option>Cone（コーン）</option>
                  </select>
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    発生モード
                  </label>
                  <select className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white">
                    <option>Continuous（連続）</option>
                    <option>Burst（バースト）</option>
                    <option>Distance（距離ベース）</option>
                  </select>
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    ブレンドモード
                  </label>
                  <select className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white">
                    <option>Alpha</option>
                    <option>Additive</option>
                    <option>Multiply</option>
                    <option>Screen</option>
                  </select>
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    最大パーティクル数
                  </label>
                  <input
                    type="number"
                    defaultValue="100"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>
              </div>

              <div className="mt-6 rounded-lg border border-dashed border-gray-300 bg-gray-50 p-8 text-center dark:border-gray-600 dark:bg-gray-900">
                <p className="text-sm text-gray-600 dark:text-gray-400">
                  プレビューエリア（Canvasベースプレビューは後续実装）
                </p>
              </div>
            </section>
          </div>
        )}

        {/* スクリーンエフェクト */}
        {activeTab === 'screen' && (
          <div className="space-y-6">
            <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
              <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
                スクリーンエフェクト作成
              </h3>
              <p className="text-sm text-gray-600 dark:text-gray-400 mb-4">
                画面全体に影響するエフェクトを設定します。
              </p>

              <div className="grid grid-cols-2 gap-4">
                <div className="col-span-2">
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    エフェクトタイプ
                  </label>
                  <select className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white">
                    <option>Shake（揺れ）</option>
                    <option>Flash（フラッシュ）</option>
                    <option>FadeIn/Out（フェード）</option>
                    <option>Zoom（ズーム）</option>
                    <option>Blur（ブラー）</option>
                    <option>SlowMotion（スローモーション）</option>
                  </select>
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    継続時間
                  </label>
                  <input
                    type="number"
                    step="0.1"
                    defaultValue="0.5"
                    className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-gray-700 dark:text-gray-300">
                    イージング
                  </label>
                  <select className="mt-1 w-full rounded-lg border border-gray-300 px-3 py-2 dark:border-gray-600 dark:bg-gray-700 dark:text-white">
                    <option>Linear</option>
                    <option>EaseIn</option>
                    <option>EaseOut</option>
                    <option>EaseInOut</option>
                  </select>
                </div>
              </div>
            </section>
          </div>
        )}

        {/* 複合エフェクト */}
        {activeTab === 'composite' && (
          <div className="space-y-6">
            <section className="rounded-lg border border-gray-200 bg-white p-6 dark:border-gray-700 dark:bg-gray-800">
              <h3 className="mb-4 text-lg font-semibold text-gray-900 dark:text-white">
                複合エフェクトタイムライン
              </h3>
              <p className="text-sm text-gray-600 dark:text-gray-400 mb-4">
                パーティクル、スクリーンエフェクト、サウンドを時間軸上で組み合わせます。
              </p>

              <div className="rounded-lg border border-dashed border-gray-300 bg-gray-50 p-8 dark:border-gray-600 dark:bg-gray-900">
                <p className="text-center text-sm text-gray-600 dark:text-gray-400">
                  タイムラインエディター（後续実装予定）
                </p>
              </div>
            </section>
          </div>
        )}
      </div>
    </div>
  )
}

