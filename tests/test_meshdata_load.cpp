#include <engine/utils/meshloader.hpp>

#include <iostream>
#include <string>
#include <string_view>
#include <optional>

#ifndef RESOURCE_FOLDER
#define RESOURCE_FOLDER ""
#endif

namespace std {
	string to_string(const optional<int>&  val) {
		return val ? std::to_string(val.value()) : std::string("nullopt");
	}
	string to_string(const glm::vec3& vec) {
		return
			"(" + std::to_string(vec[0]) + ","
			+ std::to_string(vec[1]) + ","
			+ std::to_string(vec[2]) + ")";
	}
	string to_string(const glm::vec2& vec) {
		return
			"(" + std::to_string(vec[0]) + ","
			+ std::to_string(vec[1]) + ")";
	}
} // end namespace std

template<typename T>
std::string faceMismatchError(
		int fId, int vId, 
		const std::string& attr,
		const T& get,
		const T& expected)
{
		return "expected face "
			+ std::to_string(fId) + " vertex " + std::to_string(vId)
			+ " " + attr + " to be " + std::to_string(expected)
			+ " but got " +  std::to_string(get);
}

template<typename T>
std::string valueMismatchError(
		int id,
		const std::string& attr,
		const T& get,
		const T& expected)
{
	throw "expected "+ attr  + " "+ std::to_string(id)
		+ " to be " + std::to_string(expected)
		+ ", but got " + std::to_string(get);
}

std::string sizeMismatchError(const std::string& attr, int get, int expected)
{
	return "expected " + attr + " to have size: " + std::to_string(expected)
		+ " but got: " + std::to_string(get);
}

// load tet.obj end check if all faces, vertices, texture coordinates 
// and normals read correctly
int main() {
	try {
		std::unique_ptr<const utils::MeshData> data(utils::MeshData::load(RESOURCE_FOLDER "/tet.obj"));

		std::array<glm::vec3, 4> positons = {{
			{0, 0, 0},
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, 1}
		}};
		std::array<glm::vec2, 4> texCoords = {{
			{0, 0},
			{1, 0},
			{0, 1},
			{1, 1}
		}};
		std::array<glm::vec3, 4> normals = {{
			{1, 0, 0},
			{0, 1, 0},
			{0, 0, -1},
			{1, 1, 1}
		}};

		std::array<utils::MeshData::FaceData, 16> faces;
			faces[0].indices[0].positionIdx = 0;
			faces[0].indices[0].textureCoordinateIdx = 0; 
			faces[0].indices[0].normalIdx = 0;
			faces[0].indices[1].positionIdx = 3;
			faces[0].indices[1].textureCoordinateIdx = 3;
			faces[0].indices[1].normalIdx = 0;
			faces[0].indices[2].positionIdx = 1;
			faces[0].indices[2].textureCoordinateIdx = 1;
			faces[0].indices[2].normalIdx = 0;

			faces[1].indices[0].positionIdx = 0;
			faces[1].indices[0].textureCoordinateIdx = 0; 
			faces[1].indices[0].normalIdx = 1;
			faces[1].indices[1].positionIdx = 2;
			faces[1].indices[1].textureCoordinateIdx = 2;
			faces[1].indices[1].normalIdx = 1;
			faces[1].indices[2].positionIdx = 3;
			faces[1].indices[2].textureCoordinateIdx = 3;
			faces[1].indices[2].normalIdx = 1;

			faces[2].indices[0].positionIdx = 0;
			faces[2].indices[0].textureCoordinateIdx = 0; 
			faces[2].indices[0].normalIdx = 2;
			faces[2].indices[1].positionIdx = 1;
			faces[2].indices[1].textureCoordinateIdx = 1;
			faces[2].indices[1].normalIdx = 2;
			faces[2].indices[2].positionIdx = 2;
			faces[2].indices[2].textureCoordinateIdx = 2;
			faces[2].indices[2].normalIdx = 2;

			faces[3].indices[0].positionIdx = 3;
			faces[3].indices[0].textureCoordinateIdx = 3; 
			faces[3].indices[0].normalIdx = 3;
			faces[3].indices[1].positionIdx = 1;
			faces[3].indices[1].textureCoordinateIdx = 1;
			faces[3].indices[1].normalIdx = 3;
			faces[3].indices[2].positionIdx = 2;
			faces[3].indices[2].textureCoordinateIdx = 2;
			faces[3].indices[2].normalIdx = 3;

			for (int i = 4; i < 8; ++i) {
				faces[i] = faces[i - 4];
				for(auto& idx : faces[i].indices) {
					idx.textureCoordinateIdx = std::nullopt;
				}
			}

			for (int i = 8; i < 12; ++i) {
				faces[i] = faces[i - 8];
				for(auto& idx : faces[i].indices) {
					idx.normalIdx = std::nullopt;
				}
			}

			for (int i = 12; i < 16; ++i) {
				faces[i] = faces[i - 12];
				for(auto& idx : faces[i].indices) {
					idx.textureCoordinateIdx = std::nullopt;
					idx.normalIdx = std::nullopt;
				}
			}

		// check positions
		if ( data->positions.size() != positons.size() ) {
			throw sizeMismatchError(
					"positions", data->positions.size(), positons.size());
		}
		for(size_t i = 0; i < positons.size(); ++i) {
			if ( data->positions[i] != positons[i]) {
				throw valueMismatchError(i, "positions", 
						data->positions[i], positons[i]);
			}
		}

		// check texture coordinates
		if ( data->textureCoordinates.size() != texCoords.size() ) 
		{
			throw sizeMismatchError(
					"texture coordinates", 
					data->textureCoordinates.size(), texCoords.size());
		}
		for( size_t i = 0; i < texCoords.size(); ++i) 
		{
			if ( data->textureCoordinates[i] != texCoords[i] ) 
			{
				throw valueMismatchError(i, "texture coordinates",
						data->textureCoordinates[i], texCoords[i]);
			}
		}

		// check normals
		if ( data->normals.size() != normals.size() ) {
			throw sizeMismatchError(
					"normals", data->normals.size(), positons.size());
		}
		for(size_t i = 0; i < normals.size(); ++i) {
			if ( data->normals[i] != normals[i]) {
				throw valueMismatchError(i, "normals",
						data->normals[i], positons[i]);
			}
		}

		// check faces
		if ( data->faces.size() != faces.size() ) {
			throw sizeMismatchError("faces", data->faces.size(), faces.size());
		}

		for ( size_t i = 0; i < faces.size(); ++i) {
			const utils::MeshData::FaceData& expected = faces[i];
			const utils::MeshData::FaceData& readed = data->faces[i];
			for ( int x = 0; x < 3; ++x ) 
			{
				if ( readed.indices[x].positionIdx 
						!= expected.indices[x].positionIdx ) 
				{
					throw faceMismatchError(i, x, "positionIdx", 
							readed.indices[x].positionIdx,
							expected.indices[x].positionIdx);
				}
				if ( readed.indices[x].normalIdx 
						!= expected.indices[x].normalIdx ) 
				{
					throw faceMismatchError(i, x, "normalIdx", 
							readed.indices[x].normalIdx,
							expected.indices[x].normalIdx);
				}
				if ( readed.indices[x].textureCoordinateIdx 
						!= expected.indices[x].textureCoordinateIdx ) 
				{
					throw faceMismatchError(i, x, "textureCoordinateIdx", 
							readed.indices[x].textureCoordinateIdx,
							expected.indices[x].textureCoordinateIdx);
				}
			}
		}

	} catch (const std::string& msg) {
		std::cerr << msg << std::endl;
		return 1;
	}
	return 0;
}
