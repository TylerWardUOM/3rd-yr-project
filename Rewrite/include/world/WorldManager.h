#pragma once

#include "data/WorldSnapshot.h"
#include "geometry/GeometryDatabase.h"
#include "data/Commands.h"
#include "geometry/GeometryFactory.h"
#include <unordered_map>

class WorldManager {
public:
    WorldManager(const GeometryDatabase& geomDb,
                 GeometryFactory& geomFactory);

    // Apply a single world mutation command
    void apply(const PhysicsCommand& cmd);

    // Advance simulation time (no physics yet)
    void step(double dt);

    // Produce an immutable snapshot of world state
    WorldSnapshot buildSnapshot() const;

private:
    void applyCreate(const CreateObjectCommand& c);
    void applyRemove(const RemoveObjectCommand& c);
    void applyEdit(const EditObjectCommand& c);

private:
    struct WorldObject {
        ObjectID   id;
        GeometryID geom;
        Pose       pose;
        Colour     colour;
        Role       role;
    };

    ObjectID nextId_{1};
    uint64_t seq_{0};
    double simTime_{0.0};

    std::unordered_map<ObjectID, WorldObject> objects_;
    const GeometryDatabase& geomDb_;
    GeometryFactory& geomFactory_;

};
