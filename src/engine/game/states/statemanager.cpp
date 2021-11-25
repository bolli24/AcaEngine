#include "statemanager.hpp"


void StateManager::addNewState(std::unique_ptr<GameState> new_state){
    //empty vector
    if(states.empty())
    {
        states.push_back(std::move(new_state));
        current = *states.back();
    }
    //add new state to existing vector
    else
    {
        current.onPause();
        states.push_back(std::move(new_state));
        current = *states.back();
    }
}
void StateManager::deleteLastState()
{
    if(states.size()>1)
    {
        states.pop_back();
        current = *states.back();
        current.onResume();
    }
    if(states.size()==1)
    {
        states.pop_back();
    }
}