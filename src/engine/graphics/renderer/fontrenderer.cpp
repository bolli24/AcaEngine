#include "fontrenderer.hpp"
#include "../../math/glmext.hpp"
#include "../core/opengl.hpp"
#include "../core/vertexformat.hpp"
#include "../core/shader.hpp"
#include "../core/device.hpp"
#include "../resources.hpp"

#include <glm/gtx/norm.hpp>
#include <string>
#include <algorithm>
#include <filesystem>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace math;
using namespace glm;

namespace graphics {

	constexpr int BASE_SIZE = 72;			///< Vertical size in pixels within largest bitmap
	constexpr int MAP_WIDTH = 1024;			///< Default width for textures (height depends on the required space)
	constexpr int MIP_LEVELS = 4;			///< Number of smaller mip-maps
	constexpr int MIP_RANGE = 1<<MIP_LEVELS;///< Size factor between largest (BASE_SIZE) and smallest map

	FontRenderer::FontRenderer() :
		m_texture(nullptr)
	{
		m_program.attach(ShaderManager::get("../resources/shader/font.vert", ShaderType::VERTEX));
		m_program.attach(ShaderManager::get("../resources/shader/font.geom", ShaderType::GEOMETRY));
		m_program.attach(ShaderManager::get("../resources/shader/font.frag", ShaderType::FRAGMENT));
		m_program.link();

		glCall(glGenVertexArrays, 1, &m_vao);
		glCall(glBindVertexArray, m_vao);
		// Create buffer without data for now
		glCall(glGenBuffers, 1, &m_vbo);
		glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);

		// 4 x uint16 for texture l, u, r and b
		glCall(glEnableVertexAttribArray, 0);
		glCall(glVertexAttribPointer, 0, 4, GLenum(PrimitiveFormat::UINT16), GL_TRUE, 32, (GLvoid*)(0));
		// 2 x half for size
		glCall(glEnableVertexAttribArray, 1);
		glCall(glVertexAttribPointer, 1, 2, GLenum(PrimitiveFormat::HALF), GL_FALSE, 32, (GLvoid*)(8));
		// 4 x uint8 for RGBA color
		glCall(glEnableVertexAttribArray, 2);
		glCall(glVertexAttribPointer, 2, 4, GLenum(PrimitiveFormat::UINT8), GL_TRUE, 32, (GLvoid*)(12));
		// 3 x float position in world space
		glCall(glEnableVertexAttribArray, 3);
		glCall(glVertexAttribPointer, 3, 3, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 32, (GLvoid*)(16));
		// 1 x float rotation
		glCall(glEnableVertexAttribArray, 4);
		glCall(glVertexAttribPointer, 4, 1, GLenum(PrimitiveFormat::FLOAT), GL_FALSE, 32, (GLvoid*)(28));

		m_sampler = std::make_unique<Sampler>(Sampler::Filter::POINT, Sampler::Filter::LINEAR, Sampler::Filter::LINEAR, Sampler::Border::CLAMP);

