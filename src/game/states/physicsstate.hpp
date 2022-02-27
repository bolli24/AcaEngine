#pragma once

#include <engine/game/collisionsystem.hpp>
#include <engine/game/components.hpp>
#include <engine/game/registry.hpp>
#include <engine/game/rendersystem.hpp>
#include <engine/game/states/gamestate.hpp>
#include <engine/graphics/camera.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/graphics/core/lightsystem.hpp>
#include <engine/graphics/core/sampler.hpp>
#include <engine/graphics/core/texture.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/resources.hpp>
#include <engine/math/convexhull.hpp>

// clang-format off
#include <GLFW/glfw3.h>
// clang-format on

#include <vector>

using namespace graphics;
class PhysicsState : public GameState {
   public:
    void update(float _time, float _deltaTime);
    void draw(float _time, float _deltaTime);
    void onPause(){};
    void onResume(){};
    bool isFinished() { return false; };
    PhysicsState(GLFWwindow* _window);
    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

   private:
    Camera camera;
    Sampler sampler;
    MeshRenderer meshRenderer;
    glm::vec3 cameraPosition;
    GLFWwindow* window;

    Mesh mesh;
    Texture2D::Handle texture;
    Registry registry;
};