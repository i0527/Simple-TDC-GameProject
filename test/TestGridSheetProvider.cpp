#include <iostream>
#include <cassert>

// Test includes
#include "shared/include/Data/Graphics/FrameRef.h"
#include "shared/include/Data/Graphics/AnimClip.h"
#include "shared/include/Data/Graphics/IFrameProvider.h"
#include "game/include/Game/Graphics/GridSheetProvider.h"

// Mock Texture2D for testing
Texture2D CreateMockTexture() {
    Texture2D tex = {0};
    tex.width = 256 * 4;  // 4 frames per row
    tex.height = 256 * 2; // 2 rows
    tex.mipmaps = 1;
    tex.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    tex.id = 1; // Mock ID
    return tex;
}

int main() {
    std::cout << "Testing GridSheetProvider..." << std::endl;

    // Create mock texture
    Texture2D mockTexture = CreateMockTexture();

    // Test configuration
    GridSheetProvider::Config config{256, 256, 4}; // 256x256 cells, 4 per row

    // Create provider
    GridSheetProvider provider(mockTexture, config);

    // Register test clips
    provider.RegisterClip("idle", 0, 4, true, 12.0f);   // frames 0-3, loop, 12fps
    provider.RegisterClip("walk", 4, 4, true, 15.0f);   // frames 4-7, loop, 15fps
    provider.RegisterClip("attack", 8, 6, false, 20.0f); // frames 8-13, no loop, 20fps

    // Test clip existence
    assert(provider.HasClip("idle") == true);
    assert(provider.HasClip("walk") == true);
    assert(provider.HasClip("attack") == true);
    assert(provider.HasClip("nonexistent") == false);
    std::cout << "✓ Clip existence tests passed" << std::endl;

    // Test frame counts
    assert(provider.GetFrameCount("idle") == 4);
    assert(provider.GetFrameCount("walk") == 4);
    assert(provider.GetFrameCount("attack") == 6);
    std::cout << "✓ Frame count tests passed" << std::endl;

    // Test FPS
    assert(provider.GetClipFps("idle") == 12.0f);
    assert(provider.GetClipFps("walk") == 15.0f);
    assert(provider.GetClipFps("attack") == 20.0f);
    std::cout << "✓ FPS tests passed" << std::endl;

    // Test looping
    assert(provider.IsLooping("idle") == true);
    assert(provider.IsLooping("walk") == true);
    assert(provider.IsLooping("attack") == false);
    std::cout << "✓ Looping tests passed" << std::endl;

    // Test frame retrieval
    FrameRef frame = provider.GetFrame("idle", 0);
    assert(frame.valid == true);
    assert(frame.tex == &mockTexture);
    assert(frame.src.width == 256);
    assert(frame.src.height == 256);
    assert(frame.src.x == 0);     // First frame in first row
    assert(frame.src.y == 0);
    assert(frame.durationSec == 1.0f / 12.0f); // 1/12 seconds
    std::cout << "✓ Frame retrieval tests passed" << std::endl;

    // Test frame indexing
    frame = provider.GetFrame("walk", 1);
    assert(frame.src.x == 256); // Second frame in first row
    assert(frame.src.y == 0);

    frame = provider.GetFrame("attack", 4);
    assert(frame.src.x == 256); // First frame in second row (frame 8+4=12, but wait...)
    // Actually, attack starts at frame 8, so frame index 4 would be frame 12
    // 12 % 4 = 0 (frames per row), so x = 0 * 256 = 0
    // 12 / 4 = 3 rows, so y = 3 * 256 = 768
    assert(frame.src.x == 0);
    assert(frame.src.y == 768); // Third row (0-indexed: 0,1,2,3)
    std::cout << "✓ Frame indexing tests passed" << std::endl;

    // Test invalid frame index
    frame = provider.GetFrame("idle", 10); // Out of bounds
    assert(frame.valid == false);
    std::cout << "✓ Invalid frame index test passed" << std::endl;

    // Test invalid clip
    frame = provider.GetFrame("nonexistent", 0);
    assert(frame.valid == false);
    std::cout << "✓ Invalid clip test passed" << std::endl;

    std::cout << "All GridSheetProvider tests passed! ✓" << std::endl;
    return 0;
}
