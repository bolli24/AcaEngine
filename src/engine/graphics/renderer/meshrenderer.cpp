#include "meshrenderer.hpp"

#include "../core/opengl.hpp"
#include "../core/texture.hpp"

#include <engine/graphics/resources.hpp>
#include <engine/graphics/renderer/mesh.hpp>

namespace graphics {

MeshRenderer::MeshRenderer() {
    const auto* fragmentShader = ShaderManager::get("shader/demo.frag", ShaderType::FRAGMENT);
    const auto* vertexShader = ShaderManager::get("shader/demo.vert", ShaderType::VERTEX);

    m_program.attach(vertexShader);
    m_program.attach(fragmentShader);
    m_program.link();
}

void MeshRenderer::present(const Camera& camera) {
    for (const auto& meshInstance : m_meshInstances) {
        glm::mat4 meshProjection = meshInstance.transform * camera.getViewProjection();
        m_program.setUniform(0, meshProjection);
        meshInstance.texture.bind(0);
        meshInstance.mesh.draw();
    }
}

void MeshRenderer::draw(const Mesh& _mesh, const Texture2D& _texture, const glm::mat4& _transform) {
    m_meshInstances.push_back({_mesh, _texture, _transform});
}

void MeshRenderer::clear() {
    m_meshInstances.clear();
}
}  // namespace graphics
