#pragma once

#include <math.h>

#include <algorithm>
#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <numbers>
#include <stack>
#include <unordered_map>
#include <vector>

struct Face;

struct Edge {
    std::pair<uint64_t, uint64_t> idx;
    std::array<std::shared_ptr<Face>, 2> faces;
    bool onConvexHull = true;
};

struct Face {
    std::array<uint64_t, 3> indexVertices;
    std::array<std::weak_ptr<Edge>, 3> edges;
    std::vector<glm::vec3> outsideSet;
    bool onConvexHull = true;
};

class Hash {
   public:
    size_t operator()(const Edge& edge) const {
        return static_cast<size_t>(edge.idx.first) + static_cast<size_t>(edge.idx.second);
    }
};

struct ConvexMesh {
    struct Face {
        std::array<uint64_t, 3> vertexIndices;
        glm::vec3 normal;
    };

    std::vector<glm::vec3> positions;
    std::vector<ConvexMesh::Face> faces;
    glm::vec3 center = glm::vec3(0);
};

struct MeshData {
    std::vector<glm::vec3> positions;
    std::vector<std::shared_ptr<Face>> faces;
    std::unordered_map<Edge, std::shared_ptr<Edge>, Hash> edges;
};

inline bool operator==(const Face& a, const Face& b) {
    return a.indexVertices == b.indexVertices;
}

inline bool operator<(const Edge& a, const Edge& b) {
    return a.idx.first < b.idx.first && a.idx.second < b.idx.second;
}

inline bool operator==(const Edge& a, const Edge& b) {
    return (b.idx.first == a.idx.first && b.idx.second == a.idx.second) ||
           (b.idx.first == a.idx.second && b.idx.second == a.idx.first);
}

typedef glm::vec<3, float, glm::packed_highp>::length_type lengthType;

