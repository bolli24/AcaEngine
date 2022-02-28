#include <engine/graphics/renderer/mesh.hpp>

namespace graphics {

const std::vector<VertexAttribute> Mesh::attributes = {{PrimitiveFormat::FLOAT, 3},
                                                       {PrimitiveFormat::FLOAT, 2},
                                                       {PrimitiveFormat::FLOAT, 3}};

Mesh::Mesh(const utils::MeshData& _meshData)
    : meshData(&_meshData),
      geometryBuffer(GLPrimitiveType::TRIANGLES, attributes.data(), attributes.size(), 0) {
    std::vector<float> data;

    const auto& pos = _meshData.positions;
    const auto& uvs = _meshData.textureCoordinates;
    const auto& normals = _meshData.normals;

    for (const auto& face : _meshData.faces) {
        for (const auto& idx : face.indices) {
            data.push_back(pos[idx.positionIdx][0]);
            data.push_back(pos[idx.positionIdx][1]);
            data.push_back(pos[idx.positionIdx][2]);

            if (idx.textureCoordinateIdx.has_value()) {
                data.push_back(uvs[idx.textureCoordinateIdx.value()][0]);
                data.push_back(uvs[idx.textureCoordinateIdx.value()][1]);
            } else {
                data.push_back(0.f);
                data.push_back(0.f);
            }

            if (idx.normalIdx.has_value()) {
                data.push_back(normals[idx.normalIdx.value()][0]);
                data.push_back(normals[idx.normalIdx.value()][1]);
                data.push_back(normals[idx.normalIdx.value()][2]);
            } else {
                data.push_back(0.f);
                data.push_back(0.f);
                data.push_back(-1.f);
            }
        }
    }
    geometryBuffer.setData(data.data(), data.size() * sizeof(float));
}

void Mesh::draw() const {
    geometryBuffer.draw();
}

}  // namespace graphics