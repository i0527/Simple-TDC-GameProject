export type EditorMode = 'entity' | 'stage' | 'ui' | 'skill' | 'effect' | 'sound'

export interface EditorState {
  mode: EditorMode
  isLoading: boolean
  error: string | null
}

