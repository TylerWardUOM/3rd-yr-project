#include "geometry/GeometryDatabase.h"
#include <stdexcept>

void GeometryDatabase::registerGeometry(const GeometryEntry& entry) {
    if (entries_.count(entry.id)) {
        throw std::runtime_error("GeometryID already registered");
    }

    entries_.emplace(entry.id, entry);
    typeIndex_[entry.type] = entry.id;
}


const GeometryEntry& GeometryDatabase::get(GeometryID id) const {
    auto it = entries_.find(id);
    if (it == entries_.end()) {
        throw std::runtime_error("Invalid GeometryID lookup");
    }
    return it->second;
}

GeometryID GeometryDatabase::getByType(SurfaceType type) const {
    auto it = typeIndex_.find(type);
    if (it == typeIndex_.end())
        throw std::runtime_error("Type not registered");

    return it->second;
}


bool GeometryDatabase::contains(GeometryID id) const {
    return entries_.count(id) != 0;
}
