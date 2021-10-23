#pragma  once

#include <tuple>
#include <optional>
#include <vector>
#include <array>
#include <glm/mat3x3.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/geometric.hpp>


#include <iostream>
#include <cassert>

namespace utils {
	namespace triangulation {
		inline float orientationF(
				const glm::vec2& a,
				const glm::vec2& b,
				const glm::vec2& c)
		{
			return ((b.x*c.y + a.x*b.y + a.y*c.x) - (a.y*b.x + b.y*c.x + a.x*c.y));
		}
		inline char orientation(
				const glm::vec2& a,
				const glm::vec2& b,
				const glm::vec2& c)
		{
			float v = orientationF(a, b, c);
			return 		0b10 * (v <= 0)
					| 	0b01 * (v >= 0);
		}
		inline bool isIn(
				const glm::vec2& p,
				const glm::vec2& a,
				const glm::vec2& b,
				const glm::vec2& c)
		{
			char orient[] = {
				orientation(a,b,p),
				orientation(b,c,p),
				orientation(c,a,p)
			};
			return 	   ((orient[0] & 0b10) == (orient[1] & 0b10) && orient[1] == (orient[2] & 0b10))
					|| ((orient[0] & 0b01) == (orient[1] & 0b01) && orient[1] == (orient[2] & 0b01));
		}
		inline int getId (
				std::vector<std::optional<glm::vec2>>& data,
				int id,
				int i = 0) {
			if(i == 0) { return id; }
			else if(i < 0) {
				const auto& res = id == 0
					? data[id = data.size() - 1]
					: data[--id];
				if(res) { return id; }
				return getId(data, id, i);

			} else {
				const auto& res  = (++id == static_cast<int>(data.size()))
					? data[id = 0]
					: data[id];
				if(res) { return id; }
				return getId(data, id, i);
			}
		};
		inline const glm::vec2& get(
				std::vector<std::optional<glm::vec2>>& data,
				int id,
				int i = 0) {
			return *data[getId(data, id, i)];
		}
	}

	
	inline std::vector<int> triangulateEarCut(const std::vector<glm::vec3>& polygone) {
		glm::vec3 x,y;
		{
			int i = 0;
			do {
				x = glm::normalize(polygone[i+1] - polygone[i]),
				y = glm::normalize(polygone[i+2] - polygone[i+1]);
				++i;
			} while(glm::dot(x-y,x-y) < 0.0001f);
		}
		{
			glm::vec3 up = glm::normalize(glm::cross(x,y));
			y = glm::normalize(glm::cross(up, x));
		}
		std::vector<std::optional<glm::vec2>> projected{};
		projected.reserve(polygone.size());
		for(const glm::vec3& v : polygone) {
			projected.push_back({{glm::dot(x,v), glm::dot(y,v)}});
		}

		std::vector<int> types;
		int reflexEnd = 0;
		auto insertReflex = [&](int i) {
			types.insert(types.begin(), i);
			++reflexEnd;
		};
		auto insertConvex = [&](int i) {
			types.insert(types.begin() + reflexEnd, i);
		};
		auto promoteReflex = [&](int i) {
			--reflexEnd;
			auto buff = types[reflexEnd];
			types[reflexEnd] = types[i];
			types[i] = buff;
		};
		auto findEar = [&]() -> int {
			for(auto itr = types.begin() + reflexEnd;
				itr != types.end(); ++itr) {
				bool isEar = true;
				if (projected[*itr])
				for(auto refl = types.begin();
					refl != types.begin() + reflexEnd; ++refl) {
					int ids[] = {
						triangulation::getId(projected, *refl),
						triangulation::getId(projected, *itr, -1),
						triangulation::getId(projected, *itr),
						triangulation::getId(projected, *itr, 1)
					};
					if(	   ids[1] != ids[0] && ids[2] != ids[0] && ids[3] != ids[0]
						&& triangulation::isIn(
							*projected[ids[0]],
							*projected[ids[1]],
							*projected[ids[2]],
							*projected[ids[3]]))
					{
						isEar = false; break;
					}
				}
				if(isEar) { 
					int ear = *itr;
					types.erase(itr);
					return ear; 
				}
			}
			throw "no ear found!";
		};
		// assumption: alls set
		auto slide = [](auto data, auto fun) {
			auto itr = data.begin();
			fun(itr - data.begin(), *data.back(), *itr[0], *itr[1]);
			++itr;
			for(;itr != data.end() - 1; ++itr) {
				fun(itr - data.begin(), *itr[-1], *itr[0], *itr[1]);
			}
			fun(itr - data.begin(), *data.end()[-2], *data.back(), *data.front());
		};
		char mainOrientation;
		{
			float sum = 0;
			slide(projected, [&sum](auto itr, auto a, auto b, auto c) {
				sum += triangulation::orientationF(a, b, c);
			});
			assert(sum != 0);
			mainOrientation = sum < 0 ? 0b10 : 0b01;
			slide(projected, [&](auto i, auto a, auto b, auto c) {
				if(triangulation::orientation(a, b, c) & mainOrientation) {
					insertConvex(i);
				} else {
					insertReflex(i);
				}
			});
		}
		std::vector<int> res{};
		size_t finalSize = (projected.size() - 2) * 3;
		res.reserve(finalSize);
		while(res.size() != finalSize) {
			int ear = 0;
			try { 
				ear = findEar();
			} catch (const char*) {
				res.clear();
				return res;
			}
			int p = triangulation::getId(projected, ear, -1),
				 pp = triangulation::getId(projected, p, -1);
			int n = triangulation::getId(projected, ear, 1),
				 nn = triangulation::getId(projected, n, 1);
			res.push_back(p);
			res.push_back(ear);
			res.push_back(n);
			
			auto match = std::find(types.begin(), types.end(), p) - types.begin();
			if(    	match < reflexEnd
				&& 	triangulation::orientation(*projected[pp], *projected[p], *projected[n]) & mainOrientation) {
					promoteReflex(match);
			}
			match = std::find(types.begin(), types.end(), n) - types.begin();
			if(		match < reflexEnd
				&&	triangulation::orientation(*projected[p], *projected[n], *projected[nn]) & mainOrientation) {
					promoteReflex(match);
			}
			projected[ear] = std::nullopt;
		}
		return res;
	}
}
