import { create } from 'zustand'
import { CharacterDef } from '../types/character'
import { StageDef } from '../types/stage'
import { UILayoutDef } from '../types/ui'

interface EditorStore {
  selectedCharacter: CharacterDef | null
  selectedStage: StageDef | null
  selectedLayout: UILayoutDef | null
  isDirty: boolean
  
  setSelectedCharacter: (character: CharacterDef | null) => void
  setSelectedStage: (stage: StageDef | null) => void
  setSelectedLayout: (layout: UILayoutDef | null) => void
  setDirty: (dirty: boolean) => void
}

export const useEditorStore = create<EditorStore>((set) => ({
  selectedCharacter: null,
  selectedStage: null,
  selectedLayout: null,
  isDirty: false,
  
  setSelectedCharacter: (character) => set({ selectedCharacter: character }),
  setSelectedStage: (stage) => set({ selectedStage: stage }),
  setSelectedLayout: (layout) => set({ selectedLayout: layout }),
  setDirty: (dirty) => set({ isDirty: dirty }),
}))

