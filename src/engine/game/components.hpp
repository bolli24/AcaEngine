#pragma once

#include <glm/glm.hpp>

struct Position {
    glm::vec3 value;
};

struct Light {
    glm::vec3 position;
    glm::vec3 color;
};