#pragma once

#include <glm/glm.hpp>

namespace graphics {

	class Camera
	{
	public:
		// Perspective
		// @param _fov as angle
		Camera(float _fov, float _zNear, float zFar);
		// Orthogonal
		Camera(glm::vec2 _size, glm::vec2 _origin = glm::vec2(0.f), float _zNear = 0.f, float _zFar = 1.f);

		const glm::mat4& getView() const { return m_view; }
		const glm::mat4& getProjection() const { return m_projection; }
		const glm::mat4& getViewProjection() const { return m_viewProjection; }

		void setView(const glm::mat4& _view) { m_view = _view; updateMatrices(); }

		// @Return the position of the point _screenSpace on the near plane of this camera
		glm::vec3 toWorldSpace(const glm::vec2& _sceenSpace) const;
	private:
		void updateMatrices();

		glm::mat4 m_projection;
		glm::mat4 m_view;
		glm::mat4 m_viewProjection;
		glm::mat4 m_viewProjectionInv;
	};
}