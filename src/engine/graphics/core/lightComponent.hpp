#pragma once

#include <engine/game/registry.hpp>

#include "../core/shader.hpp"
#include "glm/glm.hpp"

namespace graphics {

class Light {
   public:
    Light();
    void createLights(Program& program);
};
};  // namespace graphics