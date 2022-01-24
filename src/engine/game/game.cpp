#include "game.hpp"

using namespace std::chrono_literals;
using namespace graphics;

using gameClock = std::chrono::high_resolution_clock;
using duration_t = std::chrono::duration<float>;

void add2(float& a) {
    a += 2;
}

void multiply3(float& a) {
    a *= 3;
}

void sumUp(float& a, float& sum) {
    sum += a;
}

namespace game {

static const duration_t targetFT = std::chrono::microseconds(16667);

void Game::run() {
    graphics::Device::initialize(1366, 768, false);
    GLFWwindow* window = graphics::Device::getWindow();
    glCall(glEnable, GL_DEPTH_TEST);

    /* Registry registry;

    Entity entity1 = registry.create();
    Entity entity2 = registry.create();
    Entity entity3 = registry.create();

    auto& positions = registry.getComponents<glm::vec3>();
    positions.insert(entity3, {0.1f, 1.0f, 0.5f});

    glm::vec3 pos = *positions.at(entity3);

    auto& ints = registry.getComponents<int>();
    ints.insert(entity1, 1);
    ints.insert(entity2, 2);
    ints.insert(entity3, 3);

    auto a = ints[entity1];
    auto b = ints[entity3];

    ints.erase(entity2);

    auto c = ints[entity1];
    auto d = ints[entity3];

    const auto& ints2 = registry.getComponents<int>();
    ints2.at(entity2);

    registry.erase(entity3);

    const Registry& registry2 = registry;
    registry2.getComponents<int>();*/

    {
        std::unique_ptr<GameState> dynamicstate = std::make_unique<DynamicState>(window);
        StateManager stateManager;
        stateManager.addNewState(dynamicstate);

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