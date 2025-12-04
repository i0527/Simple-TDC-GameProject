export interface UILayoutDef {
  id: string
  name: string
  elements: Array<{
    id: string
    type: 'panel' | 'button' | 'text' | 'progressBar' | 'image'
    position: { x: number; y: number }
    size?: { width: number; height: number }
    properties: Record<string, any>
    children?: UILayoutDef['elements']
  }>
}

