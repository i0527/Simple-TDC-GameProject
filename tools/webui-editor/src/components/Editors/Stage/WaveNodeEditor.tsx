import { useState } from 'react'
import {
  ReactFlow,
  Node,
  Edge,
  addEdge,
  Connection,
  useNodesState,
  useEdgesState,
  Background,
  Controls,
} from '@xyflow/react'
import '@xyflow/react/dist/style.css'
import { StageDef } from '../../../types/stage'

interface WaveNodeEditorProps {
  stage: StageDef
  onSave: (stage: StageDef) => void
}

const initialNodes: Node[] = [
  {
    id: 'stage',
    position: { x: 250, y: 0 },
    data: { label: 'Stage' },
    type: 'input',
  },
]

const initialEdges: Edge[] = []

export default function WaveNodeEditor({
  stage,
  onSave,
}: WaveNodeEditorProps) {
  const [nodes, setNodes, onNodesChange] = useNodesState(initialNodes)
  const [edges, setEdges, onEdgesChange] = useEdgesState(initialEdges)

  const onConnect = (connection: Connection) => {
    setEdges((eds) => addEdge(connection, eds))
  }

  const handleAddWave = () => {
    const newWaveNode: Node = {
      id: `wave-${nodes.length}`,
      position: { x: 250, y: nodes.length * 100 },
      data: { label: `Wave ${stage.waves.length + 1}` },
      type: 'default',
    }
    setNodes((nds) => nds.concat(newWaveNode))
  }

  return (
    <div className="flex h-full flex-col">
      <div className="border-b border-gray-200 p-4 dark:border-gray-700">
        <button
          onClick={handleAddWave}
          className="rounded-lg bg-green-600 px-4 py-2 text-white hover:bg-green-700 dark:bg-green-500 dark:hover:bg-green-600"
        >
          + Wave追加
        </button>
      </div>
      <div className="flex-1">
        <ReactFlow
          nodes={nodes}
          edges={edges}
          onNodesChange={onNodesChange}
          onEdgesChange={onEdgesChange}
          onConnect={onConnect}
        >
          <Background />
          <Controls />
        </ReactFlow>
      </div>
    </div>
  )
}

