#pragma once

#include "../utils/assert.hpp"

#include <glm/glm.hpp>
#include <cstdint>

namespace math {

	// predeclaration for the Box constructor
	template<unsigned Dim, typename FloatT>
	struct HyperSphere;

	template<unsigned Dim, typename FloatT>
	struct Box
	{
		using VecT = glm::vec<Dim, FloatT, glm::defaultp>;

		VecT min;
		VecT max;

		/// \brief Create uninitialized box.
		Box() noexcept {}

		/// \brief Construct from minimal and maximal coordinates.
		Box(const VecT& _min, const VecT& _max) noexcept
			: min(_min), max(_max) 
		{
			ASSERT(_min == glm::min(min, max) && _max == glm::max(min, max),
				"Minimum coordinates must be smaller or equal the maximum.");
		}

		/// \brief Create an optimal box for a set of points.
		Box(const VecT* _points, uint32_t _numPoints) noexcept
		{
			ASSERT(_points && _numPoints > 0, "The point list must have at least one point.");
			min = max = *_points++;
			for (uint32_t i = 1; i < _numPoints; ++i, ++_points)
			{
				min = glm::min(min, *_points);
				max = glm::max(max, *_points);
			}
		}

		/// \brief Create a minimal bounding box for a Sphere.
		explicit Box(const HyperSphere<Dim, FloatT>& _sphere) noexcept
			: min(_sphere.center - VecT(_sphere.radius)),
			max(_sphere.center + VecT(_sphere.radius))
		{
		}

		VecT size() const noexcept
		{
			return max - min;
		}

		// Intersection check with another Box.
		// Matching lines are considered intersecting.
		bool intersect(const Box& oth) const
		{
			for (int i = 0; i < Dim; ++i)
				if (min[i] > oth.max[i] || max[i] < oth.min[i])
					return false;

			return true;
		}

		/// \brief Check whether a point is inside the box.
		bool isIn(const VecT& _point) const
		{
			for (int i = 0; i < Dim; ++i)
				if (min[i] > _point[i] || max[i] < _point[i])
					return false;

			return true;
		}

		bool operator==(const Box& oth) const { return min == oth.min && max == oth.max; }
		bool operator!=(const Box& oth) const { return min != oth.min || max != oth.max; }
	};

	// common box types
	using Rectangle = Box<2, float>;

	// Axis aligned bounding boxes
	template<unsigned Dim, typename FloatT = float>
	using AABB = Box<Dim, FloatT>;
	using AABB2D = AABB<2>;

	template<unsigned Dim, typename FloatT>
	struct HyperSphere
	{
		using VecT = glm::vec<Dim, FloatT, glm::defaultp>;

		VecT center;
		FloatT radius;

		HyperSphere(const VecT& _center, FloatT _radius) noexcept
			: center(_center), radius(_radius) {}

		// Construct the largest sphere which fits inside _box.
		explicit HyperSphere(const Box<Dim, FloatT>& _box) noexcept
			: center((_box.min + _box.max) * 0.5f),
			radius(std::numeric_limits<FloatT>::max())
		{
			const VecT dif = _box.max - _box.min;
			for (unsigned i = 0; i < Dim; ++i)
				if (radius > dif[i]) radius = dif[i];
		}
	};

	using Circle = HyperSphere<2, float>;

	template<unsigned Dim, typename FloatT>
	struct Ray
	{
		using VecT = glm::vec<Dim, FloatT, glm::defaultp>;

		VecT origin;
		VecT direction;

		Ray(const VecT& _origin, const VecT& _direction) noexcept
			: origin(_origin), direction(_direction) {}
	};

	using Ray2D = Ray<2, float>;
}