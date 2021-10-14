#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/utils/meshloader.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/utils/meshloader.hpp>
#include <engine/graphics/core/opengl.hpp>
#include <GLFW/glfw3.h>

#include <thread>

// CRT's memory leak detection
#ifndef NDEBUG 
#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

using namespace std::chrono_literals;

int main(int argc, char *argv[])
{
#ifndef NDEBUG 
#if defined(_MSC_VER)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//	_CrtSetBreakAlloc(2760);
#endif
#endif

	graphics::Device::initialize(1366, 768,false);
	GLFWwindow* window = graphics::Device::getWindow();
	input::InputManager::initialize(window);

	graphics::glCall(glClearColor,0.f, 1.f, 0.f, 1.f);

	while (!glfwWindowShouldClose(window) && !input::InputManager::isKeyPressed(input::Key::ESCAPE))
	{
		graphics::glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glfwPollEvents();
		glfwSwapBuffers(window);
		std::this_thread::sleep_for(16ms);
	}

	utils::MeshLoader::clear();
	graphics::Device::close();
	return EXIT_SUCCESS;
}
