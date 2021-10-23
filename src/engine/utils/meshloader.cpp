#include "meshloader.hpp"

#include "triangulation.hpp"

#include <string>
#include <string_view>
#include <fstream>
#include <sstream>
#include <exception>
#include <tuple>
#include <cctype>
#include <charconv>

#include <iostream>

#include <glm/gtx/norm.hpp>

class parsing_error : public std::exception {
public:
	parsing_error(const std::string& msg) : m_msg{msg} {} 
	const char* what() const noexcept override { return m_msg.c_str(); }
private:
	std::string m_msg;
};

void errorTypeNotSupported(std::string_view _fileName) {
	spdlog::error("[utils] Could not load '", _fileName, "' "
	"type not supported. Only support '.obj'");
}

struct TmpData {
	size_t currentMtl;
	utils::MeshData::group_t currentGroup;
	const std::string& directory;
};

float parseFloat(const char* begin, const char* last, const char** new_begin = nullptr) {
	float f;
	auto [ptr, ec] = std::from_chars(begin, last, f);
	if(ec == std::errc::invalid_argument) {
		throw parsing_error(std::string("Failed to convert '") + begin + "' to number!");
	} else if (ec == std::errc::result_out_of_range ){
		throw parsing_error(std::string("Unable to store number as float: '") + begin + "'");
	}
	if(new_begin) { *new_begin = ptr; }
	return f;
}

float parseFloat(const std::string_view& line) {
	return parseFloat(line.data(), line.data() + line.size());
}

glm::vec3 parseVec3(const std::string_view& line, std::string::size_type& begin) {
	glm::vec3 res;
	const char* str = line.data() + begin;
	for(int i = 0; i < 3; ++i) 
	{
		res[i] = parseFloat(str, line.data() + line.size(), &str);
		++str;
	}
	begin = str - line.data();
	return res;
}

glm::vec2 parseVec2(const std::string& line, std::string::size_type begin) {
	std::string::size_type pos = begin, diff;
	glm::vec2 res;
	for(int i = 0; i < 2; ++i) 
	{
		float val = std::stof(line.substr(pos), &diff);
		if ( std::fabs(val) == 0 ) val = 0;
		res[i] = val;
		pos += diff;
	}
	return res;
}

