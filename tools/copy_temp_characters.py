#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Copy temp character assets to assets/characters directory"""

import os
import shutil
from pathlib import Path

base_dir = Path(r"Z:\kaken")
temp_dir = base_dir / "data" / "temp"
assets_dir = base_dir / "data" / "assets" / "characters"

# Character mapping: temp_name -> (folder_name, display_name, char_id)
characters = {
    "bat": ("Bat", "コウモリ", "char_sub_bat_001"),
    "blueSlime": ("BlueSlime", "ブルースライム", "char_sub_blueslime_001"),
    "GreenBossSlime": ("GreenBossSlime", "グリーンボススライム", "char_sub_greenbossslime_001"),
    "CrystalBoss": ("CrystalGolem", "クリスタルゴーレム", "char_sub_crystalgolem_001"),
    "sea_serpentBoss": ("SeaSerpent", "シーサーペント", "char_sub_seaserpent_001"),
    "Dkurage": ("PoisonJellyfish", "毒クラゲ", "char_sub_poisonjellyfish_001"),
    "kimokimo": ("KimoIsogin", "キモキモイソギン", "char_sub_kimoisogin_001"),
    "mush": ("MushMeramera", "マッシュメラメラ", "char_sub_mushmeramera_001"),
}

for temp_name, (folder_name, display_name, char_id) in characters.items():
    src_dir = temp_dir / temp_name
    dst_dir = assets_dir / folder_name
    
    # Create destination directory
    dst_dir.mkdir(parents=True, exist_ok=True)
    
    # Copy files
    files_to_copy = {
        "icon.png": "icon.png",
        "move.png": "move.png",
        "attack.png": "attack.png",
    }
    
    for src_file, dst_file in files_to_copy.items():
        src_path = src_dir / src_file
        dst_path = dst_dir / dst_file
        
        if src_path.exists():
            shutil.copy2(src_path, dst_path)
            print(f"Copied {src_path} -> {dst_path}")
        else:
            print(f"Warning: {src_path} not found")

print("Done copying character assets!")
