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

    m_program.attach(vertexShader);
    m_program.attach(fragmentShader);
    m_program.link();

    Light light;
    light.createLights(m_program);
}

void MeshRenderer::present(const Camera& camera, const glm::vec3& cameraPosition) {
    m_program.setUniform(1, camera.getProjection());
    m_program.setUniform(2, camera.getView());
    m_program.setUniform(4, cameraPosition);

    for (auto& meshInstance : m_meshInstances) {
        m_program.setUniform(3, meshInstance.transform);
        m_program.use();
        meshInstance.texture.bind(0);
        meshInstance.mesh.draw();
    }
}

void MeshRenderer::draw(Mesh& _mesh, Texture2D& _texture, glm::mat4& _transform) {
    m_meshInstances.push_back({_mesh, _texture, _transform});
}

void MeshRenderer::clear() {
    m_meshInstances.clear();
}
}  // namespace graphics
