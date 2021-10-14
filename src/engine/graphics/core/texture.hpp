#pragma once

#include "sampler.hpp"
#include <cstdint>

namespace graphics {

	/// A collection of useful OpenGL texture formats
	enum class TexFormat
	{
		D32F = 0x8CAC,			// GL_DEPTH_COMPONENT32F
		D32 = 0x81A7,			// GL_DEPTH_COMPONENT32
		D24S8 = 0x88F0,			// GL_DEPTH24_STENCIL8
		D32FS8 = 0x8CAD,		// GL_DEPTH32F_STENCIL8

		RGBA32F = 0x8814,		// GL_RGBA32F
		RGBA32I = 0x8D82,		// GL_RGBA32I
		RGBA32U = 0x8D70,		// GL_RGBA32UI
		RG32F = 0x8230,			// GL_RG32F
		RG32I = 0x823B,			// GL_RG32I
		RG32U = 0x823C,			// GL_RG32UI
		R32F = 0x822E,			// GL_R32F
		R32I = 0x8235,			// GL_R32I
		R32U = 0x8236,			// GL_R32UI
		RGBA16F = 0x881A,		// GL_RGBA16F
		RGBA16I = 0x8D88,		// GL_RGBA16I
		RGBA16U = 0x8D76,		// GL_RGBA16UI
		RG16I = 0x8239,			// GL_RG16I
		RG16F = 0x822F,			// GL_RG16F
		RG16U = 0x823A,			// GL_RG16UI
		R16F = 0x822D,			// GL_R16F
		R16I = 0x8233,			// GL_R16I
		R16U = 0x8234,			// GL_R16UI
		R8 = 0x8229,			// GL_R8
		R8U = 0x8232,			// GL_R8UI
		R8I = 0x8231,			// GL_R8I
		RG8 = 0x822B,			// GL_RG8
		RG8U = 0x8238,			// GL_RG8UI
		RG8I = 0x8237,			// GL_RG8I
		RGB8 = 0x8051,			// GL_RGB8
		RGB8U = 0x8D7D,			// GL_RGB8UI
		RGB8I = 0x8D8F,			// GL_RGB8I
		RGBA8 = 0x8058,			// GL_RGBA8
		RGBA8U = 0x8D7C,		// GL_RGBA8UI
		RGBA8I = 0x8D8E,		// GL_RGBA8I

		R11G11B10F = 0x8C3A,	// GL_R11F_G11F_B10F
		RGB9E5 = 0x8C3D,		// GL_RGB9_E5
		C_BPTC_RGB = 0x8E8C,	// GL_COMPRESSED_RGBA_BPTC_UNORM
		C_BPTC_sRGB = 0x8E8D,	// GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
		C_RGTC2_RGU = 0x8DBD,	// GL_COMPRESSED_RG_RGTC2
		C_RGTC2_RGI = 0x8DBE,	// GL_COMPRESSED_SIGNED_RG_RGTC2
		C_BC5_sRGBA = 0x8C4F,	// GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
		C_BC5_RGBA = 0x83F3,	// GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	};

	/// Base class for different (2D, 3D, cube) textures.
	class Texture
	{
	public:
		unsigned getID() const { return m_textureID; }

		Texture() : m_textureID(0) {}

	protected:
		unsigned m_textureID;
	};

	/// 2D Texture class
	class Texture2D: public Texture
	{
	public:
		typedef const Texture2D* Handle;

		static Handle load(const char* _fileName, const Sampler& _sampler, bool _srgb = false);
		static void unload(Handle _texture);

		/// Create a new empty texture.
		/// \details There is a support for LDR (8-bit) textures only.
		static Texture2D* create(int _width, int _height, int _numComponents, const Sampler& _sampler);
		/// More advanced creation function with arbitrary formats
		static Texture2D* create(int _width, int _height, TexFormat _format, const Sampler& _sampler);
		/// Upload information for a single texture layer
		/// \param [in] _level Mip-map level starting with 0 for the higest resolution.
		/// \param [in] _data Pixel data with 8-bit per component and N components per pixel.
		void fillMipMap(int _level, const uint8_t* _data, bool _srgb = false);
		/// Makes the texture resident and may compute mip-maps.
		/// \details If the texture was created by load() it already is resident.
		Handle finalize(bool _createMipMaps = false, bool _makeResident = true);

