#include <math.h>

#include <algorithm>
#include <glm/glm.hpp>
#include <limits>
#include <numbers>
#include <stack>
#include <vector>

class ConvexHull {
   public:
    struct VertexAngle {
        glm::vec3 position;
        float polarAngle;
        float length;
    };

    struct MeshData {
        std::vector<glm::vec3> positions;
        std::vector<std::array<int, 3>> faces;
    };

    static MeshData getConvexHull(std::vector<glm::vec3> vertices) {
        MeshData outputMesh = createSimplex(vertices);

        return outputMesh;
    }

    static MeshData createSimplex(std::vector<glm::vec3>& vertices) {
        float max[3] = {INT_MIN, INT_MIN, INT_MIN};
        float min[3] = {INT_MAX, INT_MAX, INT_MAX};

        int extremeVertexIndices[6];

        for (int i = 0; i < vertices.size(); i++) {
            for (int j = 0; j < 3; j++) {
                if (max[j] < vertices[i][j]) {
                    max[j] = vertices[i][j];
                    extremeVertexIndices[2 * j] = i;
                }
                if (min[j] > vertices[i][j]) {
                    min[j] = vertices[i][j];
                    extremeVertexIndices[2 * j + 1] = i;
                }
            }
        }

        for (int i = 0; i < 3; i++)
            if (min[i] == max[i]) throw std::exception("Polyhedron is not 3 dimensional");

        float maxDistance = INT_MIN;

        int P1, P2;

        for (int i = 0; i < 5; i++) {
            glm::vec3& vertex = vertices[extremeVertexIndices[i]];

            for (int j = i + 1; j < 6; j++) {
                float d = glm::distance(vertex, vertices[extremeVertexIndices[j]]);

                if (d > maxDistance) {
                    P1 = extremeVertexIndices[i];
                    P2 = extremeVertexIndices[j];
                    maxDistance = d;
                }
            }
        }

        maxDistance = INT_MIN;
        int P3;

        for (int i = 0; i < 6; i++) {
            float d = distanceFromLine(vertices[P1], vertices[P2], vertices[extremeVertexIndices[i]]);
            if (d > maxDistance) {
                maxDistance = d;
                P3 = extremeVertexIndices[i];
            }
        }

        if (maxDistance == 0) throw std::exception("Polyhedron is not 3 dimensional");

        maxDistance = 0;
        int P4;

        for (int i = 0; i < vertices.size(); i++) {
            float d = distanceFromFace(vertices[P1], vertices[P2], vertices[P3], vertices[i]);

            if (abs(d) > abs(maxDistance)) {
                maxDistance = d;
                P4 = i;
            }
        }

        if (maxDistance == 0) throw std::exception("Polyhedron is not 3 dimensional");

        MeshData mesh;

        mesh.positions = {vertices[P1], vertices[P2], vertices[P3], vertices[P4]};

        if (maxDistance > 0) {
            mesh.faces.push_back({0, 2, 1});
        } else {
            mesh.faces.push_back({0, 1, 2});
        }

        mesh.faces.push_back({0, 1, 3});
        mesh.faces.push_back({0, 3, 2});
        mesh.faces.push_back({1, 2, 3});

        return mesh;
    }

   private:
    // Reference: https://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
    static float distanceFromLine(glm::vec3 a, glm::vec3 b, glm::vec3 x) {
        return glm::length(glm::cross(b - a, a - x)) / glm::length(b - a);
    }

    // Reference: https://mathworld.wolfram.com/Point-PlaneDistance.html
    static float distanceFromFace(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 x) {
        glm::vec3 normal = glm::cross(b - a, c - a);
        glm::vec3 unitNormal = normal / glm::length(normal);
        return glm::dot(unitNormal, (x - a));
    }
};