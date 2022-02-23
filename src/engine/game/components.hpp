#pragma once

#include <engine/math/geometrictypes.hpp>
#include <glm/glm.hpp>

enum class ColliderType { Projectile, MovingTarget, Target };

struct Transform {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 rotation;
    glm::vec3 angularVelocity;
    glm::vec3 scale = glm::vec3(1.0f);
};

struct AABBCollider {
    ColliderType colliderType;
    math::AABB<3> aabb;
    glm::vec3 offset;
};

struct Light {
    glm::vec3 position;
    glm::vec3 color;
};