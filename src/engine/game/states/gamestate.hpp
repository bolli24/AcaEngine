#pragma once

#include <GLFW/glfw3.h>

class GameState {
   public:
    virtual ~GameState(){};
    virtual void update(float _time, float _deltaTime) = 0;
    virtual void draw(float _time, float _deltaTime) = 0;
    virtual void onPause() = 0;
    virtual void onResume() = 0;
    virtual bool isFinished() = 0;

    virtual void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) = 0; /* purely abstract function */

    static GameState *event_handling_instance;
    // technically setEventHandling should be finalized so that it doesn't
    // get overwritten by a descendant class.
    virtual void setEventHandling() { event_handling_instance = this; }

    static void keyCallbackDispatch(GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (event_handling_instance)
            event_handling_instance->keyCallback(window, key, scancode, action, mods);
    }
};