#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <chrono>
#include <engine/game/game.hpp>
#include <engine/game/registry.hpp>
#include <engine/game/states/statemanager.hpp>
#include <game/states/physicsstate.hpp>
#include <game/states/springstate.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/graphics/core/geometrybuffer.hpp>
#include <engine/graphics/core/opengl.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/utils/meshloader.hpp>
#include <iostream>
#include <thread>
#include <vector>

namespace game {
class Game {
   public:
    Game(){};
    ~Game(){};
    void run();
    static GLFWwindow* window;
};
}  // namespace game