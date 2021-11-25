#pragma once

#include "gamestate.hpp"
#include <engine/input/inputmanager.hpp>
#include <vector>
#include <memory>

class StateManager {
   public:
    void addNewState(std::unique_ptr<GameState>& new_state);
    void deleteLastState();
    std::vector<std::unique_ptr<GameState>> states;
    std::unique_ptr<GameState> current;
    StateManager(){};

   private:
};