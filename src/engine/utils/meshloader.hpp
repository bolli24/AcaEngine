#pragma once

#include "resourcemanager.hpp"

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include <array>
#include <optional>
#include <vector>
#include <unordered_map>
#include <string>
#include <concepts>

namespace utils {
	/** 
	 * @brief Collection of raw mesh data.
	 * triangle data can be found in: faces
	 */
	struct MeshData 
	{
		using Handle = const MeshData*;
		using group_t = uint32_t;

		/**
		 * @brief Collection of vertices describing one face. 
		 * contains 3 ordered vertices
		 */
		struct FaceData 
		{
			/**
			 * @brief Collection of ids describing one vertex. 
			 */
			struct VertexIndices 
			{
				int positionIdx;
				///< id to fetch the position from MeshData::positions
				std::optional<int> textureCoordinateIdx;
				///< id to fetch the texture Coordinates from MeshData::textureCoordinates
				///< may be not set for a vertex
				std::optional<int> normalIdx;
				///< id to fetch the normal from MeshData::normals
				///< may be not set for a vertex
			};
			std::array<VertexIndices, 3> indices;
			group_t groups;
		};


		/**
		 * @brief load mesh data from an file 
		 * @param _fileName path to file 
		 */
		static Handle load( const char* _fileName ) ;

		static void unload( Handle _meshData );

		/// list of all positions contains values used by FaceData
		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> textureCoordinates;
		std::vector<glm::vec3> normals;
		/// list of all faces described in .obj in order
		std::vector<FaceData>  faces;

		struct MaterialData {
			struct {
				glm::vec3 ambient;
				glm::vec3 diffuse;
				glm::vec3 spectral;
			} color;
			struct TextureData {
				glm::vec3 offset;
				glm::vec3 scale;
				bool clamp;
				std::string name;
			};
			struct {
				TextureData ambient;
				TextureData diffuse;
				TextureData spectral;
				TextureData spectralExponent;
				TextureData reflection;
			} textures;
			float spectralExponent;
			enum USED_LIGHT {
				DIFFUSE_ONLY,
				DIFFUSE_AND_AMBIENT,
				FULL,
				END
			} illumination;
		};
		struct ObjectData {
			std::string name;
			size_t begin;
			size_t end;
			size_t maitalIdx;
		};
		std::vector<MaterialData> material;
		std::vector<ObjectData> objects;

		std::unordered_map<std::string,group_t> group_names;
		std::unordered_map<std::string,size_t> material_names;

		using VertexPosition = glm::vec3;
		struct VertexTexture {
			glm::vec3 position;
			glm::vec2 texture;
		};
		struct VertexNormal {
			glm::vec3 position;
			glm::vec3 normal;
		};
		struct VertexTextureNormal {
			glm::vec3 position;
			glm::vec3 normal;
			glm::vec2 texture;
		};

	};

	
	using MeshLoader = utils::ResourceManager<MeshData>;

} // end utils
