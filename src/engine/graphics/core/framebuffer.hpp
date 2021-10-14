#pragma once

#include <array>

namespace graphics {

	class Texture;

	/// A framebuffer is a collection of textures to render on.
	/// Renderbuffers are currently not supported.
	class FrameBuffer
	{
	public:
		unsigned getID() const { return m_fboID; }

		FrameBuffer();
		~FrameBuffer();
		FrameBuffer(const FrameBuffer&) = delete;
		FrameBuffer(FrameBuffer&&) noexcept;
		FrameBuffer& operator = (const FrameBuffer &) = delete;
		FrameBuffer& operator = (FrameBuffer &&) noexcept;

		/// Binding a framebuffer to draw into its textures.
		void bind();
		void unbind();

		/// Attach a 2D texture or a layer of another texure as depth buffer.
		///	The texture format must be compatible (e.g. D32F).
		void attachDepth(Texture & _texture, int _mipLevel = 0);

		void attach(int _colorAttachmentIdx, Texture & _texture, int _mipLevel = 0);

		/// Render one of the textures with a screen filling quad.
		/// \param [in] _attachment GL_DEPTH_ATTACHMENT or GL_COLOR_ATTACHMENTi
		void show(unsigned _attachment);

		/// @brief Clear this buffers attachments if currently bound.
		void clear();
	private:
		unsigned m_fboID;
		std::array<unsigned, 16> m_drawBuffers;
		int m_maxUsedIndex;
		// Following handles are stored for the show function
		unsigned m_depthTexture;
		std::array<unsigned,16> m_colorTextures;
	};

} // namespace graphics
