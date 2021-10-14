#pragma once

#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include <vector>
#include <functional>

struct GLFWwindow;

namespace graphics {

	enum class BlendOp
	{
		ADD = 0x8006,				///< GL_FUNC_ADD
		SUBTRACT = 0x800A,			///< GL_FUNC_SUBTRACT
		REVERSE_SUBTRACT = 0x800B,	///< GL_FUNC_REVERSE_SUBTRACT
		MIN = 0x8007,				///< GL_MIN
		MAX = 0x8008,				///< GL_MAX
	};

	enum class BlendFactor {
		ZERO = 0,					///< GL_ZERO
		ONE = 1,					///< GL_ONE
		SRC_ALPHA = 0x0302,			///< GL_SRC_ALPHA
		INV_SRC_ALPHA = 0x0303,		///< GL_ONE_MINUS_SRC_ALPHA
		DST_ALPHA = 0x0304,			///< GL_DST_ALPHA
		INV_DST_ALPHA = 0x0305,		///< GL_ONE_MINUS_DST_ALPHA
		SRC_COLOR = 0x0300,			///< GL_SRC_COLOR
		INV_SRC_COLOR = 0x0301,		///< GL_ONE_MINUS_SRC_COLOR
		DST_COLOR = 0x0306,			///< GL_DST_COLOR
		INV_DST_COLOR = 0x0307,		///< GL_ONE_MINUS_DST_COLOR,
		// only available with GL_ARB_imaging
		CONST_COLOR = 0x8001,		///< GL_CONSTANT_COLOR
		INV_CONST_COLOR = 0x8002,	///< GL_ONE_MINUS_CONSTANT_COLOR
		CONST_ALPHA = 0x8003,		///< GL_CONSTANT_ALPHA
		INV_CONST_ALPHA = 0x8004,	///< GL_ONE_MINUS_CONSTANT_ALPHA
	};
	
	enum class CullMode
	{
		FRONT = 0x0404,			///< GL_FRONT
		BACK = 0x0405,			///< GL_BACK
		NONE
	};

	enum class FillMode
	{
		SOLID = 0x1B02,			///< GL_FILL
		WIREFRAME = 0x1B01,		///< GL_LINE
	};

	enum struct ComparisonFunc {
		NEVER = 0x0200,			///< GL_NEVER Fail always
		LESS = 0x0201,			///< GL_LESS Comparison function less '<'
		EQUAL = 0x0202,			///< GL_EQUAL Comparison function equal '=='
		LEQUAL = 0x0203,		///< GL_LEQUAL Comparison function less equal '<='
		GREATER = 0x0204,		///< GL_GREATER Comparison function greater '>'
		GEQUAL = 0x0206,		///< GL_GEQUAL Comparison function greater equal '>='
		NOTEQUAL = 0x0205,		///< GL_NOTEQUAL Comparison function not equal '!='
		ALWAYS = 0x0207			///< GL_ALWAYS Disables z-Testing / stencil test
	};

	enum struct StencilOp {
		KEEP = 0x1E00,			///< GL_KEEP
		ZERO = 0,				///< GL_ZERO
		REPLACE = 0x1E01,		///< GL_REPLACE
		INC_WARP = 0x8507,		///< GL_INCR_WRAP
		DEC_WARP = 0x8508,		///< GL_DECR_WRAP
		INC_SAT = 0x1E02,		///< GL_INCR
		DEC_SAT = 0x1E03,		///< GL_DECR
		INVERT = 0x150A,		///< GL_INVERT
	};

	// OpenGL state machine.
	class Device
	{
	public:
		/// Creates the context and window.
		static bool initialize(int _width, int _height, bool _fullScreen);
		static void close();
		/// Registers a function to be called by close() bevor the device shuts down.
		static void registerResources(std::function<void()> _deleter);

		static GLFWwindow* getWindow() { return s_window; }
		static glm::ivec2 getBufferSize();
		static float getAspectRatio();
		
		// Optimized state changes (only calls gl.. if necessary)
		/// \param _operation A valid blend operation. The default is ADD for all targets
		/// \param _target The render target when MRT are used.
		static void setBlendOp(BlendOp _operation, int _target = 0);
		static void setBlendFactor(BlendFactor _srcFactor, BlendFactor _dstFactor, int _target = 0);
		static void setBlendColor(const glm::vec4& _color);
		static void enableBlending(bool _enable);
		static void enableAlphaToCoverage(bool _enable);
		
		static void setFillMode(FillMode _mode);
		static void setCullMode(CullMode _mode);
		
		/// Defaults for all is KEEP
		static void setStencilOp(StencilOp _stencilFailBack, StencilOp _zfailBack, StencilOp _passBack,
								 StencilOp _stencilFailFront, StencilOp _zfailFront, StencilOp _passFront);
		/// \param [in] _ref Reference value in [0,255]. Default is 0.
		static void setStencilFunc(ComparisonFunc _funcFront, ComparisonFunc _funcBack, int _ref = 0, int _mask = 0xff);
		static void enableStencil(bool _enable);
		static void setZFunc(ComparisonFunc _zFunc);
		static void setZWrite(bool _enable);

		/// Enables scissor test for the given rectangle in pixel coordinates.
		/// \param [in] _x Lower-left pixel position.
		static void scissorTest(int _x, int _y, int _width, int _height);
		static void disableScissorTest();
		
	private:
		static BlendOp s_blendOp[8];
		static BlendFactor s_srcFactor[8], s_dstFactor[8];
		static glm::vec4 s_blendColor;
		static bool s_blendEnable;
		static bool s_alphaToCoverageEnable;
		static CullMode s_cullMode;
		static FillMode s_fillMode;
		static int s_stencilRef;
		static int s_stencilMask;
		static StencilOp s_stencilFailBack, s_zfailBack, s_passBack;
		static StencilOp s_stencilFailFront, s_zfailFront, s_passFront;
		static ComparisonFunc s_stencilFuncFront, s_stencilFuncBack;
		static bool s_stencilEnable;
		static ComparisonFunc s_zFunc;
		static bool s_zEnable;
		static bool s_zWriteEnable;
		static bool s_scissorEnable;
		static GLFWwindow* s_window;
		static float s_aspectRatio;
		static std::vector<std::function<void()>> s_resourceDeleters;
	};
	
} // namespace graphics
