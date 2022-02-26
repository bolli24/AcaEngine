#include <math.h>

#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <numbers>
#include <stack>
#include <unordered_map>
#include <vector>

struct Face {
    std::array<int, 3> indexVertices;
    std::vector<glm::vec3> outsideSet;
    bool onConvexHull = true;
};

struct Edge {
    std::pair<int, int> idx;
};

struct MeshData {
    std::vector<glm::vec3> positions;
    std::vector<Face> faces = {};
};

inline bool operator==(const Face& a, const Face& b) {
    return a.indexVertices == b.indexVertices;
}

inline bool operator==(const Edge& a, const Edge& b) {
    return b.idx.first == a.idx.first && b.idx.second == a.idx.first;
}

class Hash {
   public:
    size_t operator()(const Edge& edge) const {
        return edge.idx.first + edge.idx.second * edge.idx.second;
    }

    size_t operator()(const Face& face) const {
        return face.indexVertices[0] + face.indexVertices[1] * face.indexVertices[2];
    }
};

// Reference: http://algolist.ru/maths/geom/convhull/qhull3d.php
class ConvexHull {
   public:
    static MeshData getConvexHull(std::vector<glm::vec3> vertices) {
        std::vector<Face> remainingFaces;
        MeshData outputMesh = createSimplex(vertices, remainingFaces);

        for (auto& face : remainingFaces) {
            addToOutsideSet(outputMesh, face, vertices);
        }

        vertices.clear();

        bool loop = true;
        while (loop) {
            Face currentFace = remainingFaces[0];

            float maxDistance = INT_MIN;
            glm::vec3 eyePoint;

            auto a = currentFace.outsideSet;

            for (glm::vec3& point : currentFace.outsideSet) {
                float d = distanceFromFace(getPosFromIndices(currentFace, outputMesh.positions), point);
                if (d > maxDistance) {
                    maxDistance = d;
                    eyePoint = point;
                }
            }

            std::unordered_map<Edge, bool, Hash> horizonEdges;
            calculateHorizon(vertices, eyePoint, {}, 0, horizonEdges, outputMesh, remainingFaces);

            outputMesh.positions.push_back(eyePoint);
            const int eyePointIndex = outputMesh.positions.size() - 1;

            for (const std::pair<const Edge, bool>& edge : horizonEdges) {
                Face newFace = {{eyePointIndex, edge.first.idx.first, edge.first.idx.second}};
                remainingFaces.push_back(newFace);
                remainingFaces[remainingFaces.size() - 1];
                addToOutsideSet(outputMesh, newFace, vertices);
            }

            vertices.clear();

            loop = false;
            for (const auto& face : remainingFaces) {
                if (!face.outsideSet.empty()) {
                    loop = true;
                    break;
                }
            }
        }

        for (const auto& face : remainingFaces) {
            outputMesh.faces.push_back(face);
        }

        std::vector<glm::ivec3> faces;

        for (Face& face : outputMesh.faces) {
            faces.push_back({face.indexVertices[0] + 1, face.indexVertices[1] + 1, face.indexVertices[2] + 1});
        }

        return outputMesh;
    }

    static MeshData createSimplex(std::vector<glm::vec3>& vertices, std::vector<Face>& remainingFaces) {
        std::array<int, 6> extremeVertexIndices;

        for (int i = 0; i < 3; i++) {
            const auto [min, max] = std::minmax_element(vertices.begin(), vertices.end(),
                                                        [&](glm::vec3& a, glm::vec3& b) { return a[i] < b[i]; });
            extremeVertexIndices[2 * i] = std::distance(vertices.begin(), min);
            extremeVertexIndices[2 * i + 1] = std::distance(vertices.begin(), max);
        }

        for (int i = 0; i < 3; i++)
            if (extremeVertexIndices[2 * i] == extremeVertexIndices[2 * i + 1]) throw std::runtime_error("Polyhedron is not 3 dimensional");

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

        if (maxDistance == 0) throw std::runtime_error("Polyhedron is not 3 dimensional");

        maxDistance = 0;
        int P4;

        for (int i = 0; i < vertices.size(); i++) {
            float d = distanceFromFace(vertices[P1], vertices[P2], vertices[P3], vertices[i]);

            if (abs(d) > abs(maxDistance)) {
                maxDistance = d;
                P4 = i;
            }
        }

        if (maxDistance == 0) throw std::runtime_error("Polyhedron is not 3 dimensional");

        MeshData mesh;

        mesh.positions = {vertices[P1], vertices[P2], vertices[P3], vertices[P4]};

        if (maxDistance > 0) {
            remainingFaces.push_back({{0, 2, 1}});
            remainingFaces.push_back({{0, 1, 3}});
            remainingFaces.push_back({{0, 3, 2}});
            remainingFaces.push_back({{1, 2, 3}});
        } else {
            remainingFaces.push_back({{0, 1, 2}});
            remainingFaces.push_back({{0, 3, 1}});
            remainingFaces.push_back({{0, 2, 3}});
            remainingFaces.push_back({{1, 3, 2}});
        }

        int a = remainingFaces.size();

        vertices.erase(vertices.begin() + P1);
        vertices.erase(vertices.begin() + P2);
        vertices.erase(vertices.begin() + P3);
        vertices.erase(vertices.begin() + P4);

        return mesh;
    }

