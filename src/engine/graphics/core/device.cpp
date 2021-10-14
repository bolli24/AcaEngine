#include "opengl.hpp"
#include "device.hpp"
#include "shader.hpp"
#include "texture.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace graphics {

	BlendOp Device::s_blendOp[8] 				= {BlendOp::ADD};
	BlendFactor Device::s_srcFactor[8] 			= {BlendFactor::ONE};
	BlendFactor Device::s_dstFactor[8] 			= {BlendFactor::ZERO};
	glm::vec4 Device::s_blendColor				= { 0.f, 0.f, 0.f, 0.f };
	bool Device::s_blendEnable					= false;
	bool Device::s_alphaToCoverageEnable		= false;
	CullMode Device::s_cullMode 				= CullMode::BACK;
	FillMode Device::s_fillMode 				= FillMode::SOLID;
	int Device::s_stencilRef					= 0;
	int Device::s_stencilMask					= 0xff;
	StencilOp Device::s_stencilFailBack			= StencilOp::KEEP;
	StencilOp Device::s_zfailBack				= StencilOp::KEEP;
	StencilOp Device::s_passBack				= StencilOp::KEEP;
	StencilOp Device::s_stencilFailFront		= StencilOp::KEEP;
	StencilOp Device::s_zfailFront 				= StencilOp::KEEP;
	StencilOp Device::s_passFront 				= StencilOp::KEEP;
	ComparisonFunc Device::s_stencilFuncFront	= ComparisonFunc::ALWAYS;
	ComparisonFunc Device::s_stencilFuncBack	= ComparisonFunc::ALWAYS;
	bool Device::s_stencilEnable				= false;
	ComparisonFunc Device::s_zFunc				= ComparisonFunc::ALWAYS;
	bool Device::s_zEnable						= true;
	bool Device::s_zWriteEnable					= true;
	bool Device::s_scissorEnable				= true;
	GLFWwindow* Device::s_window				= nullptr;
	float Device::s_aspectRatio					= 1.f;
	std::vector<std::function<void()>> Device::s_resourceDeleters = {};

	static void ErrorCallback(int, const char* err_str)
	{
		spdlog::error("GLFW Error: {}", err_str);
	}

	bool Device::initialize(int _width, int _height, bool _fullScreen)
	{
		spdlog::info("Creating OpenGL context.");

		if (!glfwInit())
		{
			const char* msg;
			int err = glfwGetError(&msg);
			spdlog::error("Could not initialize glew. error: " + std::to_string(err) + msg);
			return false;
		}

		glfwSetErrorCallback(ErrorCallback);

		glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // We want OpenGL 4.5
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_FOCUSED, GL_FALSE);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

		spdlog::info("dimensions: {} {}: full-screen?: {}", _width, _height, _fullScreen);
		s_window = glfwCreateWindow(_width, _height, "AcaEngine", nullptr, nullptr);
		if (!s_window)
		{
			spdlog::error("Could not create window.");
			glfwTerminate();
			return false;
		}

		glfwMakeContextCurrent(s_window);
		GLenum GlewInitResult = glewInit();
		if (GLEW_OK != GlewInitResult)
		{
			if (GlewInitResult != 4) {
				spdlog::error("Could not initialize glew. with: '{}'", glewGetErrorString(GlewInitResult));
				glfwTerminate();
				return false;
			}
		}

		return true;
	}

	void Device::close()
	{
		spdlog::info("Releasing all GPU resources.");
		for (auto it = s_resourceDeleters.rbegin(); it != s_resourceDeleters.rend(); ++it)
			(*it)();

		spdlog::info("Shutting down.");
		glfwDestroyWindow(s_window);
		glfwTerminate();
	}

	void Device::registerResources(std::function<void()> _deleter)
	{
		s_resourceDeleters.emplace_back(std::move(_deleter));
	}

	// *********************************************************************** //
	glm::ivec2 Device::getBufferSize()
	{
		int w, h;
		glfwGetFramebufferSize(s_window, &w, &h);
		return glm::ivec2(w, h);
	}

	float Device::getAspectRatio()
	{
		int w, h;
		glfwGetFramebufferSize(s_window, &w, &h);
		return static_cast<float>(w) / h;
	}

	// TODO: glCall(glEnable, GL_BLEND);
	void Device::setBlendOp(BlendOp _operation, int _target)
	{
		if(s_blendOp[_target] != _operation)
		{
			glCall(glBlendEquationi, _target, unsigned(_operation));
			s_blendOp[_target] = _operation;
		}
	}

	void Device::setBlendFactor(BlendFactor _srcFactor, BlendFactor _dstFactor, int _target)
	{
		if(s_srcFactor[_target] != _srcFactor || s_dstFactor[_target] != _dstFactor)
		{
			glCall(glBlendFunci, _target, unsigned(_srcFactor), unsigned(_dstFactor));
			s_srcFactor[_target] = _srcFactor;
			s_dstFactor[_target] = _dstFactor;
		}
	}

	void Device::setBlendColor(const glm::vec4& _color)
	{
		if (s_blendColor != _color)
		{
			glCall(glBlendColor, _color.r, _color.g, _color.b, _color.a);
		}
	}

	void Device::enableBlending(bool _enable)
	{
		if(s_blendEnable != _enable)
		{
			if(_enable)
				glCall(glEnable, GL_BLEND);
			else
				glCall(glDisable, GL_BLEND);
			s_blendEnable = _enable;
		}
	}

	void Device::enableAlphaToCoverage(bool _enable)
	{
		if(s_alphaToCoverageEnable != _enable)
		{
			if(_enable)
				glCall(glEnable, GL_SAMPLE_ALPHA_TO_COVERAGE);
			else
				glCall(glDisable, GL_SAMPLE_ALPHA_TO_COVERAGE);
			s_alphaToCoverageEnable = _enable;
		}
	}

	// *********************************************************************** //
	void Device::setFillMode(FillMode _mode)
	{
		if(s_fillMode != _mode)
		{
			glCall(glPolygonMode, GL_FRONT_AND_BACK, int(_mode));
			s_fillMode = _mode;
		}
	}

	void Device::setCullMode(CullMode _mode)
	{
		if(s_cullMode != _mode)
		{
			if(_mode == CullMode::NONE)
				glCall(glDisable, GL_CULL_FACE);
			else {
				glCall(glEnable, GL_CULL_FACE);
				glCall(glCullFace, int(_mode));
			}
			s_cullMode = _mode;
		}
	}

	// *********************************************************************** //
	void Device::setStencilOp(StencilOp _stencilFailBack, StencilOp _zfailBack, StencilOp _passBack,
							  StencilOp _stencilFailFront, StencilOp _zfailFront, StencilOp _passFront)
	{
		if(s_stencilFailBack != _stencilFailBack || s_zfailBack != _zfailBack || s_passBack != _passBack)
		{
			glStencilOpSeparate(GL_BACK, unsigned(_stencilFailBack), unsigned(_zfailBack), unsigned(_passBack));
			s_stencilFailBack = _stencilFailBack;
			s_zfailBack = _zfailBack;
			s_passBack = _passBack;
		}
		if(s_stencilFailFront != _stencilFailFront || s_zfailFront != _zfailFront || s_passFront != _passFront)
		{
			glStencilOpSeparate(GL_FRONT, unsigned(_stencilFailFront), unsigned(_zfailFront), unsigned(_passFront));
			s_stencilFailFront = _stencilFailFront;
			s_zfailFront = _zfailFront;
			s_passFront = _passFront;
		}
	}

	void Device::setStencilFunc(ComparisonFunc _funcFront, ComparisonFunc _funcBack, int _ref, int _mask)
	{
		// Call both if either _ref or _mask changed
		if(s_stencilRef != _ref || s_stencilMask != _mask)
		{
			glCall(glStencilFuncSeparate, GL_BACK, unsigned(_funcBack), _ref, _mask);
			glCall(glStencilFuncSeparate, GL_FRONT, unsigned(_funcFront), _ref, _mask);
			s_stencilRef = _ref;
			s_stencilMask = _mask;
			s_stencilFuncBack = _funcBack;
			s_stencilFuncFront = _funcFront;
		} else {
			if(s_stencilFuncBack != _funcBack)
			{
				glCall(glStencilFuncSeparate, GL_BACK, unsigned(_funcBack), _ref, _mask);
				s_stencilFuncBack = _funcBack;
			}
			if(s_stencilFuncFront != _funcFront)
			{
				glCall(glStencilFuncSeparate, GL_FRONT, unsigned(_funcFront), _ref, _mask);
				s_stencilFuncFront = _funcFront;
			}
		}
	}

	void Device::enableStencil(bool _enable)
	{
		if(s_stencilEnable != _enable)
		{
			if(_enable)
				glCall(glEnable, GL_STENCIL_TEST);
			else
				glCall(glDisable, GL_STENCIL_TEST);
			s_stencilEnable = _enable;
		}
	}

	void Device::setZFunc(ComparisonFunc _zFunc)
	{
		if(s_zFunc != _zFunc)
		{
			if(_zFunc == ComparisonFunc::ALWAYS)
			{
				glCall(glDisable, GL_DEPTH_TEST);
				s_zEnable = false;
			} else {
				if(!s_zEnable) {
					glCall(glEnable, GL_DEPTH_TEST);
					s_zEnable = true;
				}
				glCall(glDepthFunc, unsigned(_zFunc));
			}
			s_zFunc = _zFunc;
		}
	}

	void Device::setZWrite(bool _enable)
	{
		if(s_zWriteEnable != _enable)
		{
			glCall(glDepthMask, _enable);
			s_zWriteEnable = _enable;
		}
	}

	// *********************************************************************** //
	void Device::scissorTest(int _x, int _y, int _width, int _height)
	{
		if(!s_scissorEnable) {
			glCall(glEnable, GL_SCISSOR_TEST);
			s_scissorEnable = true;
		}
		glCall(glScissor, _x, _y, _width, _height);
	}

	void Device::disableScissorTest()
	{
		if(s_scissorEnable) {
			glCall(glDisable, GL_SCISSOR_TEST);
			s_scissorEnable = false;
		}
	}

} // namespace graphics
