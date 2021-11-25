#pragma once

#include "../core/shader.hpp"
#include "../camera.hpp"
#include "glm/glm.hpp"
#include <vector>

namespace graphics {

class Mesh;
class Texture2D;

struct MeshInstance {
    const Mesh& mesh;
    const Texture2D& texture;
    const glm::mat4& transform;
};

class MeshRenderer {
   public:
    MeshRenderer();

    void draw(const Mesh& _mesh, const Texture2D& _texture, const glm::mat4& _transform);

    void present(const Camera& _camera, const glm::vec3& cameraPosition);
    void clear();

   private:
    std::vector<MeshInstance> m_meshInstances;
    Program m_program;
};
}  // namespace graphics