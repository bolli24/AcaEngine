#pragma once

#include <engine/game/components.hpp>
#include <engine/game/registry.hpp>
#include <engine/math/convexhull.hpp>
#include <engine/utils/containers/octree.hpp>
#include <set>

#include "glm/gtc/matrix_transform.hpp"

struct CollisionInfo {
    Entity entityA;
    Entity entityB;
    glm::vec3 point;
};

class CollisionSystem {
   public:
    static inline const float restitution = 0.8f;  // disperse some kinectic energy

    static void updateMeshCollsions(Registry& registry) {
        std::vector<CollisionInfo> collisions;

        std::unordered_map<uint64_t, std::vector<glm::vec3>> transformedVertices;

        registry.execute<Entity, MeshCollider, Transform>([&](Entity& entity, MeshCollider& collider, Transform& transform) {
            glm::mat4 transformMatrix = glm::translate(glm::mat4(1.0f), transform.position);
            transformMatrix = glm::rotate(transformMatrix, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            transformMatrix = glm::rotate(transformMatrix, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            transformMatrix = glm::rotate(transformMatrix, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            transformMatrix = glm::scale(transformMatrix, transform.scale);

            transformedVertices[entity.id] = getTransformedVertices(collider.mesh, transformMatrix);
        });

        registry.execute<Entity, MeshCollider, Transform>([&](Entity& entity, MeshCollider& collider, Transform& transform) {
            auto& colliders = registry.getComponents<MeshCollider>();
            auto otherEntities = colliders.getEntities();

            for (Entity& otherEntity : otherEntities) {
                if (otherEntity == entity) continue;
                bool collision = false;
                for (glm::vec3& vertex : transformedVertices[otherEntity.id]) {
                    collision = true;
                    for (ConvexMesh::Face& face : collider.mesh->faces) {
                        float distance = glm::dot(face.normal, (vertex - transformedVertices[entity.id][face.vertexIndices[0]]));
                        if (distance > 0) {
                            collision = false;
                            break;
                        }
                    }

                    if (collision) {
                        collisions.push_back({otherEntity, entity, vertex});
                        break;
                    }
                }
            }
        });

        for (CollisionInfo& collision : collisions) {
            ConvexMesh* mesh = registry.getComponents<MeshCollider>().at(collision.entityA)->mesh;
            Transform* transformA = registry.getComponents<Transform>().at(collision.entityA);
            Transform* transformB = registry.getComponents<Transform>().at(collision.entityB);
            PhysicsObject* physicsObjectA = registry.getComponents<PhysicsObject>().at(collision.entityA);
            PhysicsObject* physicsObjectB = registry.getComponents<PhysicsObject>().at(collision.entityB);

            for (ConvexMesh::Face& face : mesh->faces) {
                auto intersection = intersectionLinePlane(collision.point, -transformA->velocity,
                                                          transformedVertices[collision.entityA.id][face.vertexIndices[0]],
                                                          face.normal);
                if (intersection.has_value() && pointInTriangle(intersection.value().first,
                                                                transformedVertices[collision.entityA.id][face.vertexIndices[0]],
                                                                transformedVertices[collision.entityA.id][face.vertexIndices[1]],
                                                                transformedVertices[collision.entityA.id][face.vertexIndices[2]])) {
                    // Resolve Collision with Impulse method
                    float totalMass = 1.0f / physicsObjectA->mass + 1.0f / physicsObjectB->mass;
                    glm::vec3 point = intersection.value().first;
                    glm::vec3 normal = -glm::cross(transformedVertices[collision.entityA.id][face.vertexIndices[0]] -
                                                      transformedVertices[collision.entityA.id][face.vertexIndices[1]],
                                                  transformedVertices[collision.entityA.id][face.vertexIndices[0]] -
                                                      transformedVertices[collision.entityA.id][face.vertexIndices[2]]);
                    float penetration = intersection.value().second;

                    glm::vec3 relativeA = point - transformA->position;
                    glm::vec3 relativeB = point - transformB->position;

                    transformA->position -= normal * penetration * (1.0f / physicsObjectA->mass / totalMass);
                    transformB->position += normal * penetration * (1.0f / physicsObjectB->mass / totalMass);

                    glm::vec3 angVelocityA = glm::cross(transformA->angularVelocity, relativeA);
                    glm::vec3 angVelocityB = glm::cross(transformB->angularVelocity, relativeB);
                    glm::vec3 fullVelocityA = transformA->velocity + angVelocityA;
                    glm::vec3 fullVelocityB = transformB->velocity + angVelocityB;
                    glm::vec3 contactVelocity = fullVelocityB - fullVelocityA;

                    float impulseForce = glm::dot(contactVelocity, normal);

                    glm::vec3 inertiaA = glm::cross(physicsObjectA->inertiaTensor * glm::cross(relativeA, normal), relativeA);
                    glm::vec3 inertiaB = glm::cross(physicsObjectB->inertiaTensor * glm::cross(relativeB, normal), relativeB);

                    float angularEffect = glm::dot(inertiaA + inertiaB, normal);
                    float j = (-(1.0f - restitution) * impulseForce) / (totalMass + angularEffect);

                    glm::vec3 impulse = j * normal;

                    transformA->angularVelocity += (1.0f / physicsObjectA->inertiaTensor) * -impulse;
                    transformA->velocity -= glm::cross(relativeA, -impulse) * (1.0f / physicsObjectA->mass);

                    transformB->angularVelocity += (1.0f / physicsObjectB->inertiaTensor) * impulse;
                    transformB->velocity -= glm::cross(relativeB, impulse) * (1.0f / physicsObjectB->mass);

                    break;
                }
            }
        }
    }

    // Reference: https://en.wikipedia.org/wiki/Line%E2%80%93plane_intersection
    static std::optional<std::pair<glm::vec3, float>> intersectionLinePlane(glm::vec3 lineOrigin, glm::vec3 lineDirection, glm::vec3 planeOrigin, glm::vec3 planeNormal) {
        float dot = glm::dot(lineDirection, planeNormal);
        if (dot < 0.00001f && dot > -0.00001f) return {};

        float distance = glm::dot(planeOrigin - lineOrigin, planeNormal) / dot;
        glm::vec3 intersectionPoint = lineOrigin + lineDirection * distance;
        return {{intersectionPoint, distance}};
    }

    static bool pointInTriangle(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
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

    static void addMeshCollider(Registry& registry, Entity& entity, utils::MeshData::Handle mesh) {
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

   private:
    static std::unordered_map<utils::MeshData::Handle, ConvexMesh> convexHulls;

    static std::vector<glm::vec3> getTransformedVertices(ConvexMesh* mesh, glm::mat4 transformMatrix) {
        std::vector<glm::vec3> positions = mesh->positions;

        for (glm::vec3& pos : positions) {
            pos = transformMatrix * glm::vec4(pos, 1.0f);
            auto p = pos;
        }

        return positions;
    }
};