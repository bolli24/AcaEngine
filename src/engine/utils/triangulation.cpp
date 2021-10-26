#include "./triangulation.hpp"


#include <glm/mat3x3.hpp>
#include <glm/vec2.hpp>
#include <glm/geometric.hpp>

#include <tuple>
#include <optional>
#include <array>
#include <cassert>

enum Orientation : char {NONE = 0b00, POSITIVE = 0b01, NEGATIVE = 0b10, STRAIGHT = 0b11};
enum Direction {CURRENT, NEXT, PREVIOUS };

/// calculate orientation factor of three following points
float orientationF(
		const glm::vec2& a,
		const glm::vec2& b,
		const glm::vec2& c)
{
	return ((b.x*c.y + a.x*b.y + a.y*c.x) - (a.y*b.x + b.y*c.x + a.x*c.y));
}

/// returns Orientation value
Orientation orientation(
		const glm::vec2& a,
		const glm::vec2& b,
		const glm::vec2& c)
{
	float v = orientationF(a, b, c);
	return 	static_cast<Orientation>(
				(v <= 0 ? Orientation::NEGATIVE : Orientation::NONE)
			| 	(v >= 0 ? Orientation::POSITIVE : Orientation::NONE));
}

/// checks if p is inside triangle a,b,c
bool isIn(
		const glm::vec2& p,
		const glm::vec2& a,
		const glm::vec2& b,
		const glm::vec2& c)
{
	Orientation orient[] = {
		orientation(a,b,p),
		orientation(b,c,p),
		orientation(c,a,p)
	};
	return (orient[0] & Orientation::POSITIVE && orient[1] & Orientation::POSITIVE && orient[2] & Orientation::POSITIVE)
		|| (orient[0] & Orientation::NEGATIVE && orient[1] & Orientation::NEGATIVE && orient[2] & Orientation::NEGATIVE);
}

/// get next valid id in relation of a valid id
int getId (
		std::vector<std::optional<glm::vec2>>& data,
		int id,
		Direction dir = Direction::CURRENT) {
	if(dir == Direction::PREVIOUS)
	{
		const std::optional<glm::vec2>* res;
		do {
		res = id == 0 			// underflow check
			? &data[id = data.size() - 1]
			: &data[--id];
		} while(*res == std::nullopt);

	} else if (dir == Direction::NEXT){
		const std::optional<glm::vec2>* res;
		do {
		res  = (++id == static_cast<int>(data.size())) // overflow check
			? &data[id = 0]
			: &data[id];
		} while(*res == std::nullopt);
	}
	return id;
};

/// get next vertex in relation of current vertex
const glm::vec2& get(
		std::vector<std::optional<glm::vec2>>& data,
		int id,
		Direction dir = Direction::CURRENT)
{
	return *data[getId(data, id, dir)];
}

/// runs fun on each triplet of vertices
/** \param fun callable object with arguments: itr to current vertex, vertex before, vertex, vertex behind
 * \attention only works when still all vertices are set
 */
void slide(auto data, auto fun)
{
	auto itr = data.begin();
	fun(itr - data.begin(), *data.back(), *itr[0], *itr[1]);
	++itr;
	for(;itr != data.end() - 1; ++itr) {
		fun(itr - data.begin(), *itr[-1], *itr[0], *itr[1]);
	}
	fun(itr - data.begin(), *data.end()[-2], *data.back(), *data.front());
}

/// calculate primary orientation of polygone
Orientation determineMainOrientation(auto data) {
	float sum = 0;
	slide(data, [&sum](auto itr, auto a, auto b, auto c) {
		sum += orientationF(a, b, c);
	});
	assert(sum != 0);
	return sum < 0 ? Orientation::NEGATIVE : Orientation::POSITIVE;
}

class EarCutTriangulation {
public:
	EarCutTriangulation(const std::vector<glm::vec3>& polygone) : m_polygone{polygone} {}
	std::vector<int> operator()();
private:
	void insertReflex(int i) { m_types.insert(m_types.begin(), i); ++m_reflexEnd; }
	void insertConvex(int i) { m_types.push_back(i); }
	void promoteReflex(int i); ///< moves convex to reflex vertices

	/// searches and for ear vertex and removes it from m_types
	/** \return index of ear
	 * \throws  const char* if no ear is found */
	int findEar();

	/// classifies all vertices between reflex and convex, stores results in m_types,m_reflexEnd
	/** \param mainOrientation primary orientation of polygone (needed to determined inside for classification of reflex and convex	 */
	void classifyVertices(Orientation mainOrientation);

