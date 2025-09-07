#pragma once

namespace mc_tables {

// Corner offsets (must match your marching cubes convention)
extern const int CORNER[8][3];

// Edge -> corner mapping (must match tables)
extern const int EDGE2C[12][2];

// Lookup tables
extern const int edgeTable[256];
extern const int triTable[256][16];

} // namespace mc_tables
