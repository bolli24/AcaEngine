#pragma once

#include <engine/graphics/core/sampler.hpp>
#include <engine/input/inputmanager.hpp>
#include <memory>
#include <vector>

#include "gamestate.hpp"

class StateManager {
   public:
    void addNewState(std::unique_ptr<GameState>& new_state);
    void deleteLastState();
    std::vector<std::unique_ptr<GameState>> states;
    std::unique_ptr<GameState> current;
    StateManager(graphics::Sampler* _sampler) {
        sampler = _sampler;
        instance = this;
    };

    static StateManager* instance;
    static graphics::Sampler* sampler;
};