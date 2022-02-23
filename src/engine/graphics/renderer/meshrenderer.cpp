#include "meshrenderer.hpp"

#include <engine/graphics/core/lightComponent.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/resources.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "../core/opengl.hpp"
#include "../core/texture.hpp"

namespace graphics {

MeshRenderer::MeshRenderer() {
    const auto* fragmentShader = ShaderManager::get("shader/base.frag", ShaderType::FRAGMENT);
    const auto* vertexShader = ShaderManager::get("shader/base.vert", ShaderType::VERTEX);

    program.attach(vertexShader);
    program.attach(fragmentShader);
    program.link();
    program.use();
}

void MeshRenderer::present(const Camera& camera, const glm::vec3& cameraPosition) {
    program.setUniform(1, camera.getProjection());
    program.setUniform(2, camera.getView());
    program.setUniform(4, cameraPosition);

    for (auto& meshInstance : meshInstances) {
        program.setUniform(3, meshInstance.transform);

        meshInstance.texture.bind(0);
        meshInstance.mesh.draw();
    }
}

void MeshRenderer::draw(Mesh& _mesh, Texture2D& _texture, glm::mat4& _transform) {
    meshInstances.push_back({_mesh, _texture, _transform});
}

void MeshRenderer::clear() {
    meshInstances.clear();
}
}  // namespace graphics