struct MtlParser {
	std::string_view m_filename;
	utils::MeshData& m_mesh;
	std::string m_currentName;
	TmpData& m_tmp;
	std::optional<utils::MeshData::MaterialData> m_newMtl = std::nullopt;
	MtlParser(utils::MeshData& mesh, TmpData& tmp, const std::string& filename)
		: m_filename{filename}, m_mesh{mesh}, m_tmp{tmp}
	{
		while (m_filename[m_filename.size() - 1] == 13) {
			m_filename.remove_suffix(1);
		}
	}
	utils::MeshData::MaterialData::TextureData parseText(const std::string_view& line) {
		utils::MeshData::MaterialData::TextureData data{glm::vec3{0}, glm::vec3{0}, false, ""};
		std::string_view rest = line;
		while(rest[0] == '-') {
			auto end = rest.find_first_of(' ');
			const std::string_view option = rest.substr(1, end);
			if (option == "o") { data.offset = parseVec3(rest, end); rest = rest.substr(end+1); }
			else if (option == "s") { data.scale = parseVec3(rest, end); rest = rest.substr(end+1); }
			else if (option == "clamp") {
				auto end2 = rest.find_first_of(' ', end+1);
				auto on_off = rest.substr(end+1, end2);
				if (on_off == "on") {
					data.clamp = true;
				} else if (on_off != "off") {
					spdlog::error("Material texture has invalid clamp value! valid values are: 'on' and 'off'");
				}
				rest = rest.substr(end2+1);
			}
			else {
				spdlog::warn("unparsed texture option: '{}'", option);
			}
		}
		spdlog::info("load texture: {}", rest);
		data.name = rest;
		return data;
	}
	void parseIlumination(const std::string_view& line) {
		int i = static_cast<int>(parseFloat(line.data(), line.data() + line.size()));
		if(i < 0 || i >= utils::MeshData::MaterialData::USED_LIGHT::END) {
			spdlog::warn("unsupported illumination model for object, use FULL instead!");
			i = 2;
		}
		m_newMtl->illumination = static_cast<utils::MeshData::MaterialData::USED_LIGHT>(i);
	}
	void parseOpacity(const std::string_view& line) {
		float f = parseFloat(line.data(), line.data() + line.size());
		if (f != 1.f) { spdlog::warn("opacity value not equal 1!"); }
	}
	void parseLine(const std::string_view& line) {
		std::string::size_type begin = line.find_first_of(' ');
		std::string_view cmd = line.substr(0, begin);
		++begin;
		if(cmd == "newmtl" ) {
			if (m_newMtl) {
				m_mesh.material_names[m_currentName] = m_mesh.material.size();
				m_mesh.material.push_back(m_newMtl.value());
			}
			m_currentName = line.substr(begin+1, line.find_last_of((char)13));
			m_newMtl = utils::MeshData::MaterialData{};
		}
		else if (cmd == "Ka") { m_newMtl->color.ambient = parseVec3(line, begin); }
		else if (cmd == "Kd") { m_newMtl->color.diffuse = parseVec3(line, begin); }
		else if (cmd == "Ks") { m_newMtl->color.spectral = parseVec3(line, begin); }
		else if (cmd == "Ke") { spdlog::info("Ignores Ke value"); }
		else if (cmd == "map_Ka") { m_newMtl->textures.ambient = parseText(line.substr(begin)); }
		else if (cmd == "map_Kd") { m_newMtl->textures.diffuse = parseText(line.substr(begin)); }
		else if (cmd == "map_Ks") { m_newMtl->textures.spectral = parseText(line.substr(begin)); }
		else if (cmd == "map_Ns") { m_newMtl->textures.spectralExponent = parseText(line.substr(begin)); }
		else if (cmd == "illum") { parseIlumination(line.substr(begin)); }
		else if (cmd == "d") { parseOpacity(line.substr(begin)); }
		else if (cmd == "Ns") { m_newMtl->spectralExponent = parseFloat(line.substr(begin+1)); }
		else if (cmd == "Ni") { spdlog::info("pasrer ingnores opctical density!"); }
		else if (cmd == "relf") { m_newMtl->textures.reflection = parseText(line.substr(begin+1)); }
	}

	void parse() {
		std::string_view suffix(m_filename);
		auto pos = suffix.find_last_of('.');
		suffix.remove_prefix(pos);
		if(suffix == ".mtl") {
			std::string filename = m_tmp.directory + m_filename.data();
			std::ifstream file(filename);
			if(file) {
				try {
					std::string line;
					while(std::getline(file, line))	
					{
						parseLine(line);
					}
					m_mesh.material_names[m_currentName] = m_mesh.material.size();
					m_mesh.material.push_back(m_newMtl.value());
				} catch (const parsing_error& err) {
					spdlog::error("failed to load material data from file: '{}', with: '{}'",
							m_filename, err.what());
					throw parsing_error("failed to parse material");
				}
			} else {
				spdlog::error("failed to open file: '{}'", filename);
				throw parsing_error("failed to open file");
			}
		} else {
			errorTypeNotSupported(m_filename);
		}
	}
};



void mapIndices(const utils::MeshData& data, const TmpData& tmp, std::vector<utils::MeshData::FaceData::VertexIndices>& f, std::vector<utils::MeshData::FaceData>& faces) {
	using Vertex = utils::MeshData::FaceData::VertexIndices;
	for(Vertex& v : f) {
		if((v.positionIdx -= 1) < 0) {
			v.positionIdx = data.positions.size() + v.positionIdx + 1;
		}
		if(v.textureCoordinateIdx && (*v.textureCoordinateIdx -=1 ) < 0) {
			v.textureCoordinateIdx = data.textureCoordinates.size() + *v.textureCoordinateIdx + 1; 
		}
		if(v.normalIdx && (*v.normalIdx -= 1) < 0) {
			v.normalIdx = data.normals.size() + *v.normalIdx + 1;
		}
	}
	if(f.size() == 3) {
		faces.push_back({{f[0], f[1], f[2]}, tmp.currentGroup});
	} else if (f.size() == 4) {
		if(
				glm::distance2(data.positions[f[0].positionIdx], data.positions[f[2].positionIdx])
				< glm::distance2(data.positions[f[1].positionIdx], data.positions[f[3].positionIdx]))
		{
			faces.push_back({{f[0],f[1],f[2]}, tmp.currentGroup});
			faces.push_back({{f[2],f[3],f[0]}, tmp.currentGroup});
		} else {
			faces.push_back({{f[1],f[2],f[3]}, tmp.currentGroup});
			faces.push_back({{f[3],f[0],f[1]}, tmp.currentGroup});
		}
	} else {
		std::vector<glm::vec3> poly{};
		for(Vertex& v : f) { poly.push_back(data.positions[v.positionIdx]); }
		std::vector<int> trias = utils::triangulateEarCut(poly);
		for(auto itr = trias.begin(); itr != trias.end();) {
			faces.push_back({{f[*itr++],f[*itr++],f[*itr++]}, tmp.currentGroup});
		}
	}
		
}


