#pragma once

#include <engine/graphics/core/sampler.hpp>

using namespace graphics;

namespace game {
class Game {
   public:
    Game(){};
    void run();

    static const Sampler sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                                 Sampler::Filter::LINEAR, Sampler::Border::MIRROR);
};
}  // namespace game