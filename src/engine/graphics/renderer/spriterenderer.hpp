#pragma once

#include "sprite.hpp"
#include "../core/shader.hpp"
#include "../camera.hpp"
#include <vector>

namespace graphics {

	/// Batch renderer for sprites.
	/// \details This renderer represents exactly one pipeline state and contains one vertex and index
	///		buffer, as well as a texture-atlas.
	class SpriteRenderer
	{
	public:
		/// Initialize the renderer once.
		SpriteRenderer();

		/// \param [in] _position Position in world (x,y) and z for the "layer". You may also use
		///		the sprites in a 3D environment as billboards. Dependent on the camera z is also
		///		used for perspective division (not if orthographic).
		/// \param [in] _rotation Angle (radiants) of a rotation around the z-axis).
		/// \param [in] _scale A relative scale where 1.0 renders the sprite pixel perfect
		///		(1 screen-pixel = 1 texture-pixel) at a distance of 1.0** or with orthographic projection.
		/// \param [in] _color Color used to replace blue areas of the texture. If a == 0 it is ignored.
		/// \param [in] _animX Choose a tile in X direction when the sprite was created with more than one.
		///		E.g. a 1.4f means, that the second and third tile are interpolated with factor 0.4.
		///		If necessary a modulo operation is applied automatically.
		/// \param [in] _animY Choose a tile in Y direction when the sprite was created with more than one.
		///		E.g. a 1.4f means, that the second and third tile are interpolated with factor 0.4.
		///		If necessary a modulo operation is applied automatically.
		void draw(
			const Sprite& _sprite, 
			const glm::vec3& _position, 
			float _rotation, 
			const glm::vec2& _scale, 
			const glm::vec4& _color = glm::vec4(0.f),
			float _animX = 0.0f, 
			float _animY = 0.0f);

		/// Clear all existing instances (recommended for fully dynamic buffers)
		void clear();
		
		/// Single draw call for all instances.
		void present(const Camera& _camera);

		/// Check if there are any instances to draw
		bool isEmpty() const { return m_instances.empty(); }

	private:
#pragma pack(push, 4)

		struct SpriteInstance
		{
			Sprite::SpriteData sprite;
			glm::vec3 position;
			float rotation;
			glm::uvec2 scale; // 4 packed halfs
			glm::vec2 animation;
			glm::vec4 color;
		};
#pragma pack(pop)

		unsigned m_vao;		///< OpenGL vertex array object
		unsigned m_vbo;		///< OpenGL vertex buffer for sprites

		std::vector<SpriteInstance> m_instances;
		mutable bool m_dirty;

		Program m_program; // todo: move outside to reuse
	};

}
