#include "game.hpp"

using namespace std::chrono_literals;
using namespace graphics;

using gameClock = std::chrono::high_resolution_clock;
using duration_t = std::chrono::duration<float>;

namespace game {

GLFWwindow* Game::window;

static const duration_t targetFT = std::chrono::microseconds(16667);

void Game::run() {
    graphics::Device::initialize(1920, 1080, false);
    window = graphics::Device::getWindow();
    glCall(glEnable, GL_DEPTH_TEST);

    {
        Sampler sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                        Sampler::Filter::LINEAR, Sampler::Border::MIRROR);

        StateManager stateManager(&sampler);
        std::unique_ptr<GameState> physicsState = std::make_unique<PhysicsState>();
        stateManager.addNewState(physicsState);

        auto now = gameClock::now();
        auto t = now;

        duration_t dt = targetFT;
        glfwSetKeyCallback(window, GameState::keyCallbackDispatch);
        glfwSetMouseButtonCallback(window, GameState::mouseButtonCallbackDispatch);

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