		spdlog::info("[graphics] Created font renderer.");
	}

	FontRenderer::~FontRenderer()
	{
		// Make sure nothing is bound to release the two buffers
		glCall(glBindVertexArray, 0);
		glCall(glBindBuffer, GL_ARRAY_BUFFER, 0);
		glCall(glDeleteBuffers, 1, &m_vbo);
		glCall(glDeleteVertexArrays, 1, &m_vao);

		if(m_texture) Texture2D::unload(m_texture);
		m_sampler = nullptr;

		spdlog::info("[graphics] Destroyed font renderer.");
	}

	FontRenderer::Handle FontRenderer::load(const char* _fileName, bool _generateCaf)
	{
		namespace fs = std::filesystem;

		FontRenderer* renderer = new FontRenderer();
		fs::path path(_fileName);
		fs::path cafPath(_fileName);
		cafPath.replace_extension("caf");
		const std::string cafPathStr = cafPath.string();
		std::string pathStr;

		if (fs::exists(cafPath))
		{
			renderer->loadCaf(cafPathStr.c_str());
		}
		else if(fs::exists(path.replace_extension("ttf")))
		{
			pathStr = path.string();
			renderer->createFont(pathStr.c_str(), reinterpret_cast<const char*>(u8" 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\u00E4\u00FC\u00F6\u00DF\u0060#'\"^_@%&|,;.:!?~+-*/(){}[]<>\u03B5\u03A9\u262F\u2713"));
			if (_generateCaf)
			{
				renderer->storeCaf(cafPathStr.c_str());
				spdlog::info("[graphics] Generating Cartographer font '{}' from '{}'.", cafPathStr, pathStr);
			}
		}
		else
		{
			spdlog::error("[graphics] Could not load either '{}' or '{}'.", cafPathStr, pathStr);
		}

		return renderer;
	}

	void FontRenderer::unload(FontRenderer::Handle _renderer)
	{
		delete _renderer;
	}

	void FontRenderer::draw(const glm::vec3& _position, const char* _text, float _size, const glm::vec4& _color, float _rotation, float _alignX, float _alignY, bool _roundToPixel)
	{
		// Compress color to vertex format
		u8vec4 color;
		color.r = (uint8)clamp(_color.a*_color.r*255.0f, 0.0f, 255.0f);
		color.g = (uint8)clamp(_color.a*_color.g*255.0f, 0.0f, 255.0f);
		color.b = (uint8)clamp(_color.a*_color.b*255.0f, 0.0f, 255.0f);
		color.a = (uint8)clamp(_color.a*255.0f, 0.0f, 255.0f);

		mat2x2 rotateAndScale = rotation(_rotation) * (_size / BASE_SIZE);

		size_t firstNewVertex = m_instances.size();
		vec2 maxCursor = renderingKernel(_position, _text, _size, rotateAndScale, _roundToPixel,
			[this, _rotation, color](const glm::vec3 & _charPosition, char32_t _char, const CharacterDef & _charMetric, float _scale) {
			// Create sprite instance
			CharacterVertex v;
			v.position = _charPosition;
			v.rotation = -_rotation;
			v.size = packHalf2x16({ _charMetric.texSize.x * _scale,_charMetric.texSize.y * _scale });
			v.color = color;
			v.texCoords = _charMetric.texCoords;
			m_instances.push_back(v);
		});

		// Compute a vector to move the whole text to its alignment
		vec2 align = rotateAndScale * vec2(maxCursor.x * _alignX, (maxCursor.y + BASE_SIZE) * _alignY - maxCursor.y);
		// Move all new characters to the reference point
		for(size_t i = firstNewVertex; i < m_instances.size(); ++i)
		{
			if(_roundToPixel)
				m_instances[i].position = vec3(floor(m_instances[i].position.x - align.x), m_instances[i].position.y - align.y, m_instances[i].position.z);
			else
				m_instances[i].position -= vec3(align, 0.0f);
		}

		m_dirty = true;
	}

	math::Rectangle FontRenderer::getBoundingBox(const vec3& _position, const char* _text, float _size, float _rotation, float _alignX, float _alignY, bool _roundToPixel)
	{
		// Convert pixel size into a scale factor and apply rotation
		mat2 rotateAndScale = rotation(_rotation) * (_size / BASE_SIZE);

		const CharacterDef * lastCharMetric = nullptr;
		vec2 maxCursor = renderingKernel(_position, _text, _size, rotateAndScale, _roundToPixel,
			[&](const glm::vec3 & _charPosition, char32_t _char, const CharacterDef & _charMetric, float _scale) {
			lastCharMetric = &_charMetric;
		});

		// Add line height and transform the last cursor position into an after char position.
		math::Rectangle out( vec2(0.0f), maxCursor );
		out.max.y += float(BASE_SIZE);
		if(lastCharMetric)
			out.max.x += lastCharMetric->texSize.x - lastCharMetric->advance/64.0f;

		// Compute a vector to move the whole text to its alignment
		vec2 align( (out.max.x - out.min.x) * _alignX,
					 out.max.y * _alignY - maxCursor.y );
		out.min -= align;
		out.max -= align;
		// Transform to world space
		out.min = rotateAndScale * out.min + vec2(_position);
		out.max = rotateAndScale * out.max + vec2(_position);

		return out;
	}

	unsigned FontRenderer::findPosition(const glm::vec2 & _findPosition, const vec2 & _textPosition, const char * _text, float _size, float _rotation, float _alignX, float _alignY, bool _roundToPixel)
	{
		// First tranform the real position into a local position. This requires the
		// knowledge of the maxCursor.
		mat2 rotateAndScale = rotation(_rotation) * (_size / BASE_SIZE);
		const CharacterDef * lastCharMetric = nullptr;
		vec2 maxCursor = renderingKernel(vec3(_textPosition, 0.0f), _text, _size, rotateAndScale, _roundToPixel,
			[&](const glm::vec3 & _charPosition, char32_t _char, const CharacterDef & _charMetric, float _scale) {
			lastCharMetric = &_charMetric;
		});
		// Add alignment (which requires the maxCursor).
		vec2 alignedCursor = _findPosition + rotateAndScale * vec2(maxCursor.x * _alignX,
			(maxCursor.y + BASE_SIZE) * _alignY - maxCursor.y);

		// TODO: multiline correct?
		alignedCursor.y -= (m_baseLineOffset + BASE_SIZE) * _size / BASE_SIZE;
		// The base offset of characters leads to fail detections at the end of the string.
		// To fix this repeat the last characters baseX, as if there is another one following.
		if(lastCharMetric)
			maxCursor.x += lastCharMetric->baseX;

		// Second run over the text to find the closest position.
		unsigned charCount = 0;
		float minDist = length2(rotateAndScale * maxCursor + _textPosition - alignedCursor);
		unsigned optPos = static_cast<unsigned>(strlen(_text));
		renderingKernel(vec3(_textPosition, 0.0f), _text, _size, rotateAndScale, _roundToPixel,
			[&](const glm::vec3 & _charPosition, char32_t _char, const CharacterDef & _charMetric, float _scale) {
			float d = length2(vec2(_charPosition) - alignedCursor);
			if(d < minDist)
			{
				minDist = d;
				optPos = charCount;
			}
			charCount++;
		});

		return optPos;
	}

	void FontRenderer::clearText()
	{
		m_instances.clear();
	}

	void FontRenderer::present(const Camera& _camera)
	{
		m_program.use();
		m_program.setUniform(0, _camera.getViewProjection()); //todo: move this into an uniform buffer object?

		// Update buffer
		// TODO: ringbuffer
		if(m_dirty)
		{
			glCall(glBindBuffer, GL_ARRAY_BUFFER, m_vbo);
			glCall(glBufferData, GL_ARRAY_BUFFER, m_instances.size() * sizeof(CharacterVertex), m_instances.data(), GL_DYNAMIC_DRAW);
			m_dirty = false;
		}
		// Enable alpha blending (permultiplied)
		//glCall(glEnable, GL_BLEND);
		Device::setBlendOp(BlendOp::ADD);
		Device::setBlendFactor(BlendFactor::ONE, BlendFactor::INV_SRC_ALPHA);
		// Draw
		m_texture->bind(0);
		glCall(glBindVertexArray, m_vao);
		glCall(glDrawArrays, GL_POINTS, 0, static_cast<GLsizei>(m_instances.size()));
	}

	void FontRenderer::createFont(const char* _fontFile, const char* _characters)
	{
	//	HRClock clock;

		// Init library
		FT_Library ftlib;
		if( FT_Init_FreeType( &ftlib ) )
		{
			spdlog::error("[graphics] Cannot initalize freetype!");
			return;
		}

		// Load regular face (oposed to italic...)
		FT_Face fontFace;
		if( FT_New_Face(ftlib, _fontFile, 0, &fontFace) )
		{
			spdlog::error("[graphics] Could not open font!");
			return;
		}

		// Create MipMaps beginning with the smallest defined one (halfing may round, doubling doesn't).
		int mipFactor = MIP_RANGE, mipLevel = MIP_LEVELS;
		std::vector<uint8> map[MIP_LEVELS+1];
		int texHeight = MIP_RANGE * createMap(map[mipLevel--], _characters, fontFace, BASE_SIZE / MIP_RANGE, MAP_WIDTH / MIP_RANGE, MIP_RANGE / mipFactor);
		if(!texHeight)
		{
			spdlog::error("[graphics] Cancel font creation because basic mipmap could not be created!");
			return;
		}
		while(mipFactor > 1)
		{
			mipFactor /= 2;
			createMagMap(map[mipLevel--], fontFace, BASE_SIZE / mipFactor, MAP_WIDTH / mipFactor, texHeight / mipFactor, MIP_RANGE / mipFactor, MIP_RANGE / mipFactor);
		}
		// Upload texture data
		// Add dummy mip-maps at the lower end
		Texture2D* texture = Texture2D::create(MAP_WIDTH, texHeight, 1, *m_sampler);
		mipLevel = 0;
		while((1<<mipLevel) <= max(MAP_WIDTH, texHeight))
		{
			texture->fillMipMap(mipLevel, map[min(3, mipLevel)].data());
			++mipLevel;
		}
		m_texture = texture->finalize(false, false);
		normalizeCharacters(fontFace, _characters);

		// Free resources
		if( FT_Done_FreeType(ftlib) )
		{
			spdlog::error("[graphics] Cannot free all Freetype resources!");
			return;
		}

	//	double time = clock.deltaTime();
		spdlog::info("[graphics] Loaded font '{}'.", _fontFile);
	}

