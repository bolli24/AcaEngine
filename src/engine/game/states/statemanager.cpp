#include <engine/game/states/statemanager.hpp>

StateManager* StateManager::instance;
graphics::Sampler* StateManager::sampler;

void StateManager::addNewState(std::unique_ptr<GameState>& new_state) {
    // empty vector
    if (states.empty()) {
        states.push_back(std::move(new_state));
        current = std::move(states.back());
    }
    // add new state to existing vector
    else {
        if (current->isFinished()) {
            states.pop_back();
        } else {
            current->onPause();
        }

        states.push_back(std::move(new_state));
        current = std::move(states.back());
    }

    current->setEventHandling();
}
void StateManager::deleteLastState() {
    if (states.size() > 1) {
        states.pop_back();
        current = std::move(states.back());
        current->onResume();
        current->setEventHandling();
    }
    if (states.size() == 1) {
        states.pop_back();
    }
}