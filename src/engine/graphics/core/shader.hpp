#pragma once

#include <glm/glm.hpp>

namespace graphics {
	
	enum class ShaderType
	{
		VERTEX			= 0x8B31,	///< GL_VERTEX_SHADER
		TESS_CONTROLL   = 0x8E88,	///< GL_TESS_CONTROL_SHADER
		TESS_EVALUATION = 0x8E87,	///< GL_TESS_EVALUATION_SHADER
		GEOMETRY		= 0x8DD9,	///< GL_GEOMETRY_SHADER
		FRAGMENT		= 0x8B30,	///< GL_FRAGMENT_SHADER
		COMPUTE 		= 0x91B9,	///< GL_COMPUTE_SHADER
	};

	/// Base class to load single shader files.
	/// \details This class also searches for include directives recursively.
	class Shader
	{
	public:
		typedef const Shader* Handle;

		~Shader();

		static Handle load(const char* _name, ShaderType _type, const char* _source = nullptr);

		static void unload(Handle _shader);
		
	private:
		/// Construction to create a shader from source code.
		///
		/// This is private to allow only load() to be used
		Shader(const char* _source, ShaderType _type);
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		// Move semantics are possible but the default generated ones would be wrong.
		Shader(Shader&&) = delete;
		Shader& operator=(Shader&&) = delete;

		unsigned m_shaderID;	///< OpenGL shader ID.
		friend class Program;
	};
	
	/// Linked usable program with different shaders.
	class Program
	{
	public:
		/// Create empty program.
		Program();
		// Move semantics are possible but the default generated ones would be wrong.
		Program(const Shader&) = delete;
		Program(Shader&&) = delete;
		Program& operator=(const Shader&) = delete;
		Program& operator=(Shader&&) = delete;
		~Program();
		
		/// Attach one more shader. Not all combinations are valid.
		/// \details Validity is tested on linking.
		void attach(const Shader* _shader);
		
		/// Use this program now, links if necessary.
		void use() const;
		
		/// Build program from the attached files.
		void link();
		
		/// TODO: reflection and uniform stuff.

		/// Find the location of a uniform variable
		int getUniformLoc(const char* _uniformName);
		
		void setUniform(int _location, float _value);
		void setUniform(int _location, const glm::vec2& _value);
		void setUniform(int _location, const glm::vec3& _value);
		void setUniform(int _location, const glm::vec4& _value);
		void setUniform(int _location, const glm::mat4x4& _value);
		void setUniform(int _location, int _value);
		void setUniform(int _location, const glm::ivec2& _value);
		void setUniform(int _location, const glm::ivec3& _value);
		void setUniform(int _location, const glm::ivec4& _value);
		void setUniform(int _location, unsigned _value);
		void setUniform(int _location, const glm::uvec2& _value);
		void setUniform(int _location, const glm::uvec3& _value);
		void setUniform(int _location, const glm::uvec4& _value);

		/// Get OpenGL handle.
		unsigned getID() const { return m_programID; }
	private:
		const Shader* m_shaders[5]; 	///< List of attached shaders
		int m_numShaders;
		unsigned m_programID;		///< OpenGL program ID
	};

} // namespace graphics