// Reference: http://algolist.ru/maths/geom/convhull/qhull3d.php
class ConvexHull {
   public:
    static ConvexMesh getConvexHull(std::vector<glm::vec3> vertices) {
        std::vector<Face> remainingFaces;
        MeshData mesh = createSimplex(vertices);

        for (auto& face : mesh.faces) {
            addToOutsideSet(mesh, *face, vertices);
        }

        vertices.clear();

        std::shared_ptr<Face> currentFace;

        for (const auto& face : mesh.faces) {
            if (!face->outsideSet.empty()) {
                currentFace = face;
                break;
            }
        }

        bool loop = true;
        while (loop) {
            float maxDistance = INT_MIN;
            glm::vec3 eyePoint;

            for (glm::vec3& point : currentFace->outsideSet) {
                float d = distanceFromFace(getPosFromIndices(*currentFace, mesh.positions), point);
                if (d > maxDistance) {
                    maxDistance = d;
                    eyePoint = point;
                }
            }

            mesh.positions.push_back(eyePoint);
            uint64_t eyePointIndex = mesh.positions.size() - 1;
            std::vector<Face> visibleFaces = getVisibleFaces(eyePoint, mesh);
            std::vector<Edge> horizonEdges = calculateHorizon(mesh, visibleFaces, vertices);

            auto end = horizonEdges.end();
            for (auto it = horizonEdges.begin(); it != end; ++it) {
                end = std::remove(it + 1, end, *it);
            }
            horizonEdges.erase(end, horizonEdges.end());

            // Richtige Reihenfolge der ersten Kante
            Edge& firstEdge = horizonEdges.back();
            std::shared_ptr<Face> notConvexHullFace = !firstEdge.faces[0]->onConvexHull ? firstEdge.faces[0] : firstEdge.faces[1];
            std::array<uint64_t, 3> idx = notConvexHullFace->indexVertices;
            if ((idx[0] == firstEdge.idx.first || idx[0] == firstEdge.idx.second) && (idx[1] == firstEdge.idx.first || idx[1] == firstEdge.idx.second))
                firstEdge.idx = {idx[0], idx[1]};
            else if ((idx[1] == firstEdge.idx.first || idx[1] == firstEdge.idx.second) && (idx[2] == firstEdge.idx.first || idx[2] == firstEdge.idx.second))
                firstEdge.idx = {idx[1], idx[2]};
            else
                firstEdge.idx = {idx[2], idx[0]};

            std::vector<Edge> sortedHorizonEdges = {horizonEdges.back()};

            while (sortedHorizonEdges.size() <= horizonEdges.size() - 1) {
                for (int i = 0; i < horizonEdges.size() - 1; i++) {
                    if (horizonEdges[i].idx.first == sortedHorizonEdges[sortedHorizonEdges.size() - 1].idx.second) {
                        sortedHorizonEdges.push_back(horizonEdges[i]);
                        break;
                    } else if (horizonEdges[i].idx.second == sortedHorizonEdges[sortedHorizonEdges.size() - 1].idx.second &&
                               horizonEdges[i].idx.first != sortedHorizonEdges[sortedHorizonEdges.size() - 1].idx.first) {
                        uint64_t first = horizonEdges[i].idx.first;
                        horizonEdges[i].idx.first = horizonEdges[i].idx.second;
                        horizonEdges[i].idx.second = first;
                        sortedHorizonEdges.push_back(horizonEdges[i]);
                        break;
                    }
                }
            }  // E - K + F = 2

            std::shared_ptr<Edge> lastEdge = std::make_shared<Edge>(Edge{{eyePointIndex, sortedHorizonEdges[0].idx.first}});
            std::shared_ptr<Edge> currentEdge = lastEdge;

            // Skip first and last
            for (int i = 0; i < sortedHorizonEdges.size() - 1; i++) {
                Edge& edge = sortedHorizonEdges[i];
                // std::shared_ptr<Edge> newEdge0 = std::make_shared<Edge>(edge);
                std::shared_ptr<Edge> oldEdge = mesh.edges[edge];
                std::shared_ptr<Edge> newEdge = std::make_shared<Edge>(Edge{{edge.idx.second, eyePointIndex}});

                std::shared_ptr<Face> newFace = std::make_shared<Face>(Face{{edge.idx.first, edge.idx.second, eyePointIndex},
                                                                            {oldEdge, newEdge, currentEdge}});

                if (!oldEdge->faces[0]->onConvexHull) {
                    oldEdge->faces[0] = newFace;
                } else {
                    oldEdge->faces[1] = newFace;
                }

                newEdge->faces[0] = newFace;
                currentEdge->faces[1] = newFace;

                mesh.edges[*newEdge] = newEdge;
                mesh.edges[*currentEdge] = currentEdge;

                mesh.faces.push_back(newFace);

                currentEdge = newEdge;

                addToOutsideSet(mesh, *newFace, vertices);
            }
            std::shared_ptr<Edge> oldEdge = mesh.edges[sortedHorizonEdges.back()];
            std::shared_ptr<Face> lastFace = std::make_shared<Face>(Face{{sortedHorizonEdges.back().idx.first, sortedHorizonEdges.back().idx.second, eyePointIndex},
                                                                         {oldEdge, lastEdge, currentEdge}});

            if (!oldEdge->faces[0]->onConvexHull) {
                oldEdge->faces[0] = lastFace;
            } else {
                oldEdge->faces[1] = lastFace;
            }
            lastEdge->faces[0] = lastFace;
            currentEdge->faces[1] = lastFace;
            mesh.faces.push_back(lastFace);
            addToOutsideSet(mesh, *lastFace, vertices);

            vertices.clear();
            currentFace->outsideSet.clear();

            mesh.faces.erase(std::remove_if(mesh.faces.begin(), mesh.faces.end(), [](std::shared_ptr<Face>& face) {
                                 return !face->onConvexHull;
                             }),
                             mesh.faces.end());

            for (auto it = mesh.edges.begin(); it != mesh.edges.end();) {
                if (!it->second->onConvexHull)
                    it = mesh.edges.erase(it);
                else
                    it++;
            }

            loop = false;
            for (const auto& face : mesh.faces) {
                if (!face->outsideSet.empty()) {
                    currentFace = face;
                    loop = true;
                    break;
                }
            }
        }

        ConvexMesh convexMesh = {mesh.positions};
        for (auto& face : mesh.faces) {
            convexMesh.faces.push_back(ConvexMesh::Face{face->indexVertices, getFaceNormal(getPosFromIndices(*face, mesh.positions))});
        }

        glm::vec3 sum(0);
        for (glm::vec3 pos : mesh.positions)
            sum += pos;

        sum /= mesh.positions.size();
        convexMesh.center = sum;

        return convexMesh;
    }

