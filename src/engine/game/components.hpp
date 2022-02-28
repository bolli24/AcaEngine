#pragma once

#include <engine/graphics/core/texture.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/math/geometrictypes.hpp>
#include <glm/glm.hpp>
#include <engine/math/convexhull.hpp>

enum class ColliderType { Projectile,
                          MovingTarget,
                          Target };

struct Transform {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 rotation;
    glm::vec3 angularVelocity;
    glm::vec3 scale = glm::vec3(1.0f);
};

struct MeshRender {
    graphics::Mesh* mesh;
    graphics::Texture2D::Handle texture;
};

struct MeshCollider {

    ColliderType colliderType;
    ConvexMesh* mesh;
};

struct PhysicsObject {
    float mass;
    float inertiaTensor;
};

struct AABBCollider {
    ColliderType colliderType;
    math::AABB<3> aabb;
};

struct Light {
    glm::vec3 position;
    glm::vec3 color;
};