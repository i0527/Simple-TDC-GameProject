#pragma once

#include <raylib.h>

namespace Components {
    // Position component - simple data structure
    struct Position {
        float x;
        float y;
    };

    // Velocity component - simple data structure
    struct Velocity {
        float dx;
        float dy;
    };

    // Renderable component - simple data structure
    struct Renderable {
        Color color;
        float radius;
    };

    // Player tag component
    struct Player {};
}