struct Command 
{ 
	static bool check(const std::string_view& name) { return name == "#"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData&) {}
};

struct Vertex
{
	static bool check(const std::string_view& name) { return name == "v"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData&) 
	{
		begin += 1;
		mesh.positions.emplace_back(parseVec3(line, begin));
	}
};

struct TextureCoordinate
{
	static bool check(const std::string_view& name) { return name == "vt"; } 
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData&) 
	{
		++begin;
		mesh.textureCoordinates.emplace_back(parseVec2(line, begin));
	}
};

struct Normal
{
	static bool check(const std::string_view& name) { return name == "vn"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData&) 
	{
		++begin;
		mesh.normals.emplace_back(parseVec3(line, begin));
	}
};

struct Point
{
	static bool check(const std::string_view& name) { return name == "p"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData&)
	{
		spdlog::warn("Parser will ignore ponit data!");
	}
};

struct Line
{
	static bool check(const std::string_view& name) { return name=="l"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData&)
	{
		spdlog::warn("Parser will ignore lines!");
	}
};

struct Group
{
	static bool check(const std::string_view& name) { return name=="l"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData& tmp)
	{
		std::string::size_type start = begin, end;
		const std::string_view& view = line;
		std::string name;
		utils::MeshData::group_t activeGroups = 0;
		auto add = [&](const std::string& name){
			auto [itr, added] = mesh.group_names.insert({name, 0b1 << mesh.group_names.size()});
			activeGroups |= itr->second;
		};
		while((end = view.find_last_of(' ', start)) != std::string::npos) {
			name = view.substr(start, end - start);
			add(name);
			start = end + 1;
			while(view[start] == ' ') { ++start; }
		}
		name = view.substr(start);
		if (name.size() > 0) { add(name); }
		
		tmp.currentGroup = activeGroups;
	}
};

struct Name
{
	static bool check(const std::string_view& name) { return name == "o"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData& tmp)
	{
		mesh.objects.push_back({line.substr(begin), mesh.faces.size(), 0, });
	}
};

struct Face
{
	static bool check(const std::string_view& name) { return name == "f"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData& tmp) 
	{
		static std::vector<utils::MeshData::FaceData::VertexIndices> f;
		int posIdx;
		std::optional<int> texCoordIdx;
		std::optional<int> normalIdx;
		f.resize(0);
		std::string::size_type pos = begin, diff;
		while(pos < line.size() && line[pos] != 13) {
			posIdx = std::stoi(line.substr(pos), &diff); 		
			pos += diff;
			if (line[pos] == '/')  
			{
				++pos;
				if (line[pos] == '/') // with normal, without texture
				{
					++pos;
					normalIdx = std::stoi(line.substr(pos), &diff);
					texCoordIdx = std::nullopt;
					pos += diff;
				}
				else // with texture
				{
					texCoordIdx = std::stoi(line.substr(pos), &diff);
					pos += diff;
					if (line[pos] == '/') // with normal
					{
						++pos;
						normalIdx = std::stof(line.substr(pos), &diff);
						pos += diff;
					}
					else // without normal
					{
						normalIdx = std::nullopt;
					}
				}
			} 
			else // only position 
			{
				texCoordIdx = std::nullopt;
				normalIdx = std::nullopt;
			}
			f.emplace_back(posIdx, texCoordIdx, normalIdx);
		} 
		bool contains[] = {
			f.front().textureCoordinateIdx.has_value(),
			f.front().normalIdx.has_value(),
		};
		for(const auto& v : f) {
			if(v.textureCoordinateIdx.has_value() != contains[0]
					|| v.normalIdx.has_value() != contains[1])
			{
				throw parsing_error("face with inconsistent vertex descriptions");	
			}
		}
		mapIndices(mesh, tmp, f,mesh.faces);
	}
};

