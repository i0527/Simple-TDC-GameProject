#!/usr/bin/env python3
"""
キャラクターのPNGファイルからJSONファイルを自動生成するスクリプト
"""
import json
import os
from pathlib import Path
from PIL import Image

def get_image_size(image_path):
    """PNGファイルのサイズを取得"""
    try:
        img = Image.open(image_path)
        return img.width, img.height
    except Exception as e:
        print(f"Error loading image {image_path}: {e}")
        return None, None

def generate_json_from_png(png_path, json_path, action_name):
    """PNGファイルからJSONファイルを生成"""
    width, height = get_image_size(png_path)
    if width is None or height is None:
        return False
    
    if height <= 0:
        print(f"Invalid image height: {png_path}")
        return False
    
    # フレーム数を計算（横サイズを縦サイズで割る）
    frame_count = width // height
    if frame_count <= 0:
        print(f"Invalid frame count: {frame_count} for {png_path}")
        return False
    
    # JSONファイルを生成
    frames = {}
    for i in range(frame_count):
        frame_name = f"{action_name}-{i}.aseprite"
        frames[frame_name] = {
            "frame": {
                "x": i * height,
                "y": 0,
                "w": height,
                "h": height
            },
            "rotated": False,
            "trimmed": False,
            "spriteSourceSize": {
                "x": 0,
                "y": 0,
                "w": height,
                "h": height
            },
            "sourceSize": {
                "w": height,
                "h": height
            },
            "duration": 100
        }
    
    json_data = {
        "frames": frames,
        "meta": {
            "app": "https://www.aseprite.org/",
            "version": "1.3.x",
            "image": os.path.basename(png_path),
            "format": "RGBA8888",
            "size": {
                "w": width,
                "h": height
            },
            "scale": "1",
            "frameTags": [
                {
                    "name": action_name,
                    "from": 0,
                    "to": frame_count - 1,
                    "direction": "forward"
                }
            ]
        }
    }
    
    # JSONファイルを書き込み
    try:
        with open(json_path, 'w', encoding='utf-8') as f:
            json.dump(json_data, f, indent=1, ensure_ascii=False)
        print(f"Generated: {json_path} (frames: {frame_count})")
        return True
    except Exception as e:
        print(f"Error writing JSON {json_path}: {e}")
        return False

def generate_character_json_files(character_folder):
    """キャラクターフォルダからJSONファイルを生成"""
    print(f"Processing: {character_folder}")
    
    actions = [
        ("idle", "idle.png", "idle.json"),
        ("walk", "walk.png", "walk.json"),
        ("attack", "attack.png", "attack.json"),
        ("death", "die.png", "die.json")
    ]
    
    for action_name, png_file, json_file in actions:
        png_path = os.path.join(character_folder, png_file)
        if not os.path.exists(png_path):
            print(f"  Skipping {png_file} (not found)")
            continue
        
        json_path = os.path.join(character_folder, json_file)
        if not generate_json_from_png(png_path, json_path, action_name):
            print(f"  Failed to generate JSON for {png_file}")

def main():
    base_path = "assets/characters"
    
    # subフォルダのキャラクターを処理
    characters = [
        "sub/HatSlime",
        "sub/LanterfishAnglerfish",
        "sub/LongTailedTit",
        "sub/Orca",
        "sub/Rainbow",
        "sub/SeaHorse",
        "sub/Whale",
        "sub/YodarehakiDragonfish"
    ]
    
    for character in characters:
        character_path = os.path.join(base_path, character)
        if os.path.exists(character_path) and os.path.isdir(character_path):
            generate_character_json_files(character_path)
        else:
            print(f"Character folder not found: {character_path}")

if __name__ == "__main__":
    main()

