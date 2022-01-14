#include "meshrenderer.hpp"

#include "../core/opengl.hpp"
#include "../core/texture.hpp"

#include <engine/graphics/resources.hpp>
#include <engine/graphics/renderer/mesh.hpp>

namespace graphics {

MeshRenderer::MeshRenderer() {
    const auto* fragmentShader = ShaderManager::get("shader/base.frag", ShaderType::FRAGMENT);
    const auto* vertexShader = ShaderManager::get("shader/base.vert", ShaderType::VERTEX);

    m_program.attach(vertexShader);
    m_program.attach(fragmentShader);
    m_program.link();
}

    const glm::vec3 lightPos1(1.2f, -3.f, 2.f);
    const glm::vec3 lightDiff1(1.f, 1.f, 1.f);
    const glm::vec3 lightSpec1(1.f, 1.f, 1.f);

    const glm::vec3 lightPos2( 0.f, 0.f, 2.f);
    const glm::vec3 lightDiff2(1.f, 0.f, 0.f);
    const glm::vec3 lightSpec2(1.f, 0.f, 0.f);

void MeshRenderer::present(const Camera& camera, const glm::vec3& cameraPosition) {
    m_program.setUniform(0, lightPos1);
    m_program.setUniform(5, lightDiff1);
    m_program.setUniform(6, lightSpec1);

    m_program.setUniform(7, lightPos2);
    m_program.setUniform(8, lightDiff2);
    m_program.setUniform(9, lightSpec2);

    m_program.setUniform(1, camera.getProjection());
    m_program.setUniform(2, camera.getView());
    m_program.setUniform(4, cameraPosition);

    for (const auto& meshInstance : m_meshInstances) {
        m_program.setUniform(3, meshInstance.transform);
        m_program.use();
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
