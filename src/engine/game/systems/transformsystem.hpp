#pragma once

#include <engine/game/components.hpp>
#include <engine/game/registry.hpp>
#include <glm/glm.hpp>

using namespace graphics;

class TransformSystem {
   public:
    static void updateTransforms(Registry& registry) {

        registry.execute<Entity, Transform>([&](Entity& entity, Transform& transform) {
            transform.position += transform.velocity;
            transform.rotation += transform.angularVelocity;
        });
    }
};