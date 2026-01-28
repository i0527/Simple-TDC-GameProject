# -*- coding: utf-8 -*-
"""Generate EnhancementOverlay.Render.cpp from EnhancementOverlay.cpp."""
with open("game/core/states/overlays/EnhancementOverlay.cpp", "r", encoding="utf-8") as f:
    lines = f.readlines()

# Render 623-1565 (Render through RenderAttachmentSlot), then RenderSearchBox 1943-2100
# 0-indexed: 622:1565, 1942:2100
body_lines = lines[622:1565] + lines[1942:2100]
body = "".join(body_lines)

# Replace anonymous-namespace symbols with hi::
for name in ("FONT_TITLE", "FONT_SECTION", "FONT_HEADER", "FONT_BODY", "FONT_BUTTON", "FONT_CAPTION",
             "TABLE_ROW_HEIGHT", "BUTTON_GAP", "FormatFloat", "BuildAttachmentEffectText", "ToAttachmentTargetLabel"):
    body = body.replace(name, "hi::" + name)

header = """#include "EnhancementOverlay.hpp"
#include "EnhancementOverlayInternal.hpp"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "../../api/BaseSystemAPI.hpp"
#include "../../api/GameplayDataAPI.hpp"
#include "../../config/RenderPrimitives.hpp"
#include "../../config/RenderTypes.hpp"
#include "../../ecs/entities/TowerAttachment.hpp"
#include "../../system/TowerEnhancementEffects.hpp"
#include "../../ui/OverlayColors.hpp"
#include "../../ui/UIEffects.hpp"

namespace game {
namespace core {

namespace hi = enhancement_overlay_internal;

"""

footer = """
} // namespace core
} // namespace game
"""

with open("game/core/states/overlays/EnhancementOverlay.Render.cpp", "w", encoding="utf-8", newline="\n") as out:
    out.write(header + body + footer)
print("EnhancementOverlay.Render.cpp written")
