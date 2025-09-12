#pragma once
#include "world/DoubleBuffer.h"
#include "haptics/ToolState.h"

struct HapticsBuffers {
    // Producers:
    //  - Device I/O writes ToolIn.devicePose_ws
    //  - UI/Render writes ToolIn.refPose_ws
    DoubleBuffer<ToolIn> inBuf;

    // Haptics loop writes these:
    DoubleBuffer<ToolOut>        outBuf;   // for device command + proxy pose
    DoubleBuffer<HapticSnapshot> snapBuf;  // for visualisation HUD/plots
};