   private:
    static void calculateHorizon(std::vector<glm::vec3>& unclaimedVertices, glm::vec3& eyePoint, std::optional<Edge> crossedEdge,
                                 int currentFaceIndex, std::unordered_map<Edge, bool, Hash>& horizonEdges, MeshData& convexHull,
                                 std::vector<Face>& remainingFaces) {
        Face& currentFace = remainingFaces[currentFaceIndex];

        if (!currentFace.onConvexHull) {  // current face not on convex hull = outside set not empty

            horizonEdges[crossedEdge.value()] = false;
            return;
        }

        std::array<glm::vec3, 3> currentFacePos = getPosFromIndices(currentFace, convexHull.positions);

        const auto& idx = currentFace.indexVertices;

        if (isFaceVisible(currentFacePos, eyePoint)) {
            currentFace.onConvexHull = false;
            unclaimedVertices.insert(unclaimedVertices.end(), currentFace.outsideSet.begin(), currentFace.outsideSet.end());
            currentFace.outsideSet.clear();
           
            if (crossedEdge.has_value()) {  // is not first face
                horizonEdges[crossedEdge.value()] = false;
            }

            if (horizonEdges.find(Edge{{idx[0], idx[1]}}) == horizonEdges.end())
                horizonEdges[Edge{{idx[0], idx[1]}}] = true;
            if (horizonEdges.find(Edge{{idx[1], idx[2]}}) == horizonEdges.end())
                horizonEdges[Edge{{idx[1], idx[2]}}] = true;
            if (horizonEdges.find(Edge{{idx[2], idx[0]}}) == horizonEdges.end())
                horizonEdges[Edge{{idx[2], idx[0]}}] = true;

            std::optional<Edge> nextEdge = getNextEdge(crossedEdge, currentFace, horizonEdges);

            while (nextEdge.has_value()) {
                auto b = nextEdge;
                std::optional<Face> nextFace = getNextFace(nextEdge.value(), currentFace, convexHull, remainingFaces, eyePoint);
                auto c = nextFace;
                if (nextFace.has_value()) {
                    remainingFaces.push_back(nextFace.value());
                    calculateHorizon(unclaimedVertices, eyePoint, nextEdge, remainingFaces.size() - 1, horizonEdges, convexHull, remainingFaces);
                }
                nextEdge = getNextEdge(nextEdge, currentFace, horizonEdges);
            }

            // remainingFaces.erase(remainingFaces.begin() + currentFaceIndex);

            // for each remaining edge on currentFace calculateHorizon(...) in counter clockwise order;
        }

        /*      Remove all vertices from the currFace's outside set, and add them to the listUnclaimedVertices.
            c.  If the crossedEdge != NULL (only for the first face) then mark the crossedEdge as not on the convex hull
            d.  Cross each of the edges of currFace which are still on the convex hull in counterclockwise order
                starting from the edge after the crossedEdge (in the case of the first face, pick any edge to start with).
                For each currEdge recurse with the call. */
    }

