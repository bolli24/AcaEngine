#pragma once

#include <GLFW/glfw3.h>

#include <engine/game/registry.hpp>

class GameState {
   public:
    virtual ~GameState(){};
    virtual void update(float _time, float _deltaTime) = 0;
    virtual void draw(float _time, float _deltaTime) = 0;
    virtual void onPause() = 0;
    virtual void onResume() = 0;
    virtual bool isFinished() = 0;

    virtual void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) = 0; /* purely abstract function */
    virtual void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) = 0;

    static GameState *event_handling_instance;
    // technically setEventHandling should be finalized so that it doesn't
    // get overwritten by a descendant class.
    virtual void setEventHandling() final { event_handling_instance = this; }

    static void keyCallbackDispatch(GLFWwindow *window, int key, int scancode, int action, int mods) {
        if (event_handling_instance)
            event_handling_instance->keyCallback(window, key, scancode, action, mods);
    }

    static void mouseButtonCallbackDispatch(GLFWwindow *window, int button, int action, int mods) {
        if (event_handling_instance)
            event_handling_instance->mouseButtonCallback(window, button, action, mods);
    }

   private:
    Registry registry;
};