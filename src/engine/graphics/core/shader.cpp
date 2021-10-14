#include "opengl.hpp"
#include "shader.hpp"
#include <spdlog/spdlog.h>
#include <string>
#include <memory>

namespace graphics {

	Shader::Shader(const char* _source, ShaderType _type)
	{
		m_shaderID = glCall(glCreateShader, unsigned(_type));
		int len = (int)strlen(_source);
		glCall(glShaderSource, m_shaderID, 1, &_source, &len);
		glCall(glCompileShader, m_shaderID);
		GLint isCompiled = 0;
		glCall(glGetShaderiv, m_shaderID, GL_COMPILE_STATUS, &isCompiled);
		if(isCompiled == GL_FALSE)
		{
			GLint maxLength = 0;
			glCall(glGetShaderiv, m_shaderID, GL_INFO_LOG_LENGTH, &maxLength);
	 
			// The maxLength includes the NULL character
			std::string errorLog;
			errorLog.resize(maxLength);
			glCall(glGetShaderInfoLog, m_shaderID, maxLength, &maxLength, &errorLog[0]);
			spdlog::error(errorLog);
			glCall(glDeleteShader, m_shaderID);
			return;
		}
	}
	
	Shader::~Shader()
	{
		glCall(glDeleteShader, m_shaderID);
	}

	Shader::Handle Shader::load(const char* _name, ShaderType _type, const char* _source)
	{
		if(!_source) {
			spdlog::info("Creating shader from file: {}", _name);
			FILE* file = fopen(_name, "rb");
			if (!file)
				spdlog::error(("Cannot open shader file: " + std::string(_name)).c_str());
			else 
			{	
				// Get file size and allocate memory
				fseek(file, 0, SEEK_END);
				unsigned fileLength = ftell(file);
				std::unique_ptr<char[]> source(new char[fileLength + 1]);
				fseek(file, 0, SEEK_SET);
				// Read in whole file and make 0 terminated.
				fread(source.get(), 1, fileLength, file);
				source[fileLength] = 0;
				fclose(file);
				return new Shader(source.get(), _type);
			}
		}
		return new Shader(_source, _type);
	}

	void Shader::unload(Shader::Handle _shader)
	{
		// The handle is defined as const, so nobody can do damage, but now we need
		// the real address for deletion
		delete const_cast<Shader*>(_shader);
	}


	
	Program::Program() :
		m_numShaders(0),
		m_programID(0)
	{
		for(int i = 0; i < 5; ++i)
			m_shaders[i] = nullptr;
	}
	
	Program::~Program()
	{
		glCall(glDeleteProgram, m_programID);
	}
	
	void Program::attach(const Shader* _shader)
	{
		if(!m_programID)
			m_programID = glCall(glCreateProgram);
		if(m_numShaders < 5)
		{
			m_shaders[m_numShaders++] = _shader;
			glCall(glAttachShader, m_programID, _shader->m_shaderID);
		} else spdlog::error("Already 5 shaders bound. This is the maximum: Vertex + Hull + Domain + Geometry + Fragment.");
	}
		
	void Program::use() const
	{
		if(!m_programID)
			spdlog::error("Trying to use an uninitialized program!");
		glCall(glUseProgram, m_programID);
	}
	
	void Program::link()
	{
		if(!m_programID)
			spdlog::error("Trying to link an uninitialized program!");
		glCall(glLinkProgram, m_programID);

		GLint isLinked = 0;
		glCall(glGetProgramiv, m_programID, GL_LINK_STATUS, &isLinked);
		if(isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glCall(glGetProgramiv, m_programID, GL_INFO_LOG_LENGTH, &maxLength);
	 
			// The maxLength includes the NULL character
			std::string errorLog;
			errorLog.resize(maxLength);
			glCall(glGetProgramInfoLog, m_programID, maxLength, &maxLength, &errorLog[0]);
			spdlog::error(errorLog);

			glCall(glDeleteProgram, m_programID);
			m_programID = 0;
		}
	}

	int Program::getUniformLoc(const char* _uniformName)
	{
		return glCall(glGetUniformLocation, m_programID, _uniformName);
	}

	void Program::setUniform(int _location, float _value)
	{
		glCall(glProgramUniform1f, m_programID, _location, _value);
	}

	void Program::setUniform(int _location, const glm::vec2& _value)
	{
		glCall(glProgramUniform2fv, m_programID, _location, 1, (const float*)&_value);
	}

	void Program::setUniform(int _location, const glm::vec3& _value)
	{
		glCall(glProgramUniform3fv, m_programID, _location, 1, (const float*)&_value);
	}

	void Program::setUniform(int _location, const glm::vec4& _value)
	{
		glCall(glProgramUniform4fv, m_programID, _location, 1, (const float*)&_value);
	}

	void Program::setUniform(int _location, const glm::mat4x4& _value)
	{
		glCall(glProgramUniformMatrix4fv, m_programID, _location, 1, GL_FALSE, (const float*)&_value);
	}

	void Program::setUniform(int _location, int _value)
	{
		glCall(glProgramUniform1i, m_programID, _location, _value);
	}

	void Program::setUniform(int _location, const glm::ivec2 & _value)
	{
		glCall(glProgramUniform2iv, m_programID, _location, 1, (const GLint*)&_value);
	}

	void Program::setUniform(int _location, const glm::ivec3 & _value)
	{
		glCall(glProgramUniform3iv, m_programID, _location, 1, (const GLint*)&_value);
	}

	void Program::setUniform(int _location, const glm::ivec4 & _value)
	{
		glCall(glProgramUniform4iv, m_programID, _location, 1, (const GLint*)&_value);
	}

	void Program::setUniform(int _location, unsigned _value)
	{
		glCall(glProgramUniform1ui, m_programID, _location, _value);
	}

	void Program::setUniform(int _location, const glm::uvec2 & _value)
	{
		glCall(glProgramUniform2uiv, m_programID, _location, 1, (const GLuint*)&_value);
	}

	void Program::setUniform(int _location, const glm::uvec3 & _value)
	{
		glCall(glProgramUniform3uiv, m_programID, _location, 1, (const GLuint*)&_value);
	}

	void Program::setUniform(int _location, const glm::uvec4 & _value)
	{
		glCall(glProgramUniform4uiv, m_programID, _location, 1, (const GLuint*)&_value);
	}
	
} // namespace graphics
