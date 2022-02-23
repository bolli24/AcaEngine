#pragma once

#include <engine/game/components.hpp>
#include <engine/game/registry.hpp>
#include <engine/utils/containers/octree.hpp>
#include <set>

class CollisionSystem {
   public:
    static void update(Registry& registry) {
        utils::SparseOctree<Entity, 3, float> octree;

        registry.execute<Entity, AABBCollider, Transform>([&](Entity& entity, AABBCollider& collider, Transform& transform) {
            collider.aabb.min += transform.velocity;
            collider.aabb.max += transform.velocity;
            octree.insert(collider.aabb, entity);
        });

        std::vector<Entity> toRemove;

        registry.execute<Entity, AABBCollider>([&](Entity& entity, AABBCollider& collider) {
            if (collider.colliderType == ColliderType::Projectile) {
                utils::SparseOctree<Entity, 3, float>::AABBQuery query(collider.aabb);
                octree.traverse(query);

                std::vector<Entity> hits;
                std::set<Entity> s(query.hits.begin(), query.hits.end());
                hits.assign(s.begin(), s.end());

                auto& colliders = registry.getComponents<AABBCollider>();

                for (Entity& hit : hits) {
                    if (entity == hit || colliders.at(hit)->colliderType == ColliderType::Projectile) continue;

                    octree.remove(colliders.at(hit)->aabb, hit);
                    toRemove.push_back(hit);
                }
            }
        });

        for (auto& entity : toRemove) {
            registry.erase(entity);
        }
    }
};