#pragma once

#include <glm/vec3.hpp>

#include <vector>

namespace utils {
	/// triangulate a polygone with the earcut algorithm
	/** \return triangulation as indices of _polygone */
	std::vector<int> triangulateEarCut(const std::vector<glm::vec3>& polygone);
} // namespace utils
