#include "../utils/resourcemanager.hpp"
#include "core/device.hpp"
#include "core/texture.hpp"
#include "core/shader.hpp"
#include "renderer/fontrenderer.hpp"

namespace graphics {
	/// The manager to load 2D textures to avoid loading the same texture twice.
	using Texture2DManager = utils::ResourceManager<Texture2D, Device>;

	using ShaderManager = utils::ResourceManager<Shader, Device>;

	// todo split into Font + Renderer and use ResourceManager only for Font
	using FontManager = utils::ResourceManager<FontRenderer, Device>;
}