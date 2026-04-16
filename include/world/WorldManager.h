#pragma once

#include "data/WorldSnapshot.h"
#include "geometry/GeometryDatabase.h"
#include "data/Commands.h"
#include "geometry/GeometryFactory.h"
#include <unordered_map>
#include "messaging/Channel.h"
#include "world/WorldDirty.h"
#include "data/PhysicsProps.h"


class WorldManager {
public:
    WorldManager(const GeometryDatabase& geomDb,
                 GeometryFactory& geomFactory,
                 msg::Channel<WorldCommand>& worldCmds
                    );

    // Apply a single world mutation command
    void apply(const WorldCommand& cmd);

    bool consumeDirty(WorldDirty flags);
    void markDirty(WorldDirty flags);


    // Advance simulation time (no physics yet)
    void step(double dt);

    //TEMP FUNCTION TILL I CRACK MODULAR PHYSICS
    //Mabybe i replace with a world interface
    void setPose(ObjectID id, const Pose& pose) {
        auto it = objects_.find(id);
        if (it != objects_.end()) {
            it->second.pose.p = pose.p;
            it->second.pose.q = pose.q;
        }
    }

    // Produce an immutable snapshot of world state
    WorldSnapshot buildSnapshot() const;

    const PhysicsProps& getPhysicsProps(ObjectID id) const;


private:
    void applyCreate(const CreateObjectCommand& c);
    void applyRemove(const RemoveObjectCommand& c);
    void applyEdit(const EditObjectCommand& c);
    void applySetPhysicsProps(const SetPhysicsPropsCommand& c);
    void applyPatchPhysicsProps(const PatchPhysicsPropsCommand& c);

private:
    struct WorldObject {
        ObjectID   id;
        GeometryID geom;
        Pose       pose;
        Colour     colour;
        Role       role;
        PhysicsProps physics;   // authoritative
    };

    WorldDirty dirtyFlags_{WorldDirty::None};

    ObjectID nextId_{1};
    uint64_t seq_{0};
    double simTime_{0.0};

    std::unordered_map<ObjectID, WorldObject> objects_;
    const GeometryDatabase& geomDb_;
    GeometryFactory& geomFactory_;

    msg::Channel<WorldCommand>& worldCmds_;

};