#pragma pack(push, 1)
	struct CafHeader
	{
		unsigned numCharacters;
		uint16 textureWidth;
		uint16 textureHeight;
		//uint8 numMipMaps;
		int8 baseLineOffset;
	};
	struct CafCharacter
	{
		char32_t unicode;
		uint16 advance;
		int8 baseX, baseY;
		glm::u16vec4 texCoords;
		glm::vec2 texSize;
		uint16 kerningTableSize;
	};
#pragma pack(pop)

	void FontRenderer::storeCaf(const char* _fontFile)
	{
	//	HRClock clock;

		FILE* file = fopen(_fontFile, "wb");
		if(!file) { spdlog::error("[graphics] Cannot open file ", _fontFile, " for writing!"); return; }

		// Header with number of characters, global metrics and texture dimensions.
		CafHeader header;
		header.numCharacters = (unsigned)m_chars.size();
		header.textureWidth = m_texture->getWidth();
		header.textureHeight = m_texture->getHeight();
		//header.numMipMaps = MIP_LEVELS;
		header.baseLineOffset = m_baseLineOffset;
		fwrite(&header, sizeof(CafHeader), 1, file);

		// Character metrics
		for(auto& charEntry : m_chars)
		{
			CafCharacter c;
			c.unicode = charEntry.first;
			c.advance = charEntry.second.advance;
			c.baseX = charEntry.second.baseX;
			c.baseY = charEntry.second.baseY;
			c.texCoords = charEntry.second.texCoords;
			c.texSize = charEntry.second.texSize;
			c.kerningTableSize = (uint16)charEntry.second.kerning.size();
			fwrite(&c, sizeof(CafCharacter), 1, file);
			for(auto& kerningPair : charEntry.second.kerning)
				fwrite(&kerningPair, sizeof(CharacterDef::KerningPair), 1, file);
		}

		// Texture
		unsigned w = m_texture->getWidth();
		unsigned h = m_texture->getHeight();
		std::vector<uint8> buffer(w * h);
		// for all mip levels
		int mipLevel = 0;
		while(w > 1 || h > 1)
		{
			glGetTextureImage(m_texture->getID(), mipLevel++, GL_RED, GL_UNSIGNED_BYTE, (GLsizei)buffer.size(), buffer.data());
			fwrite(buffer.data(), 1, w * h, file);
			w = max(1u, w/2);
			h = max(1u, h/2);
		}

		fclose(file);

	//	double time = clock.deltaTime();
	//	logInfo("[graphics] Stored font '", _fontFile, "' in ", time, " ms.");
	}

	void FontRenderer::loadCaf(const char* _fontFile)
	{
	//	HRClock clock;
		// Clear for multiple load calls
		m_chars.clear();
		if(m_texture) Texture2D::unload(m_texture);

		FILE* file = fopen(_fontFile, "rb");
		if(!file) { spdlog::error("[graphics] Cannot open file ", _fontFile, " for reading!"); return; }

		CafHeader header;
		fread(&header, sizeof(CafHeader), 1, file);
		m_baseLineOffset = header.baseLineOffset;

		// Load character metrics
		for(unsigned i = 0; i < header.numCharacters; ++i)
		{
			CafCharacter c;
			CharacterDef def;
			fread(&c, sizeof(CafCharacter), 1, file);
			def.advance = c.advance;
			def.baseX = c.baseX;
			def.baseY = c.baseY;
			def.texCoords = c.texCoords;
			def.texSize = c.texSize;
			uint16 kerningTableSize = c.kerningTableSize;
			def.kerning.reserve(kerningTableSize);
			for(unsigned j = 0; j < kerningTableSize; ++j)
			{
				CharacterDef::KerningPair kerningPair;
				fread(&kerningPair, sizeof(CharacterDef::KerningPair), 1, file);
				def.kerning.push_back(kerningPair);
			}
			m_chars.emplace(c.unicode, std::move(def));
		}

		// Upload texture data
		Texture2D* texture = Texture2D::create(header.textureWidth, header.textureHeight, 1, *m_sampler);
		int mipLevel = 0;
		unsigned w = header.textureWidth * 2;
		unsigned h = header.textureHeight * 2;
		std::vector<uint8> buffer(w * h / 4);
		while(w > 1 || h > 1)
		{
			w = max(1u, w/2);
			h = max(1u, h/2);
			fread(buffer.data(), 1, w * h, file);
			texture->fillMipMap(mipLevel++, buffer.data());
		}
		m_texture = texture->finalize(false, false);

		fclose(file);

	//	double time = clock.deltaTime();
	//	logInfo("[graphics] Loaded font '", _fontFile, "' in ", time, " ms.");
	}

	/// Copy a rectangle into the larger target rectangle
	/// \param [in] _left left position in the target map
	/// \param [in] _top top position in the target map
	static void copyGlyph(std::vector<uint8>& _target, const FT_Bitmap& _glyph, int _left, int _top, int _mapWidth)
	{
		for(unsigned y=0; y<_glyph.rows; ++y)
		{
			for(unsigned x=0; x<_glyph.width; ++x)
			{
				unsigned idx = _left+x + _mapWidth*(_top+y);
				if(idx < _target.size())
					_target[idx] = _glyph.buffer[x+_glyph.width*(_glyph.rows-1-y)];
			}
		}
	}

	int FontRenderer::createMap(std::vector<uint8>& _target, const char* _characters, const FT_Face _fontFace, int _fontSize, int _mapWidth, int _padding)
	{
		unsigned mapHeight = _padding*2 + _fontSize;
		_target.resize(_mapWidth * mapHeight);

		// Set a fixed font size
		if( FT_Set_Pixel_Sizes(_fontFace, 0, _fontSize) )
		{
			spdlog::error("Cannot set size to ", _fontSize, "px!");
			return 0;
		}

		// Padding is expected to be a potence of 2 including 1
		int offX = _padding;
		int offY = _padding;
		int maxY = 0;
		for(char32_t c = getNext(&_characters); c; c = getNext(&_characters))
		{
			int idx = FT_Get_Char_Index(_fontFace, c);
			FT_Load_Glyph(_fontFace, idx, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
			// Get direct access pointer to the glyph slot
			FT_Bitmap& bmp = _fontFace->glyph->bitmap;
			if(offX + bmp.width + _padding >= (unsigned)_mapWidth)
			{
				offY += maxY + _padding;
				offX = _padding;
				maxY = 0;
			}
			if(offY + bmp.rows + _padding > mapHeight)
			{
				mapHeight += _padding + _fontSize;
				_target.resize(_mapWidth * mapHeight);
			}
			// Add metrics to the hashmap.
			auto charEntry = m_chars.find(c);
			if(charEntry == m_chars.end())
			{
				CharacterDef bmpChar;
				// Store absolute coordinates now and normalize to texture coordinates later because
				// the total height is unknown.
				bmpChar.texSize.x = (float)bmp.width;
				bmpChar.texSize.y = (float)bmp.rows;
				bmpChar.texCoords.x = offX;
				bmpChar.texCoords.y = offY;
				charEntry = m_chars.emplace(c, bmpChar).first;
			} else {
				// Character was already there -> do nothing
				spdlog::error("Input string contains a character repetition of '{}'.", static_cast<int>(c));
				continue;
			}
			// While copying center the glyph within its padding width
			copyGlyph(_target, bmp, offX, offY, _mapWidth);
			offX += bmp.width + _padding;
			maxY = max((int)bmp.rows, maxY);
		}

		return mapHeight;
	}

	void FontRenderer::createMagMap(std::vector<unsigned char>& _target, const FT_Face _fontFace, int _fontSize, int _mapWidth, int _mapHeight, int _padding, int _mipFactor)
	{
		_target.resize(_mapWidth * _mapHeight);

		// Set a fixed font size
		if( FT_Set_Pixel_Sizes(_fontFace, 0, _fontSize) )
		{
			spdlog::error("Cannot set size to ", _fontSize, "px!");
			return;
		}

		for(auto& cEntry : m_chars)
		{
			int idx = FT_Get_Char_Index(_fontFace, cEntry.first);
			FT_Load_Glyph(_fontFace, idx, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
			// Get direct access pointer to the glyph slot
			FT_Bitmap& bmp = _fontFace->glyph->bitmap;
			int targetCharWidth = int(cEntry.second.texSize.x)*_mipFactor;
			int centeringOff = (targetCharWidth - (int)bmp.width) / 2;
			// While copying center the glyph within its padding width
			copyGlyph(_target, bmp, int(cEntry.second.texCoords.x)*_mipFactor + centeringOff,
				int(cEntry.second.texCoords.y)*_mipFactor, _mapWidth);
		}
	}

	void FontRenderer::normalizeCharacters(const FT_Face _fontFace, const char* _characters)
	{
		//int8 baseLineOffset = 127;
		m_baseLineOffset = 10000;
		float avgBaseLine = 0.0f;
		for(auto& cEntry : m_chars)
		{
			// Resize texture sprite to a slim fit
			int idx = FT_Get_Char_Index(_fontFace, cEntry.first);
			FT_Load_Glyph(_fontFace, idx, FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
			cEntry.second.texCoords.x = cEntry.second.texCoords.x * MIP_RANGE + (int(cEntry.second.texSize.x)*MIP_RANGE - (int)_fontFace->glyph->bitmap.width) / 2;
			cEntry.second.texCoords.y *= MIP_RANGE;
			cEntry.second.texSize.y = (float)_fontFace->glyph->bitmap.rows;
			cEntry.second.texSize.x = (float)_fontFace->glyph->bitmap.width;
			cEntry.second.baseX = -MIP_RANGE + _fontFace->glyph->bitmap_left;
			cEntry.second.baseY = -MIP_RANGE - _fontFace->glyph->bitmap.rows + _fontFace->glyph->bitmap_top;
			cEntry.second.advance = (uint16)_fontFace->glyph->advance.x;
			cEntry.second.texCoords.x = (cEntry.second.texCoords.x * 0xffff) / m_texture->getWidth();
			cEntry.second.texCoords.y = (cEntry.second.texCoords.y * 0xffff) / m_texture->getHeight();
			cEntry.second.texCoords.z = cEntry.second.texCoords.x + int(cEntry.second.texSize.x * 0xffff / m_texture->getWidth());
			cEntry.second.texCoords.w = cEntry.second.texCoords.y + int(cEntry.second.texSize.y * 0xffff / m_texture->getHeight());
			// Lift all characters such that the new base-line is entirely below the text (for alignment)
			if(cEntry.second.baseY < m_baseLineOffset)
				m_baseLineOffset = cEntry.second.baseY;
			avgBaseLine += cEntry.second.baseY;
			// Build a kerning table for all characters with a special spacing
			const char* charIt = _characters;
			for(char32_t c = getNext(&charIt); c; c = getNext(&charIt))
			{
				FT_Vector kerning;
				if(!FT_Get_Kerning(_fontFace, FT_Get_Char_Index(_fontFace, cEntry.first), FT_Get_Char_Index(_fontFace, c), FT_KERNING_UNFITTED, &kerning) &&
					kerning.x != 0)
				{
					CharacterDef::KerningPair p;
					p.character = c;
					p.kern = (int16)kerning.x;
					cEntry.second.kerning.push_back(p);
				}
			}
			// Sort kerning table for faster access
			if(cEntry.second.kerning.size() > 0)
				std::sort(cEntry.second.kerning.begin(), cEntry.second.kerning.end());
		}
		avgBaseLine /= m_chars.size();

		for(auto& cEntry : m_chars)
			cEntry.second.baseY -= m_baseLineOffset;
	}

	char32_t FontRenderer::getNext(const char** _textit)
	{
		char32_t c;
		const char* ptr = *_textit;
		// First bit set -> multibyte
		if(ptr[0] & 0x80)
		{
			if((ptr[0] & 0xf8) == 0xf0)
			{
				// All three following characters must start with 10xxx...
				ASSERT(((ptr[1] & 0xc0) == 0x80) && ((ptr[2] & 0xc0) == 0x80) && ((ptr[3] & 0xc0) == 0x80), "Invalid utf8 codepoint!");
				// Take all the xxxx from the bytes and put them into one character
				c = ((ptr[0] & 0x07) << 18) | ((ptr[1] & 0x3f) << 12) | ((ptr[2] & 0x3f) << 6) | (ptr[3] & 0x3f);
				*_textit = ptr + 4;
			} else if((ptr[0] & 0xf0) == 0xe0) {
				ASSERT(((ptr[1] & 0xc0) == 0x80) && ((ptr[2] & 0xc0) == 0x80), "Invalid utf8 codepoint!");
				c = ((ptr[0] & 0x0f) << 12) | ((ptr[1] & 0x3f) << 6) | (ptr[2] & 0x3f);
				*_textit = ptr + 3;
			} else if((ptr[0] & 0xe0) == 0xc0) {
				ASSERT(((ptr[1] & 0xc0) == 0x80), "Invalid utf8 codepoint!");
				c = ((ptr[0] & 0x1f) << 6) | (ptr[1] & 0x3f);
				*_textit = ptr + 2;
			} else {
				spdlog::warn("Invalid utf8 codepoint! The first byte of an utf8 character cannot start with 10xxx.");
				c = 0;
			}
		} else {
			c = ptr[0];
			*_textit = ptr + 1;
		}
		return c;
	}

	glm::vec2 FontRenderer::renderingKernel(const glm::vec3 & _position, const char * _text, float _size, const glm::mat2& _transformation, bool _roundToPixel, PlaceCharacterFunc _place)
	{
		// Convert pixel size into a scale factor
		float scale = _size / BASE_SIZE;

		vec3 position = _position;
		if(_roundToPixel)
			position.y = roundf(position.y - m_baseLineOffset * scale) + m_baseLineOffset * scale;

		// Create a cursor in text-space (without rotation and scale)
		vec2 cursor(0.0f);
		// Avoid the offset for the first character
		bool firstInLine = true;
		vec2 maxCursor( cursor );
		auto lastC = m_chars.end();

		for(char32_t c = getNext(&_text); c; c = getNext(&_text))
		{
			if(c == '\n')
			{
				maxCursor.y += BASE_SIZE;
				cursor.x = 0.0f;
				cursor.y -= BASE_SIZE;
				lastC = m_chars.end();
				firstInLine = true;
			} else {
				auto charEntry = m_chars.find(c);
				if(charEntry != m_chars.end())
				{
					// Add kerning
					if(lastC != m_chars.end())
					{
						CharacterDef::KerningPair p; p.character = charEntry->first;
						//int s = lastC->second.kerning.size();
						auto it = std::lower_bound(lastC->second.kerning.begin(), lastC->second.kerning.end(), p);
						if(it != lastC->second.kerning.end() && it->character == p.character)
							cursor.x += it->kern / 64.0f;
					}
					if(firstInLine)
						cursor.x -= charEntry->second.baseX;
					if(_roundToPixel)
						cursor.x = roundf((cursor.x + charEntry->second.baseX) * scale) / scale - charEntry->second.baseX;
					vec2 charPos = _transformation * (cursor + vec2(charEntry->second.baseX, charEntry->second.baseY));
					_place(position + vec3(charPos, 0.0f), c, charEntry->second, scale);
					//maxCursor.x = max(maxCursor.x, cursor.x + charEntry->second.texSize.x);
					cursor.x += charEntry->second.advance / 64.0f;
					maxCursor.x = max(maxCursor.x, cursor.x);
				}
				lastC = charEntry;
				firstInLine = false;
			}
		}

		return maxCursor;
	}

} // namespace graphics
