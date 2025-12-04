import axios, { AxiosInstance } from 'axios'
import { CharacterDef } from '../types/character'
import { StageDef } from '../types/stage'
import { UILayoutDef } from '../types/ui'

export class ApiClient {
  private client: AxiosInstance

  constructor(baseURL: string = 'http://localhost:8080') {
    this.client = axios.create({
      baseURL,
      headers: {
        'Content-Type': 'application/json',
      },
    })
  }

  // Health check
  async healthCheck(): Promise<{ status: string }> {
    const response = await this.client.get('/health')
    return response.data
  }

  // Characters API
  async getCharacters(): Promise<CharacterDef[]> {
    const response = await this.client.get('/api/characters')
    return response.data
  }

  async getCharacter(id: string): Promise<CharacterDef> {
    const response = await this.client.get(`/api/characters/${id}`)
    return response.data
  }

  async createCharacter(character: CharacterDef): Promise<CharacterDef> {
    const response = await this.client.post('/api/characters', character)
    return response.data
  }

  async updateCharacter(id: string, character: CharacterDef): Promise<CharacterDef> {
    const response = await this.client.put(`/api/characters/${id}`, character)
    return response.data
  }

  async deleteCharacter(id: string): Promise<void> {
    await this.client.delete(`/api/characters/${id}`)
  }

  // Stages API
  async getStages(): Promise<StageDef[]> {
    const response = await this.client.get('/api/stages')
    return response.data
  }

  async getStage(id: string): Promise<StageDef> {
    const response = await this.client.get(`/api/stages/${id}`)
    return response.data
  }

  async createStage(stage: StageDef): Promise<StageDef> {
    const response = await this.client.post('/api/stages', stage)
    return response.data
  }

  async updateStage(id: string, stage: StageDef): Promise<StageDef> {
    const response = await this.client.put(`/api/stages/${id}`, stage)
    return response.data
  }

  async deleteStage(id: string): Promise<void> {
    await this.client.delete(`/api/stages/${id}`)
  }

  // UI Layouts API
  async getUILayouts(): Promise<UILayoutDef[]> {
    const response = await this.client.get('/api/ui')
    return response.data
  }

  async getUILayout(id: string): Promise<UILayoutDef> {
    const response = await this.client.get(`/api/ui/${id}`)
    return response.data
  }

  async createUILayout(layout: UILayoutDef): Promise<UILayoutDef> {
    const response = await this.client.post('/api/ui', layout)
    return response.data
  }

  async updateUILayout(id: string, layout: UILayoutDef): Promise<UILayoutDef> {
    const response = await this.client.put(`/api/ui/${id}`, layout)
    return response.data
  }

  async deleteUILayout(id: string): Promise<void> {
    await this.client.delete(`/api/ui/${id}`)
  }

  // Game State API
  async getGameState(): Promise<any> {
    const response = await this.client.get('/api/game/state')
    return response.data
  }
}

