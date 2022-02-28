#include <engine/game/collisionsystem.hpp>

std::unordered_map<utils::MeshData::Handle, ConvexMesh> CollisionSystem::convexHulls;