#pragma once
#include "world/DoubleBuffer.h"
#include "haptics/ToolState.h"

/// @ingroup haptics
/// @brief Haptics buffers: double buffers for tool input, output, and haptics snapshot
struct HapticsBuffers {
    // Producers:
    //  - Device I/O writes ToolIn.devicePose_ws
    //  - UI/Render writes ToolIn.refPose_ws
    DoubleBuffer<ToolIn> inBuf; ///< for device pose + user input

    // Haptics loop writes these:
    DoubleBuffer<ToolOut>        outBuf;   ///< for device commands and proxy pose
    DoubleBuffer<HapticSnapshot> snapBuf;  ///< for rendering and logging
};
