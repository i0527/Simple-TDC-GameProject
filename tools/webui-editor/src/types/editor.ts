export type EditorMode = 'entity' | 'stage' | 'ui'

export interface EditorState {
  mode: EditorMode
  isLoading: boolean
  error: string | null
}

