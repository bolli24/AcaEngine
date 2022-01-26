#pragma once

#include <vector>

#include "../camera.hpp"
#include "../core/shader.hpp"
#include "engine/game/registry.hpp"
#include "glm/glm.hpp"

namespace graphics {

class Mesh;
class Texture2D;

struct MeshInstance {
    Mesh& mesh;
    Texture2D& texture;
    glm::mat4 transform;
};

class MeshRenderer {
   public:
    MeshRenderer();

    void draw(Mesh& _mesh, Texture2D& _texture, glm::mat4& _transform);

    void present(const Camera& _camera, const glm::vec3& cameraPosition);
    void clear();

   private:
    std::vector<MeshInstance> m_meshInstances;
    Program m_program;
};
}  // namespace graphics