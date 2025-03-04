#pragma once

#include <engine/game/game.hpp>
#include <engine/game/states/gamestate.hpp>
#include <engine/graphics/camera.hpp>
#include <engine/graphics/core/sampler.hpp>
#include <engine/graphics/core/texture.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/resources.hpp>

using namespace graphics;

class FreeFallState : public GameState {
   public:
    void update(float _time, float _deltaTime);
    void draw(float _time, float _deltaTime);
    void onPause(){};
    void onResume(){};
    bool isFinished() { return false; };
    FreeFallState();

   private:
    Camera camera;
    Sampler sampler;
    MeshRenderer meshRenderer;
    glm::vec3 cameraPosition;

    Mesh mesh;
    Texture2D::Handle texture;
    glm::mat4 meshTransform;
    float speed = 0;
};