    // Erstelle ersten Polyeder aus 4 Punkten, 4 Flächen, 6 Kanten
    static MeshData createSimplex(std::vector<glm::vec3>& vertices) {
        std::array<int, 6> extremeVertexIndices = {};
        std::array<float, 6> maxMin = {std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min(),
                                       std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};

        for (lengthType i = 0; i < vertices.size(); i++) {
            for (lengthType j = 0; j < 3; j++) {
                if (maxMin[(size_t)j * 2] < vertices[i][j]) {
                    maxMin[(size_t)j * 2] = vertices[i][j];
                    extremeVertexIndices[(size_t)2 * j] = i;
                }
                if (maxMin[(size_t)j * 2 + 1] > vertices[i][j]) {
                    maxMin[(size_t)j * 2 + 1] = vertices[i][j];
                    extremeVertexIndices[(size_t)2 * j + 1] = i;
                }
            }
        }

        for (size_t i = 0; i < 3; i++)
            if (extremeVertexIndices[2 * i] == extremeVertexIndices[2 * i + 1]) throw std::runtime_error("Polyhedron is not 3 dimensional");

        float maxDistance = INT_MIN;

        int P1, P2;

        for (int i = 0; i < 5; i++) {
            const glm::vec3& vertex = vertices[extremeVertexIndices[i]];

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
            const float d = distanceFromLine(vertices[P1], vertices[P2], vertices[extremeVertexIndices[i]]);
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

        std::shared_ptr<Edge> edge1 = std::make_shared<Edge>(Edge{{0, 1}});
        std::shared_ptr<Edge> edge2 = std::make_shared<Edge>(Edge{{0, 2}});
        std::shared_ptr<Edge> edge3 = std::make_shared<Edge>(Edge{{0, 3}});
        std::shared_ptr<Edge> edge4 = std::make_shared<Edge>(Edge{{1, 2}});
        std::shared_ptr<Edge> edge5 = std::make_shared<Edge>(Edge{{1, 3}});
        std::shared_ptr<Edge> edge6 = std::make_shared<Edge>(Edge{{2, 3}});

        std::shared_ptr<Face> face1;
        std::shared_ptr<Face> face2;
        std::shared_ptr<Face> face3;
        std::shared_ptr<Face> face4;

        if (maxDistance > 0) {
            face1 = std::make_shared<Face>(Face{{0, 2, 1}, {edge2, edge4, edge1}});
            face2 = std::make_shared<Face>(Face{{0, 1, 3}, {edge1, edge5, edge3}});
            face3 = std::make_shared<Face>(Face{{0, 3, 2}, {edge3, edge6, edge2}});
            face4 = std::make_shared<Face>(Face{{1, 2, 3}, {edge4, edge6, edge5}});
        } else {
            face1 = std::make_shared<Face>(Face{{0, 1, 2}, {edge1, edge4, edge2}});
            face2 = std::make_shared<Face>(Face{{0, 3, 1}, {edge3, edge5, edge1}});
            face3 = std::make_shared<Face>(Face{{0, 2, 3}, {edge2, edge6, edge3}});
            face4 = std::make_shared<Face>(Face{{1, 3, 2}, {edge5, edge6, edge4}});
        }

        edge1->faces = {face1, face2};
        edge2->faces = {face1, face3};
        edge3->faces = {face2, face3};
        edge4->faces = {face1, face4};
        edge5->faces = {face2, face4};
        edge6->faces = {face3, face4};

        mesh.faces.push_back(face1);
        mesh.faces.push_back(face2);
        mesh.faces.push_back(face3);
        mesh.faces.push_back(face4);

        mesh.edges[*edge1] = edge1;
        mesh.edges[*edge2] = edge2;
        mesh.edges[*edge3] = edge3;
        mesh.edges[*edge4] = edge4;
        mesh.edges[*edge5] = edge5;
        mesh.edges[*edge6] = edge6;

        /* for (Face& face : mesh.faces) {
            face.edges = {Edge{{face.indexVertices[0], face.indexVertices[1]}},
                          Edge{{face.indexVertices[1], face.indexVertices[2]}},
                          Edge{{face.indexVertices[2], face.indexVertices[0]}}};
            mesh.edges[face.edges[0]];
            mesh.edges[face.edges[1]];
            mesh.edges[face.edges[2]];
        }*/

        vertices.erase(vertices.begin() + P1);
        vertices.erase(vertices.begin() + P2);
        vertices.erase(vertices.begin() + P3);
        vertices.erase(vertices.begin() + P4);

        return mesh;
    }

    // Reference: https://mathworld.wolfram.com/Point-PlaneDistance.html
    static float distanceFromFace(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c, const glm::vec3& x) {
        glm::vec3 normal = glm::cross(b - a, c - a);
        glm::vec3 unitNormal = normal / glm::length(normal);
        return glm::dot(unitNormal, (x - a));
    }

    static float distanceFromFace(const std::array<glm::vec3, 3>& fpos, const glm::vec3& x) {
        return distanceFromFace(fpos[0], fpos[1], fpos[2], x);
    }

    static glm::vec3 getFaceNormal(const std::array<glm::vec3, 3>& fpos) {
        glm::vec3 normal = glm::cross(fpos[1] - fpos[0], fpos[2] - fpos[0]);
        return (normal / glm::length(normal));
    }

   private:
    // Alle Edges onConvexHull = false, die nicht horizonEdges sind
    static std::vector<Edge> calculateHorizon(const MeshData& mesh, std::vector<Face>& visibleFaces, std::vector<glm::vec3>& vertices) {
        std::vector<Edge> horizonEdges;
        for (Face& face : visibleFaces) {
            vertices.insert(vertices.end(), face.outsideSet.begin(), face.outsideSet.end());
            face.outsideSet.clear();

            for (auto& edge : face.edges) {
                if (edge.lock()->faces[0]->onConvexHull != edge.lock()->faces[1]->onConvexHull)
                    horizonEdges.push_back(*edge.lock());
                else
                    edge.lock()->onConvexHull = false;
            }
        }

        return horizonEdges;
    }

    // Alle Faces onConvexHull = false, die sichtbar sind
    static std::vector<Face> getVisibleFaces(glm::vec3& eyePoint, MeshData& mesh) {
        std::vector<Face> visibleFaces;
        for (std::shared_ptr<Face> face : mesh.faces) {
            std::array<glm::vec3, 3> facePos = getPosFromIndices(*face, mesh.positions);
            if (isFaceVisible(facePos, eyePoint)) {
                visibleFaces.push_back(*face);
                face->onConvexHull = false;
            }
        }
        Face v = visibleFaces[0];
        return visibleFaces;
    }

    // Alle Punkte Außerhalb zu einem OutsideSet eines Faces hinzufügen
    static void addToOutsideSet(const MeshData& mesh, Face& face, std::vector<glm::vec3>& vertices) {
        std::stack<int> deleteIndices;

        for (int i = 0; i < vertices.size(); i++) {
            float d = distanceFromFace(mesh.positions[face.indexVertices[0]],
                                       mesh.positions[face.indexVertices[1]],
                                       mesh.positions[face.indexVertices[2]],
                                       vertices[i]);

            if (d > 0.0001f) {  // outside of polyhedron
                face.outsideSet.push_back(vertices[i]);
                deleteIndices.push(i);
            }
        }

        while (!deleteIndices.empty()) {
            vertices.erase(vertices.begin() + deleteIndices.top());
            deleteIndices.pop();
        }
    }

    static std::array<glm::vec3, 3> getPosFromIndices(const Face& face, const std::vector<glm::vec3>& vertices) {
        return {vertices[face.indexVertices[0]], vertices[face.indexVertices[1]], vertices[face.indexVertices[2]]};
    }

    static bool isFaceVisible(const std::array<glm::vec3, 3>& face, const glm::vec3& eyePoint) {
        float distance = distanceFromFace(face[0], face[1], face[2], eyePoint);
        return distance > 0.0001f;
    }

    // Reference: https://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
    static float distanceFromLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& x) {
        return glm::length(glm::cross(b - a, a - x)) / glm::length(b - a);
    }
};