	const std::vector<glm::vec3>& m_polygone;
	std::vector<std::optional<glm::vec2>> m_projected{}; ///< polygone projected to planar coordinate system, optional to remove vertices without break indices
	std::vector<int> m_types{};                          ///< polygone types[0,m_reflexEnd[ reflex vertices, types[m_reflexEnd,-1] convex vertices
	int m_reflexEnd = 0;
};

void EarCutTriangulation::promoteReflex(int i)
{
	--m_reflexEnd;
	auto buff = m_types[m_reflexEnd];
	m_types[m_reflexEnd] = m_types[i];
	m_types[i] = buff;
}


int EarCutTriangulation::findEar()
{	// check for each convex vertex if non reflex vertex is inside it's ear
	for (auto itr = m_types.begin() + m_reflexEnd;
		itr != m_types.end(); ++itr)
	{
		bool isEar = true;
		if (m_projected[*itr]) 
		{
			for (auto refl = m_types.begin();
				refl != m_types.begin() + m_reflexEnd; ++refl)
			{
				int ids[] = {
					getId(m_projected, *refl),
					getId(m_projected, *itr, Direction::PREVIOUS),
					getId(m_projected, *itr),
					getId(m_projected, *itr, Direction::NEXT)
				};
				if (ids[1] != ids[0] && ids[2] != ids[0] && ids[3] != ids[0] // check if ear is part of the triangle
					&& isIn(
						*m_projected[ids[0]],
						*m_projected[ids[1]],
						*m_projected[ids[2]],
						*m_projected[ids[3]]))
				{
					isEar = false; break;
				}
			}
		}
		if (isEar)
		{ 
			int ear = *itr;
			m_types.erase(itr);
			return ear; 
		}
	}
	throw "no ear found!";
};

void EarCutTriangulation::classifyVertices(Orientation mainOrientation)
{
	slide(m_projected, [&](auto i, auto a, auto b, auto c) {
		if(orientation(a, b, c) & mainOrientation) {
			insertConvex(i);
		} else {
			insertReflex(i);
		}
	});
}

namespace utils {
	
	std::vector<int> triangulateEarCut(const std::vector<glm::vec3>& polygone) {
		return EarCutTriangulation(polygone)();
	}
} /// namespace utils

std::vector<int> EarCutTriangulation::operator()() {
		glm::vec3 x,y; ///< orthonormal base for planar coordinate system
		int i = 0;
		do {	// find 3 vertices which not on a line
			x = glm::normalize(m_polygone[i+1] - m_polygone[i]),
			y = glm::normalize(m_polygone[i+2] - m_polygone[i+1]);
			++i;
		} while (glm::dot(x-y,x-y) < 0.0001f);

		glm::vec3 up = glm::normalize(glm::cross(x,y));
		y = glm::normalize(glm::cross(up, x));

		m_projected.reserve(m_polygone.size());
		for (const glm::vec3& v : m_polygone)
		{
			m_projected.push_back({{glm::dot(x,v), glm::dot(y,v)}});
		}

		Orientation mainOrientation = determineMainOrientation(m_projected);
		classifyVertices(mainOrientation);

		std::vector<int> res{};
		size_t finalSize = (m_projected.size() - 2) * 3;
		res.reserve(finalSize);
		while (res.size() != finalSize)
		{
			int ear = 0;
			try
			{ 
				ear = findEar();
			}
			catch (const char*)
			{
				res.clear();
				return res;
			}
			int p = getId(m_projected, ear, Direction::PREVIOUS);
			int n = getId(m_projected, ear, Direction::NEXT);
			res.push_back(p);
			res.push_back(ear);
			res.push_back(n);
			
			// check if previous vertex was an an reflex an if, if it now an convex
			auto match = std::find(m_types.begin(), m_types.end(), p) - m_types.begin();
			if (match < m_reflexEnd)
			{
				 int pp = getId(m_projected, p, Direction::PREVIOUS);
				 if (orientation(*m_projected[pp], *m_projected[p], *m_projected[n]) & mainOrientation)
				 {
					promoteReflex(match);
				 }
			}

			// check if next vertex was an an reflex an if, if it now an convex
			match = std::find(m_types.begin(), m_types.end(), n) - m_types.begin();
			if (match < m_reflexEnd)
			{
				 int nn = getId(m_projected, n, Direction::NEXT);
				 if (orientation(*m_projected[p], *m_projected[n], *m_projected[nn]) & mainOrientation)
				 {
					promoteReflex(match);
				 }
			}

			m_projected[ear] = std::nullopt;
		}
		return res;
	}

