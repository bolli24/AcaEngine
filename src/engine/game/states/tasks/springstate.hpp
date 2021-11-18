#pragma once

#include <engine/game/states/gamestate.hpp>
#include <engine/graphics/renderer/mesh.hpp>

using namespace graphics;

class SpringState : public GameState {
   public:
    void update(float _time, float _deltaTime){};
    void draw(float _time, float _deltaTime);
    void onPause(){};
    void onResume(){};
    bool isFinished() { return false; };

    SpringState(Mesh _mesh) : mesh(_mesh){};

   private:
    Mesh mesh;
};