		/// Bind the 2D texture to a given location.
		void bind(unsigned _slot) const;

		int getWidth() const { return m_width; }
		int getHeight() const { return m_height; } 

		/// Get the bindless texture handle.
		uint64_t getGPUHandle() const { return m_bindlessHandle; }

		/// Get the OpenGL handle.
		unsigned getID() const { return m_textureID; }

		/// Change the sampler. Not implemented!
		void setSampler(const Sampler& _sampler);
	private:
		/// Create a 2D texture without data
		Texture2D(int _width, int _height, TexFormat _format, const Sampler& _sampler);

		/// Load a 2D texture from file.
		/// \details Allowed file formats are JPG, PNG, TGA, BMP, PSD, GIF, (HDR), PIC.
		///
		///		The texture will be always resident and available for bindless access.
		/// \param [in] _textureFileName The file to load.
		/// \param [in] _sampler A sampler which will be used with this texture.
		Texture2D(const char* _textureFileName, const Sampler& _sampler, bool _srgb);

		~Texture2D();

		int m_width;
		int m_height;
		TexFormat m_format; ///< R, G, B, A?
		const Sampler* m_sampler;
		uint64_t m_bindlessHandle;
	};

	/// Class to manage multiple 2D textures.
	/// \details This texture creates a 2D texture array with dynamic allocation. The inserted
	///		textures must not have the same size. They only need to be smaller than the maximum.
	///
	///		Multiple textures can reside side by side, so sampling modes like mirror... may fail.
	///		It does not fail when the textures have the maximum resolution.
	///
	///		A single texture should not be smaller than maxSize / 32, otherwise memory will be
	///		wasted. If you want too store many very small tiles use a smaller maxSize.
/*	class TextureAtlas: public Texture
	{
	public:
		/// Create an empty texture atlas.
		TextureAtlas(int _maxWidth, int _maxHeight);
		
		struct Entry
		{
			uint16 texLayer;
			uint16 texCoordX;
			uint16 texCoordY;
			uint16 tileSize;
		};
		
		/// Add a new texture from a file to the atlas.
		/// \returns A handle to address where the texture is in the atlas.
		Entry add(const char* _textureFileName);
		
		/// Load/Reload a texture tile.
		/// \details The texture must be smaller or equal the tileSize of the location.
		void load(Entry _location, const char* _textureFileName);
		
	private:
		/// A quad tree in heap order for "buddy" memory allocation. Each node contains the maximum
		/// available space in its subtree, where 0 = max. size level.
		/// The number of root nodes is the number of texture layers in the array.
		/// The order of children is top-left, top-right, bottom-left, bottom-right.
		///	The tree has always 4^5 + 4^4 + 4^3 ... + 4^0 = 1365 uint8 elements. This allows a
		///	total depth of 5 subdivisions leading to 32x32 tiles as smallest texture partitions.
		const int TREE_SIZE = 1365;
		ScopedPtr<uint8> m_quadTree;
		int m_numRoots;
		//uint8* m_maxFreeLevel; ///< Maximum available space for each layer where 0 = max. size level.
		uint16* m_kdTree;	///< Subdivision of each layer
		int m_width;
		int m_height;
		
		/// Find a new tile location
		bool allocate(int _width, int _height, Entry& _location);
		bool recursiveAllocate(unsigned _off, unsigned _idx, int _w, int _h, Entry& _location);
		
		/// Allocate at least one more layer
		//TODO: is there a GPU GPU texture copy? Yes: glCopyImageSubData in GL4.3
		void resize();
	};*/

} // namespace graphics
