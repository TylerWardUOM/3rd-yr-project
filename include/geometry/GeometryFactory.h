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
    GeometryID getCube(); // cube of given side length

private:
    GeometryDatabase& db_;
    RenderMeshRegistry& meshRegistry_;

    GeometryID nextId_{1};

    // Cache so geometry is only created once
    std::optional<GeometryID> planeId_;
    std::optional<GeometryID>  sphereId_;
    std::optional<GeometryID> cubeId_;

    GeometryID registerPlane();
    GeometryID registerSphere();
    GeometryID registerCube(); 
};
