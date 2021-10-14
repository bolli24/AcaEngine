#pragma once

namespace graphics {
	
	/// A sampler defines how a texture is sampled (border handling and scaling)
	class Sampler
	{
	public:
		enum class Filter
		{
			POINT,
			LINEAR
		};

		enum class Border
		{
			REPEAT = 0x2901,	///< GL_REPEAT
			MIRROR = 0x8370,	///< GL_MIRRORED_REPEAT
			CLAMP  = 0x812F,	///< GL_CLAMP_TO_EDGE
			BORDER = 0x812D,	///< GL_CLAMP_TO_BORDER
		};

		Sampler(Filter _minFilter, Filter _magFilter, Filter _mipFilter, Border _borderHandling = Border::REPEAT, unsigned _maxAnisotropy = 1);
		// Move semantics are possible but the default generated ones would be wrong.
		Sampler(const Sampler&) = delete;
		Sampler(Sampler&&) = delete;
		Sampler& operator=(Sampler&&) = delete;
		Sampler& operator=(const Sampler&) = delete;
		~Sampler();

		unsigned getID() const { return m_samplerID; }

		void bind(unsigned _textureSlot) const;

	private:
		unsigned m_samplerID;
		Filter m_minFilter, m_magFilter, m_mipFilter;
		Border m_borderHandling;
		unsigned m_maxAnisotropy;
	};

} // namespace graphics
