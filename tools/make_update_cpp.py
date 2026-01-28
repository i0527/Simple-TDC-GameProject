# -*- coding: utf-8 -*-
"""Generate EnhancementOverlay.Update.cpp from EnhancementOverlay.cpp."""
with open("game/core/states/overlays/EnhancementOverlay.cpp", "r", encoding="utf-8") as f:
    lines = f.readlines()

# 0-indexed: ProcessBase 127-193, Update 276-319, ProcessSlot 320-622, On* 1566-1801
parts = []
parts.append(lines[127:193])
parts.append(lines[276:319])
parts.append(lines[320:622])
parts.append(lines[1566:1801])
body = "".join(x for p in parts for x in p)
body = body.replace("TABLE_ROW_HEIGHT", "hi::TABLE_ROW_HEIGHT")
body = body.replace("BUTTON_GAP + 4.0f", "hi::BUTTON_GAP + 4.0f")
body = body.replace("BUTTON_GAP + 2.0f", "hi::BUTTON_GAP + 2.0f")

header = """#include "EnhancementOverlay.hpp"
#include "EnhancementOverlayInternal.hpp"

#include <array>
#include <string>
#include <vector>

#include "../../api/GameplayDataAPI.hpp"
#include "../../api/InputSystemAPI.hpp"
#include "../../config/RenderTypes.hpp"
#include "../../ecs/entities/TowerAttachment.hpp"

namespace game {
namespace core {

namespace hi = enhancement_overlay_internal;

"""

footer = """
} // namespace core
} // namespace game
"""

with open("game/core/states/overlays/EnhancementOverlay.Update.cpp", "w", encoding="utf-8", newline="\n") as out:
    out.write(header + body + footer)
print("EnhancementOverlay.Update.cpp written")
