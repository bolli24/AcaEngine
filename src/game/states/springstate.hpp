#pragma once

#include <engine/game/states/gamestate.hpp>
#include <engine/graphics/camera.hpp>
#include <engine/graphics/core/sampler.hpp>
#include <engine/graphics/core/texture.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/resources.hpp>

using namespace graphics;
class SpringState : public GameState {
   public:
    void update(float _time, float _deltaTime);
    void draw(float _time, float _deltaTime);
    void onPause(){};
    void onResume(){};
    bool isFinished() { return finished; };
    SpringState();
    void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods){};

   private:
    Camera camera;
    MeshRenderer meshRenderer;
    glm::vec3 cameraPosition;

    Mesh mesh;
    Texture2D::Handle texture;
    glm::mat4 meshTransform;
    Registry registry;
    float speed = 0;
    float acceleration = 0;
    bool finished = false;
};