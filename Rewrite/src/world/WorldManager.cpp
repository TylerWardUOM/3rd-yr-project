#include "world/WorldManager.h"
#include <variant>
#include <iostream>

WorldManager::WorldManager(const GeometryDatabase& geomDb,
                           GeometryFactory& geomFactory,
                            msg::Channel<WorldCommand>& worldCmds
                               )
    : geomDb_(geomDb),
      geomFactory_(geomFactory),
      worldCmds_(worldCmds) {}



void WorldManager::apply(const WorldCommand& cmd) {
    std::visit([this](auto&& c) {
        using T = std::decay_t<decltype(c)>;
        if constexpr (std::is_same_v<T, CreateObjectCommand>) {
            applyCreate(c);
        } else if constexpr (std::is_same_v<T, RemoveObjectCommand>) {
            applyRemove(c);
        } else if constexpr (std::is_same_v<T, EditObjectCommand>) {
            applyEdit(c);
        } else if constexpr (std::is_same_v<T, SetPhysicsPropsCommand>) {
            applySetPhysicsProps(c);
        } else if constexpr (std::is_same_v<T, PatchPhysicsPropsCommand>) {
            applyPatchPhysicsProps(c);
        } else {
            // Ignore other command types for now (e.g. ForceCommand)
        }
    }, cmd);
}

void WorldManager::step(double dt) {
    //Maybe move commands to be in phsyics step
    std::vector<WorldCommand> cmds;
    worldCmds_.drain(cmds);
    //std::cout << "WorldManager: Processing " << cmds.size() << " commands in step.\n";

    for (const auto& cmd : cmds)
        apply(cmd);

    simTime_ += dt;
}

WorldSnapshot WorldManager::buildSnapshot() const {
    WorldSnapshot snap;
    snap.seq = ++const_cast<WorldManager*>(this)->seq_;
    snap.simTime = simTime_;
    snap.objects.reserve(objects_.size());

    for (const auto& [id, obj] : objects_) {
        ObjectState s;
        s.id = obj.id;
        s.geom = obj.geom;
        s.T_ws = obj.pose;
        s.v_ws = Vec3(0,0,0);
        s.w_ws = Vec3(0,0,0);
        s.colourOverride = obj.colour;
        s.role = obj.role;
        s.physics = obj.physics; //used for ui updating
        snap.objects.push_back(s);
    }
    return snap;
}

void WorldManager::applyCreate(const CreateObjectCommand& c) {
    WorldObject obj;
    obj.id = nextId_++;
    obj.geom = c.geom;
    obj.pose = c.initialPose;
    obj.colour = c.colour;
    obj.role = c.role;
    obj.physics.dynamic = c.dynamic;  //maybe change to use full physics props 

    objects_.emplace(obj.id, obj);
    markDirty(WorldDirty::Topology);
}

void WorldManager::applyRemove(const RemoveObjectCommand& c) {
    objects_.erase(c.id);
    markDirty(WorldDirty::Topology);
}

void WorldManager::applyEdit(const EditObjectCommand& c) {
    auto it = objects_.find(c.id);
    if (it == objects_.end()) return;

    it->second.pose = c.newPose;
    it->second.colour = c.newColour;
    //std::cout << "WorldManager: Edited object " << c.id << " to new pose." << std::endl;
    markDirty(WorldDirty::Physics);
}


void WorldManager::markDirty(WorldDirty flags) {
    dirtyFlags_ = static_cast<WorldDirty>(
        static_cast<uint8_t>(dirtyFlags_) |
        static_cast<uint8_t>(flags)
    );
}

bool WorldManager::consumeDirty(WorldDirty flags) {
    bool wasDirty = (static_cast<uint8_t>(dirtyFlags_) &
                     static_cast<uint8_t>(flags)) != 0;
    dirtyFlags_ = static_cast<WorldDirty>(
        static_cast<uint8_t>(dirtyFlags_) &
        ~static_cast<uint8_t>(flags)
    );
    return wasDirty;
}

void WorldManager::applySetPhysicsProps(const SetPhysicsPropsCommand& c) {
    auto it = objects_.find(c.id);
    if (it == objects_.end()) return;

    it->second.physics = c.props;

    // Mark physics config dirty so the physics engine rebuilds actors
    markDirty(WorldDirty::Physics);
}

const PhysicsProps& WorldManager::getPhysicsProps(ObjectID id) const {
    static PhysicsProps defaultProps;
    auto it = objects_.find(id);
    if (it == objects_.end()) return defaultProps;

    return it->second.physics;
}

void WorldManager::applyPatchPhysicsProps(const PatchPhysicsPropsCommand& c) {
    auto it = objects_.find(c.id);
    if (it == objects_.end()) return;

    PhysicsProps& p = it->second.physics;

    if (c.patch.dynamic)         p.dynamic         = *c.patch.dynamic;
    if (c.patch.kinematic)       p.kinematic       = *c.patch.kinematic;
    if (c.patch.density)         p.density         = *c.patch.density;
    if (c.patch.mass)            p.mass            = c.patch.mass;
    if (c.patch.linDamping)      p.linDamping      = *c.patch.linDamping;
    if (c.patch.angDamping)      p.angDamping      = *c.patch.angDamping;
    if (c.patch.staticFriction)  p.staticFriction  = *c.patch.staticFriction;
    if (c.patch.dynamicFriction) p.dynamicFriction = *c.patch.dynamicFriction;
    if (c.patch.restitution)     p.restitution     = *c.patch.restitution;

    markDirty(WorldDirty::Physics);
}
