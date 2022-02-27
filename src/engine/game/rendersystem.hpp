#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/game/components.hpp>
#include <engine/game/registry.hpp>
#include <engine/graphics/core/texture.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>

using namespace graphics;

class RenderSystem {
   public:
    static void draw(Registry& registry, MeshRenderer& meshRenderer) {
        meshRenderer.clear();

        registry.execute<Transform, MeshRender>([&](Transform& transform, MeshRender& meshRender) {
            glm::mat4 newTransform = glm::translate(glm::mat4(1.0f), transform.position);
            newTransform = glm::rotate(newTransform, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
            newTransform = glm::rotate(newTransform, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
            newTransform = glm::rotate(newTransform, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
            newTransform = glm::scale(newTransform, transform.scale);

            meshRenderer.draw(*meshRender.mesh, *const_cast<Texture2D*>(meshRender.texture), newTransform);
        });
    }
};