struct MtlLib {
	static bool check(const std::string_view& name) { return name == "mtllib"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData& tmp) {
		++begin;
		std::string name = line.substr(begin);
		MtlParser parser(mesh, tmp, name);
		parser.parse();
	}
};

struct UseMtl {
	static bool check(const std::string_view& name) { return name == "usemtl"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData& tmp) {
		tmp.currentMtl = mesh.material_names[line.substr(begin, line.find_first_of(' '))];
	}
};

struct SmoothGroup {
	static bool check(const std::string_view& name) { return name == "s"; }
	static void parse(
			const std::string& line,
			std::string::size_type begin,
			utils::MeshData& mesh,
			TmpData&) {
		spdlog::warn("Parser ingnores SmoothGroups!");
	}
};

using LineTypes = std::tuple<
	Command,
	Vertex,
	TextureCoordinate,
	Normal,
	Point,
	Line,
	Group,
	Name,
	Face,
	MtlLib,
	UseMtl,
	SmoothGroup> ;
template<int I = 0>
void parseLine(
		const std::string_view& name, 
		const std::string& line,
		std::string::size_type begin,
		utils::MeshData& mesh,
		TmpData& tmp) 
{
	using LineType = std::tuple_element_t<I, LineTypes>;
	if (LineType::check(name)) {
		LineType::parse(line, begin, mesh, tmp);	
	}
	else
	{
		if constexpr (I + 1 < std::tuple_size_v<LineTypes>)
			parseLine<I + 1>(name, line, begin, mesh, tmp);
		else
			throw parsing_error("unknown line type: >" + std::string(name) + "<");
	}
}

void parseLine( const std::string& line, utils::MeshData& data, TmpData& tmp) {
	std::string::size_type begin = 0, end = 0;
	while( std::isspace( line[begin] ) ) { ++begin; }

	end = line.find_first_of(' ', begin);

	if ( end == std::string::npos) 
	{
		if (line.length() != 0 &&  (line.length() != 1 || !std::isspace(line[0])))
		{
			throw parsing_error("expected space after type identifier!");
		}
	}
	else
	{
		parseLine(
				{line.c_str() + begin, end - begin}, 
				line.substr(begin), end, data, tmp);
	}
}

void parseObj( std::string& directory, std::ifstream& file, utils::MeshData& data) {
	std::string line, type;
	int lineNumber = 0;
	TmpData tmp{.directory = directory};
	try {
	while ( std::getline(file, line) ) 
	{
		parseLine(line, data, tmp);
		++lineNumber;
	}
	} catch (const parsing_error& err) {
		throw parsing_error("line: " + std::to_string(lineNumber)
				+ ": "+ err.what());
	}
}

namespace utils {
	MeshData::Handle MeshData::load( const char* _fileName ) 
	{
		MeshData* data = new MeshData;
		std::string_view fileName(_fileName);
		std::string_view suffix( _fileName );
		auto pos = suffix.find_last_of( '.' );
		suffix.remove_prefix( pos );
		if ( suffix == ".obj" ) {
			std::ifstream file( _fileName );
			if (file) {
				try {
					auto dir = fileName.substr(0, fileName.find_last_of('/') + 1);
					std::string directory(dir.begin(), dir.end());
					parseObj( directory, file, *data );
				} catch (const parsing_error& err) {
					spdlog::error("failed to load mesh data from file: '{}' with: '{}'",
							_fileName, err.what());
					delete data;
					return nullptr;
				}
			} else {
				spdlog::error("failed to open file: '{}'", _fileName);
				delete data;
				return nullptr;
			}
		}  else {
			errorTypeNotSupported( _fileName );
		}

		return data;
	}

	void MeshData::unload( MeshData::Handle _meshData ) 
	{
		delete const_cast<MeshData*>( _meshData );
	}

} // end utils
