#pragma once

#include <engine/game/systems/collisionsystem.hpp>
#include <engine/game/components.hpp>
#include <engine/game/registry.hpp>
#include <engine/game/systems/rendersystem.hpp>
#include <engine/game/states/gamestate.hpp>
#include <engine/game/systems/transformsystem.hpp>
#include <engine/graphics/camera.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/game/systems/lightsystem.hpp>
#include <engine/graphics/core/sampler.hpp>
#include <engine/graphics/core/texture.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/resources.hpp>
#include <engine/math/convexhull.hpp>
#include <vector>

using namespace graphics;
class PhysicsState : public GameState {
   public:
    void update(float _time, float _deltaTime);
    void draw(float _time, float _deltaTime);
    void onPause(){};
    void onResume(){};
    bool isFinished() { return finished; };
    PhysicsState();
    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods){};

   private:
    Camera camera;
    MeshRenderer meshRenderer;
    glm::vec3 cameraPosition;

    Mesh mesh;
    Texture2D::Handle texture;
    Registry registry;

    bool finished = false;
};