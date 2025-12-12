#pragma once
#include "geometry/GeometryEntry.h"
#include <unordered_map>

class GeometryDatabase {
public:
    GeometryDatabase() = default;

    // Initialization-time only
    void registerGeometry(const GeometryEntry& entry);

    // Runtime (read-only)
    const GeometryEntry& get(GeometryID id) const;

    bool contains(GeometryID id) const;

private:
    std::unordered_map<GeometryID, GeometryEntry> entries_;
};
