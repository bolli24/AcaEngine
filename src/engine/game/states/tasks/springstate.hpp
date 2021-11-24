#pragma once

#include <engine/game/states/gamestate.hpp>
#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/graphics/camera.hpp>
#include <engine/graphics/core/texture.hpp>
#include <engine/graphics/resources.hpp>

using namespace graphics;

class SpringState : public GameState {
   public:
    void update(float _time, float _deltaTime){};
    void draw(float _time, float _deltaTime);
    void onPause(){};
    void onResume(){};
    bool isFinished() { return false; };

    SpringState() : camera(90.0f, 0.1f, 100.0f),
                    mesh(*utils::MeshLoader::get("models/sphere.obj")),
                    texture(Texture2DManager::get("textures/planet1.png", game::Game::sampler)) {
        camera.setView(glm::lookAt( // TODO improve camera initilization
            glm::vec3(0, 0, 2),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)));
    };

   private:
    Camera camera;
    MeshRenderer meshRenderer;

    Mesh mesh;
    Texture2D::Handle texture;
};