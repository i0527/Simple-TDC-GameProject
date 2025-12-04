import { useState } from 'react'
import Header from './components/Layout/Header'
import Sidebar from './components/Layout/Sidebar'
import EntityEditor from './components/Editors/EntityEditor'
import StageEditor from './components/Editors/StageEditor'
import UIEditor from './components/Editors/UIEditor'
import { EditorMode } from './types/editor'

function App() {
  const [currentMode, setCurrentMode] = useState<EditorMode>('entity')

  const renderEditor = () => {
    switch (currentMode) {
      case 'entity':
        return <EntityEditor />
      case 'stage':
        return <StageEditor />
      case 'ui':
        return <UIEditor />
      default:
        return <div className="p-8">エディターを選択してください</div>
    }
  }

  return (
    <div className="flex h-screen flex-col bg-gray-50 dark:bg-gray-900">
      <Header />
      <div className="flex flex-1 overflow-hidden">
        <Sidebar currentMode={currentMode} onModeChange={setCurrentMode} />
        <main className="flex-1 overflow-auto">
          {renderEditor()}
        </main>
      </div>
    </div>
  )
}

export default App

