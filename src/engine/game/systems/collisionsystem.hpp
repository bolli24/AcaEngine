#pragma once

#include <engine/game/components.hpp>
#include <engine/game/registry.hpp>
#include <engine/math/convexhull.hpp>
#include <engine/utils/containers/octree.hpp>
#include <glm/gtx/quaternion.hpp>
#include <set>

#include "glm/gtc/matrix_transform.hpp"

struct CollisionInfo {
    Entity entityA = {0};
    Entity entityB = {0};
    glm::vec3 point = glm::vec3(0);
    float distance = 0.0f;
    bool calculated = true;
};

class CollisionSystem {
   public:
    static inline const float restitution = 0.85f;  // disperse some kinectic energy

    static void updateMeshCollsions(Registry& registry) {
        std::unordered_map<uint64_t, CollisionInfo> collisions;
        std::unordered_map<uint64_t, std::vector<glm::vec3>> transformedVertices;

        registry.execute<Entity, MeshCollider, Transform>([&](const Entity& entity, const MeshCollider& collider, const Transform& transform) {
            glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0f), transform.position);
            transformMatrix = glm::rotate(transformMatrix, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            transformMatrix = glm::rotate(transformMatrix, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            transformMatrix = glm::rotate(transformMatrix, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            transformMatrix = glm::scale(transformMatrix, transform.scale);

            transformedVertices[entity.id] = getTransformedVertices(collider.mesh, transformMatrix);
        });

        registry.execute<Entity, MeshCollider, Transform>([&](const Entity& entity, const MeshCollider& collider, Transform& transform) {
            const auto& colliders = registry.getComponents<MeshCollider>();
            const std::vector<Entity>& otherEntities = colliders.getEntities();

            for (const Entity& otherEntity : otherEntities) {
                if (otherEntity == entity) continue;
                bool collision = false;
                for (const glm::vec3& vertex : transformedVertices[otherEntity.id]) {
                    collision = true;
                    for (const ConvexMesh::Face& face : collider.mesh->faces) {
                        glm::vec3 faceNormal = ConvexHull::getFaceNormal({transformedVertices[entity.id][face.vertexIndices[0]],
                                                                          transformedVertices[entity.id][face.vertexIndices[1]],
                                                                          transformedVertices[entity.id][face.vertexIndices[2]]});
                        float distance = glm::dot(faceNormal, (vertex - transformedVertices[entity.id][face.vertexIndices[0]]));
                        if (distance > 0) {
                            collision = false;
                            break;
                        }
                    }

                    if (collision) {
                        CollisionInfo newCollision = {otherEntity, entity, vertex, glm::distance(vertex, collider.mesh->center), false};
                        if (collisions[entity.id].distance < newCollision.distance)
                            collisions[entity.id] = newCollision;
                    }
                }
            }
        });

        for (auto& entityCollision : collisions) {
            if (entityCollision.second.calculated) continue;

            CollisionInfo& collision = entityCollision.second;

            ConvexMesh* mesh = registry.getComponents<MeshCollider>().at(collision.entityB)->mesh;
            Transform* transformA = registry.getComponents<Transform>().at(collision.entityA);
            Transform* transformB = registry.getComponents<Transform>().at(collision.entityB);
            PhysicsObject* physicsObjectA = registry.getComponents<PhysicsObject>().at(collision.entityA);
            PhysicsObject* physicsObjectB = registry.getComponents<PhysicsObject>().at(collision.entityB);

            for (const ConvexMesh::Face& face : mesh->faces) {
                glm::vec3 faceNormal = -ConvexHull::getFaceNormal({transformedVertices[collision.entityB.id][face.vertexIndices[0]],
                                                                   transformedVertices[collision.entityB.id][face.vertexIndices[1]],
                                                                   transformedVertices[collision.entityB.id][face.vertexIndices[2]]});

                auto intersection = intersectionLinePlane(collision.point, transformA->velocity,
                                                                 transformedVertices[collision.entityB.id][face.vertexIndices[0]],
                                                                 -faceNormal);
                if (intersection.has_value() && intersection.value().second > 0 &&
                    pointInTriangle(intersection.value().first,
                                    transformedVertices[collision.entityB.id][face.vertexIndices[0]],
                                    transformedVertices[collision.entityB.id][face.vertexIndices[1]],
                                    transformedVertices[collision.entityB.id][face.vertexIndices[2]])) {
                    // Resolve Collision with Impulse method
                    float totalMass = 1.0f / physicsObjectA->mass + 1.0f / physicsObjectB->mass;
                    glm::vec3 point = intersection.value().first;
                    float penetration = intersection.value().second;

                    glm::vec3 relativeA = point - transformA->position;
                    glm::vec3 relativeB = point - transformB->position;

                    glm::mat3 transformedInertiaTensorA = getTransformedInertiaTensor(physicsObjectA->inertiaTensor, *transformA);
                    glm::mat3 transformedInertiaTensorB = getTransformedInertiaTensor(physicsObjectB->inertiaTensor, *transformB);

                    glm::vec3 angVelocityA = glm::cross(transformA->angularVelocity, relativeA);
                    glm::vec3 angVelocityB = glm::cross(transformB->angularVelocity, relativeB);
                    glm::vec3 fullVelocityA = transformA->velocity + angVelocityA;
                    glm::vec3 fullVelocityB = transformB->velocity + angVelocityB;
                    glm::vec3 contactVelocity = fullVelocityB - fullVelocityA;

                    if (penetration > glm::length(contactVelocity)) {
                        break;
                    }

                    transformA->position -= faceNormal * penetration * (1.0f / physicsObjectA->mass / totalMass);
                    transformB->position += faceNormal * penetration * (1.0f / physicsObjectB->mass / totalMass);

                    float impulseForce = glm::dot(contactVelocity, faceNormal);

                    glm::vec3 inertiaA = glm::cross(transformedInertiaTensorA * glm::cross(relativeA, faceNormal), relativeA);
                    glm::vec3 inertiaB = glm::cross(transformedInertiaTensorB * glm::cross(relativeB, faceNormal), relativeB);

                    float angularEffect = glm::dot(inertiaA + inertiaB, faceNormal);
                    float j = (-(1.0f - restitution) * impulseForce) / (totalMass + angularEffect);

                    glm::vec3 impulse = j * faceNormal;

                    transformA->velocity += -impulse * (1.0f / physicsObjectA->mass);
                    transformB->velocity += impulse * (1.0f / physicsObjectB->mass);

                    transformA->angularVelocity += transformedInertiaTensorA * glm::cross(relativeA, -impulse);
                    transformB->angularVelocity += transformedInertiaTensorB * glm::cross(relativeB, impulse);

                    break;
                }
            }
            if (collisions.find(collision.entityA.id) != collisions.end())
                collisions[collision.entityA.id].calculated = true;
        }
    }

    static glm::mat3 getTransformedInertiaTensor(const glm::mat3& inertiaTensor, const Transform& transform) {
        glm::mat4 transformMatrix = glm::mat4(1.0f);
        transformMatrix = glm::rotate(transformMatrix, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        transformMatrix = glm::rotate(transformMatrix, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        transformMatrix = glm::rotate(transformMatrix, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

        auto q = glm::toQuat(transformMatrix);

        glm::mat3 invOrientation = glm::mat3(glm::conjugate(q));
        glm::mat3 orientation = glm::mat3(q);

        glm::mat3 transformedInertiaTensor = orientation * inertiaTensor * invOrientation;
        return transformedInertiaTensor;
    }

    static glm::mat3 getCuboidInertiaTensor(float x, float y, float z, float mass) {
        return {{(12.0f / mass) / (z * z + y * y), 0.0f, 0.0f},
                {0.0f, (12.0f / mass) / (x * x + z * z), 0.0f},
                {0.0f, 0.0f, (12.0f / mass) / (x * x + y * y)}};
    }

    // Reference: https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
    static std::optional<std::pair<glm::vec3, float>> intersectionLinePlane(const glm::vec3& lineOrigin, const glm::vec3& lineDirection,
                                                                            const glm::vec3& planeOrigin, const glm::vec3& planeNormal) {
        glm::vec3 difference = lineOrigin - planeOrigin;
        float product1 = glm::dot(difference, planeNormal);
        float product2 = glm::dot(lineDirection, planeNormal);
        if (product2 < 0.00001f && product2 > -0.00001f) return {};
        float distance = product1 / product2;
        std::pair<glm::vec3, float> intersection = {lineOrigin - lineDirection * distance, distance * glm::length(lineDirection)};
        return intersection;
    }

    static bool pointInTriangle(const glm::vec3& p, const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        float n = glm::length(glm::cross(a - b, a - c));
        float u = glm::length(glm::cross(p - a, p - b));
        float v = glm::length(glm::cross(p - b, p - c));
        float w = glm::length(glm::cross(p - c, p - a));

        float sum = u + v + w - n;

        if (sum < 0.0001f && sum > -0.0001f) {
            return true;
        }
        return false;
    }

    static void addMeshCollider(Registry& registry, const Entity& entity, const utils::MeshData::Handle mesh) {
        if (convexHulls.find(mesh) == convexHulls.end())
            convexHulls[mesh] = ConvexHull::getConvexHull(mesh->positions);

        registry.getComponents<MeshCollider>().insert(entity, {ColliderType::Target, &convexHulls[mesh]});
    }

    static void updateAABBCollisions(Registry& registry) {
        utils::SparseOctree<Entity, 3, float> octree;

        registry.execute<Entity, AABBCollider, Transform>([&](Entity& entity, AABBCollider& collider, Transform& transform) {
            collider.aabb.min += transform.velocity;
            collider.aabb.max += transform.velocity;
            octree.insert(collider.aabb, entity);
        });

        std::vector<Entity> toRemove;

        registry.execute<Entity, AABBCollider>([&](const Entity& entity, const AABBCollider& collider) {
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

   private:
    static std::unordered_map<utils::MeshData::Handle, ConvexMesh> convexHulls;

    static std::vector<glm::vec3> getTransformedVertices(const ConvexMesh* mesh, const glm::mat4 transformMatrix) {
        std::vector<glm::vec3> positions = mesh->positions;

        for (glm::vec3& pos : positions) {
            pos = transformMatrix * glm::vec4(pos, 1.0f);
        }

        return positions;
    }
};