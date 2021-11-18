#include <engine/game/game.hpp>

#include <engine/game/states/gamestate.hpp>
#include <engine/game/states/tasks/springstate.hpp>
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
#include <vector>
#include <chrono>

using namespace std::chrono_literals;
using namespace graphics;

using gameClock = std::chrono::high_resolution_clock;
using duration_t = std::chrono::duration<float>;

namespace game {

static const duration_t targetFT = std::chrono::microseconds(16667);

void Game::run() {
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

        std::vector<std::unique_ptr<GameState>> states;

        std::unique_ptr<GameState> springState = std::make_unique<SpringState>(mesh);
        states.push_back(std::move(springState));

        auto now = gameClock::now();
        auto t = now;

        duration_t dt = targetFT;

        while (!states.empty() && !glfwWindowShouldClose(window)) {
            GameState& current = *states.back();

            current.update(t.time_since_epoch().count(), dt.count());

            glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            current.draw(t.time_since_epoch().count(), dt.count());

            glfwPollEvents();
            glfwSwapBuffers(window);

            if (current.isFinished())
                states.pop_back();

            now = gameClock::now();
            if ((now - t) - targetFT > std::chrono::milliseconds(1)) {
                std::this_thread::sleep_for(
                    --std::chrono::floor<std::chrono::milliseconds>(targetFT - dt));
            }
            do {
                now = gameClock::now();
            } while (now - t < targetFT);
            t = now;
        }
    }
    Texture2DManager::clear();
    ShaderManager::clear();

    glfwDestroyWindow(window);
    glfwTerminate();
}
}  // namespace game