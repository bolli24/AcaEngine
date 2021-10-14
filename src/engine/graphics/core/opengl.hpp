#pragma once

#include <spdlog/spdlog.h>
#include <GL/glew.h>
#include <type_traits>
#include <string>

namespace graphics {

	/// Check OpenGL for an error and report it if necessary.
	/// \details This call may stall the CPU/GPU due to glGetError().
	/// \returns true if an error occurred.
	bool GLError(const char*  _openGLFunctionName);

	/// OpenGL call with additional checks.
	template<typename FunctionType, typename... Args>
	auto _glCall(const char* _functionName, FunctionType _function, Args... _args) 
		-> typename std::enable_if<!std::is_same<decltype(_function(_args...)), void>::value, decltype(_function(_args...))>::type
	{
		if(!_function) { spdlog::error("Function '{}' not loaded!", _functionName); return 0; }
		auto ret = _function(_args...);
#ifndef NDEBUG
		GLError(_functionName);
#endif
		return ret;
	}

	/// No return overload of OpenGL call with additional checks.
	template<typename FunctionType, typename... Args>
	auto _glCall(const char* _functionName, FunctionType _function, Args... _args) 
		-> typename std::enable_if<std::is_same<decltype(_function(_args...)), void>::value, decltype(_function(_args...))>::type
	{
		if(!_function) { spdlog::error("Function '{}' not loaded!", _functionName); return; }
		_function(_args...);
#ifndef NDEBUG
		GLError(_functionName);
#endif
	}

#if defined(__GNUC__) || defined(__MINGW32__)
#define glCall(_function, ...) _glCall(#_function, _function, ##__VA_ARGS__)
#else
#define glCall(_function, ...) _glCall(#_function, _function, __VA_ARGS__)
#endif

} // namespace graphics
