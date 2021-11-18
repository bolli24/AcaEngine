#pragma once

#include <engine/game/states/gamestate.hpp>

class PhysicalEventState : public GameState {
   public:
    void update(float _time, float _deltaTime);
    void draw(float _time, float _deltaTime);
    void onPause();
    void onResume();
    bool isFinished();
};