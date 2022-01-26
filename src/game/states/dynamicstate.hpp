#pragma once

#include <engine/game/registry.hpp>
#include <engine/game/states/gamestate.hpp>
#include <engine/game/components.hpp>
#include <engine/graphics/camera.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/graphics/core/sampler.hpp>
#include <engine/graphics/core/texture.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/resources.hpp>

// clang-format off
#include <GLFW/glfw3.h>
// clang-format on

#include <vector>

using namespace graphics;
class DynamicState : public GameState {
   public:
    void update(float _time, float _deltaTime);
    void draw(float _time, float _deltaTime);
    void onPause(){};
    void onResume(){};
    bool isFinished() { return false; };
    DynamicState(GLFWwindow* _window);

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