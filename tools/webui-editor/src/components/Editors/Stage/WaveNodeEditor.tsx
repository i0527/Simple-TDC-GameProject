import { useCallback, useState } from 'react'
import {
  ReactFlow,
  Background,
  Controls,
  MiniMap,
  addEdge,
  Connection,
  useNodesState,
  useEdgesState,
  Node,
  Edge,
} from '@xyflow/react'
import '@xyflow/react/dist/style.css'
import { StageDef, WaveDef } from '../../../types/stage'

interface WaveNodeData {
  label: string
  waveId: string
  enemyCount: number
}

interface StageNodeData {
  label: string
  stageId: string
}

interface NodeEditorProps {
  stage: StageDef
  onSave: (stage: StageDef) => void
}

export default function WaveNodeEditor({ stage, onSave }: NodeEditorProps) {
  const [nodes, setNodes, onNodesChange] = useNodesState<Node<WaveNodeData | StageNodeData>[]>([])
  const [edges, setEdges, onEdgesChange] = useEdgesState<Edge[]>([])

  // ノード初期化
  const onInit = useCallback(() => {
    const initialNodes: Node[] = [
      {
        id: `stage_${stage.id}`,
        data: { label: stage.name, stageId: stage.id } as StageNodeData,
        position: { x: 250, y: 50 },
        type: 'default',
      },
      ...stage.waves.map((wave, index) => ({
        id: `wave_${wave.id}`,
        data: {
          label: `Wave ${index + 1}`,
          waveId: wave.id,
          enemyCount: wave.enemies.length,
        } as WaveNodeData,
        position: { x: 250 + index * 150, y: 150 },
        type: 'default',
      })),
    ]

    const initialEdges: Edge[] = stage.waves.map((wave, index) => ({
      id: `edge_${index}`,
      source: index === 0 ? `stage_${stage.id}` : `wave_${stage.waves[index - 1].id}`,
      target: `wave_${wave.id}`,
    }))

    setNodes(initialNodes)
    setEdges(initialEdges)
  }, [stage, setNodes, setEdges])

  const onConnect = useCallback(
    (connection: Connection) => {
      setEdges((eds) => addEdge(connection, eds))
    },
    [setEdges]
  )

  return (
    <div className="h-full w-full">
      <ReactFlow
        nodes={nodes}
        edges={edges}
        onNodesChange={onNodesChange}
        onEdgesChange={onEdgesChange}
        onConnect={onConnect}
        onInit={onInit}
      >
        <Background />
        <Controls />
        <MiniMap />
      </ReactFlow>
    </div>
  )
}