    static std::optional<Face> getNextFace(Edge& edge, Face& currentFace, MeshData& convexHull, std::vector<Face>& remainingFaces, glm::vec3& eyePoint) {
        for (Face& face : remainingFaces) {
            std::array<glm::vec3, 3> currentFacePos = getPosFromIndices(currentFace, convexHull.positions);
            bool visible = isFaceVisible(currentFacePos, eyePoint);
            if (currentFace.indexVertices == face.indexVertices && !visible) continue;

            int matches = 0;

            for (int i : currentFace.indexVertices) {
                if (i == edge.idx.first || i == edge.idx.second)
                    matches++;
            }

            if (matches == 2) return face;
        }

        return {};
    }

    static std::optional<Edge> getNextEdge(std::optional<Edge>& currentEdge, Face& currentFace, std::unordered_map<Edge, bool, Hash>& horizonEdges) {
        auto a = currentFace.indexVertices;

        if (!currentEdge.has_value())
            return Edge{{currentFace.indexVertices[0], currentFace.indexVertices[1]}};

        if (currentEdge.value().idx.first == currentFace.indexVertices[0]) {  // currentEdge = 0 - 1
            Edge newEdge = {{currentFace.indexVertices[1], currentFace.indexVertices[2]}};
            if (horizonEdges[newEdge]) return newEdge;  // Edge 1 - 2
            newEdge = {{currentFace.indexVertices[2], currentFace.indexVertices[0]}};
            if (horizonEdges[newEdge]) return newEdge;  // Edge 2 - 0
            return {};
        }

        if (currentEdge.value().idx.first == currentFace.indexVertices[1]) {  // currentEdge = 1 - 2
            Edge newEdge = {{currentFace.indexVertices[2], currentFace.indexVertices[0]}};
            if (horizonEdges[newEdge]) return newEdge;  // Edge 2 - 0
            newEdge = {{currentFace.indexVertices[0], currentFace.indexVertices[1]}};
            if (horizonEdges[newEdge]) return newEdge;  // Edge 0 - 1
            return {};
        }

        if (currentEdge.value().idx.first == currentFace.indexVertices[2]) {  // currentEdge = 2 - 0
            Edge newEdge = {{currentFace.indexVertices[0], currentFace.indexVertices[1]}};
            if (horizonEdges[newEdge]) return newEdge;  // Edge 0 - 1
            newEdge = {{currentFace.indexVertices[1], currentFace.indexVertices[2]}};
            if (horizonEdges[newEdge]) return newEdge;  // Edge 1 - 2
            return {};
        }
    }

    static void
    addToOutsideSet(MeshData& mesh, Face& face, std::vector<glm::vec3>& vertices) {
        std::stack<int> deleteIndices;

        for (int i = 0; i < vertices.size(); i++) {
            float d = distanceFromFace(mesh.positions[face.indexVertices[0]],
                                       mesh.positions[face.indexVertices[1]],
                                       mesh.positions[face.indexVertices[2]],
                                       vertices[i]);

            if (d > 0) {  // outside of polyhedron
                face.outsideSet.push_back(vertices[i]);
                deleteIndices.push(i);
            }
        }

        while (!deleteIndices.empty()) {
            vertices.erase(vertices.begin() + deleteIndices.top());
            deleteIndices.pop();
        }
    }

    static std::array<glm::vec3, 3> getPosFromIndices(Face& face, const std::vector<glm::vec3>& vertices) {
        auto a = face;
        return {vertices[face.indexVertices[0]], vertices[face.indexVertices[1]], vertices[face.indexVertices[2]]};
    }

    static bool isFaceVisible(std::array<glm::vec3, 3>& face, glm::vec3& eyePoint) {
        float distance = distanceFromFace(face[0], face[1], face[2], eyePoint);
        return distance > 0;
    }

    // Reference: https://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
    static float
    distanceFromLine(glm::vec3& a, glm::vec3& b, glm::vec3& x) {
        return glm::length(glm::cross(b - a, a - x)) / glm::length(b - a);
    }

    // Reference: https://mathworld.wolfram.com/Point-PlaneDistance.html
    static float distanceFromFace(glm::vec3& a, glm::vec3& b, glm::vec3& c, glm::vec3& x) {
        glm::vec3 normal = glm::cross(b - a, c - a);
        glm::vec3 unitNormal = normal / glm::length(normal);
        return glm::dot(unitNormal, (x - a));
    }

    static float distanceFromFace(std::array<glm::vec3, 3> fpos, glm::vec3& x) {
        return distanceFromFace(fpos[0], fpos[1], fpos[2], x);
    }
};