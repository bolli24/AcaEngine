#pragma once

#include <glm/glm.hpp>
#include <optional>

namespace math {

	// intersection of two line segments p1-p2 and q1-q2.
	template<typename T>
	constexpr std::optional<glm::vec<2,T,glm::defaultp>> intersect(
		const glm::vec<2, T, glm::defaultp>& p1,
		const glm::vec<2, T, glm::defaultp>& p2, 
		const glm::vec<2, T, glm::defaultp>& q1, 
		const glm::vec<2, T, glm::defaultp>& q2, 
		T eps = 0)
	{
		const T s1_x = p2.x - p1.x;
		const T s1_y = p2.y - p1.y;
		const T s2_x = q2.x - q1.x;
		const T s2_y = q2.y - q1.y;
		const T denom = (-s2_x * s1_y + s1_x * s2_y);
		if (denom == 0.f) return {};

		const T s = (-s1_y * (p1.x - q1.x) + s1_x * (p1.y - q1.y)) / denom;
		const T t = (s2_x * (p1.y - q1.y) - s2_y * (p1.x - q1.x)) / denom;

		if (s - eps >= 0 && s + eps <= 1 && t - eps >= 0 && t + eps <= 1)
		{
			return p1 + t * glm::vec<2, T, glm::defaultp>(s1_x, s1_y);
		}

		return {};
	}
}