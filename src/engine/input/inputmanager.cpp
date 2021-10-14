#include "inputmanager.hpp"
#include <GLFW/glfw3.h>

namespace input {

	GLFWwindow* InputManager::s_window = nullptr;
	std::array<ActionState, static_cast<size_t>(Key::Count)> InputManager::m_keyStates{};
	std::array<ActionState, static_cast<size_t>(Key::Count)> InputManager::m_buttonStates{};

	void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		switch (action)
		{
		case GLFW_PRESS: m_keyStates[key] = ActionState::PRESSED; break;
		case GLFW_RELEASE: m_keyStates[key] = ActionState::RELEASED; break;
		}
	}

	void InputManager::buttonCallback(GLFWwindow* window, int button, int action, int mods)
	{
		switch (action)
		{
		case GLFW_PRESS: m_buttonStates[button] = ActionState::PRESSED; break;
		case GLFW_RELEASE: m_buttonStates[button] = ActionState::RELEASED; break;
		}
	}

	void InputManager::initialize(GLFWwindow* _window)
	{
		s_window = _window;
		glfwSetKeyCallback(_window, keyCallback);
		glfwSetMouseButtonCallback(_window, buttonCallback);
	}

	bool InputManager::isKeyPressed(Key _key)
	{
		return glfwGetKey(s_window, static_cast<int>(_key)) == GLFW_PRESS;
	}

	const char* InputManager::getKeyName(Key _key)
	{
		return glfwGetKeyName(static_cast<int>(_key), 0);
	}

	ActionState InputManager::getKeyState(Key _key)
	{
		return m_keyStates[static_cast<int>(_key)];
	}

	// *************************************************** //
	bool InputManager::isButtonPressed(MouseButton _key)
	{
		return glfwGetMouseButton(s_window, static_cast<int>(_key)) == GLFW_PRESS;
	}

	glm::vec2 InputManager::getCursorPos()
	{
		double x, y;
		glfwGetCursorPos(s_window, &x, &y);

		return { x,y };
	}

	ActionState InputManager::getButtonState(MouseButton _button)
	{
		return m_keyStates[static_cast<int>(_button)];
	}

	void InputManager::setCursorMode(CursorMode _mode)
	{
		int mode = 0;
		switch (_mode)
		{
		case CursorMode::NORMAL: mode = GLFW_CURSOR_NORMAL; break;
		case CursorMode::HIDDEN: mode = GLFW_CURSOR_HIDDEN; break;
		case CursorMode::DISABLED: mode = GLFW_CURSOR_DISABLED; break;
		}

		glfwSetInputMode(s_window, GLFW_CURSOR, mode);
	}

	constexpr static ActionState UPDATE_MASK[] = { ActionState::UP, ActionState::DOWN, ActionState::UP, ActionState::DOWN };

	void InputManager::updateKeyStates()
	{
		for (ActionState& as : m_keyStates)
			as = UPDATE_MASK[static_cast<size_t>(as)];
		for (ActionState& as : m_buttonStates)
			as = UPDATE_MASK[static_cast<size_t>(as)];
	}
}