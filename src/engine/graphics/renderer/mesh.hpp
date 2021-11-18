#pragma once

#include <engine/utils/meshloader.hpp>
#include <engine/graphics/core/geometrybuffer.hpp>

namespace graphics {

class Mesh {
   public:
    Mesh(const utils::MeshData& _meshData);
    void draw() const;

   private:
    static const std::vector<VertexAttribute> attributes;
    GeometryBuffer geometryBuffer;
};
}  // namespace graphics
