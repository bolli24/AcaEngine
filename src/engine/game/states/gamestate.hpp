#pragma once

class GameState {
   public:
    virtual ~GameState(){};
    virtual void update(float _time, float _deltaTime) = 0;
    virtual void draw(float _time, float _deltaTime) = 0;
    virtual void onPause() = 0;
    virtual void onResume() = 0;
    virtual bool isFinished() = 0;
};