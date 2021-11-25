#include <chrono>
#include <engine/game/game.hpp>
#include <engine/game/states/statemanager.hpp>
#include <engine/game/states/tasks/rotationstate.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/graphics/core/geometrybuffer.hpp>
#include <engine/graphics/core/opengl.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/utils/meshloader.hpp>

// clang-format off
#include <GLFW/glfw3.h>
// clang-format on

#include <iostream>
#include <thread>
#include <vector>

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
        std::unique_ptr<GameState> rotationState = std::make_unique<RotationState>();
        StateManager stateManager;
        stateManager.addNewState(rotationState);

        auto now = gameClock::now();
        auto t = now;

        duration_t dt = targetFT;

        while (!stateManager.states.empty() && !glfwWindowShouldClose(window)) {
            stateManager.current->update(t.time_since_epoch().count(), dt.count());

            glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            stateManager.current->draw(t.time_since_epoch().count(), dt.count());

            glfwPollEvents();
            glfwSwapBuffers(window);

            if (stateManager.current->isFinished())
                stateManager.deleteLastState();

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