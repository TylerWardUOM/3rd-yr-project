#pragma once
#include "geometry/GeometryDatabase.h"
#include "render/RenderMeshRegistry.h"
#include <optional>
#include <unordered_map>

class GeometryFactory {
public:
    explicit GeometryFactory(GeometryDatabase& db, RenderMeshRegistry& meshRegistry);

    GeometryID getPlane();                 // infinite plane
    GeometryID getSphere();   // sphere of given radius

private:
    GeometryDatabase& db_;
    RenderMeshRegistry& meshRegistry_;

    GeometryID nextId_{1};

    // Cache so geometry is only created once
    std::optional<GeometryID> planeId_;
    std::optional<GeometryID>  sphereId_;

    GeometryID registerPlane();
    GeometryID registerSphere();
};
