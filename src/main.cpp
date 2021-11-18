#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/utils/meshloader.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/graphics/core/geometrybuffer.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/graphics/core/opengl.hpp>
#include <engine/graphics/resources.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

#include <thread>

// CRT's memory leak detection
#ifndef NDEBUG
#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

using namespace std::chrono_literals;
using namespace graphics;

int main(int argc, char* argv[]) {
#ifndef NDEBUG
#if defined(_MSC_VER)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //	_CrtSetBreakAlloc(2760);
#endif
#endif

    graphics::Device::initialize(1366, 768, false);
    GLFWwindow* window = graphics::Device::getWindow();
    glCall(glEnable, GL_DEPTH_TEST);

    {
        Camera camera(90.0f, 0.1f, 100.0f);
        camera.setView(glm::lookAt(
            glm::vec3(0, 0, 2),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)));

        auto meshData = utils::MeshLoader::get("models/sphere.obj");
        glm::mat4 meshTransform = glm::mat4(1.0f);
        glm::mat4 meshProjection = meshTransform * camera.getViewProjection();

        Mesh mesh(*meshData);

        static const Sampler sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                                     Sampler::Filter::LINEAR, Sampler::Border::MIRROR);
        auto texture = Texture2DManager::get("textures/planet1.png", sampler);
        texture->bind(0);

        const auto* fragmentShader = ShaderManager::get("shader/demo.frag", ShaderType::FRAGMENT);
        const auto* vertexShader = ShaderManager::get("shader/demo.vert", ShaderType::VERTEX);

        graphics::Program program;
        program.attach(vertexShader);
        program.attach(fragmentShader);
        program.link();
        program.setUniform(1, meshProjection);
        program.use();

        while (!glfwWindowShouldClose(window)) {
            glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            mesh.draw();

            glfwPollEvents();
            glfwSwapBuffers(window);
        }

        Texture2DManager::clear();
        ShaderManager::clear();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
