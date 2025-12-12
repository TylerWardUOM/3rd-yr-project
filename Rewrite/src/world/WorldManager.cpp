#include "world/WorldManager.h"
#include <variant>

WorldManager::WorldManager(const GeometryDatabase& geomDb,
                           GeometryFactory& geomFactory)
    : geomDb_(geomDb),
      geomFactory_(geomFactory) {}



void WorldManager::apply(const PhysicsCommand& cmd) {
    std::visit([this](auto&& c) {
        using T = std::decay_t<decltype(c)>;
        if constexpr (std::is_same_v<T, CreateObjectCommand>) {
            applyCreate(c);
        } else if constexpr (std::is_same_v<T, RemoveObjectCommand>) {
            applyRemove(c);
        } else if constexpr (std::is_same_v<T, EditObjectCommand>) {
            applyEdit(c);
        } else {
            // Ignore other command types for now (e.g. ForceCommand)
        }
    }, cmd);
}

void WorldManager::step(double dt) {
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

    objects_.emplace(obj.id, obj);
}

void WorldManager::applyRemove(const RemoveObjectCommand& c) {
    objects_.erase(c.id);
}

void WorldManager::applyEdit(const EditObjectCommand& c) {
    auto it = objects_.find(c.id);
    if (it == objects_.end()) return;

    it->second.pose = c.newPose;
}
