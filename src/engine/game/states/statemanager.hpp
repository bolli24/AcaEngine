#pragma once

#include "gamestate.hpp"
#include <engine/input/inputmanager.hpp>

class StateManager
{
public:
    void addNewState(std::unique_ptr<GameState> new_state);
    void deleteLastState();
    std::vector<std::unique_ptr<GameState>> states;
    GameState& current;
private:
};