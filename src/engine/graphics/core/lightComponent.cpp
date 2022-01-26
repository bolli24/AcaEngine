#include "lightComponent.hpp"

#include <engine/game/registry.hpp>

#include "../core/shader.hpp"
#include "glm/glm.hpp"

namespace graphics {

Light::Light() {}

void Light::createLights(glm::vec3& meshPosition, Program& program) {
    glm::vec3 lightPos[8] = {glm::vec3(1.2f, -3.f, 2.f),
                             glm::vec3(0.f, 0.f, 2.f),
                             glm::vec3(0.7f, 0.2f, 2.0f),
                             glm::vec3(1.5f, -2.f, 2.0f),
                             glm::vec3(-4.0f, 2.0f, -1.0f),
                             glm::vec3(0.0f, 0.0f, 3.0f),
                             glm::vec3(2.f, -2.f, 1.f),
                             glm::vec3(1.f, -3.f, 1.f)};

    glm::vec3 lightCol[8] = {glm::vec3(1.f, 1.f, 1.f),
                             glm::vec3(1.f, 1.f, 1.f),
                             glm::vec3(0.f, 1.f, 0.f),
                             glm::vec3(0.f, 1.f, 0.f),
                             glm::vec3(1.f, 0.f, 0.f),
                             glm::vec3(1.f, 0.f, 0.f),
                             glm::vec3(0.f, 0.f, 1.f),
                             glm::vec3(0.f, 0.f, 1.f)};

    for (int i = 0; i < 8; i++) {
        /* glm::vec3 diffVec = glm::abs(lightPos[i] - meshPosition);
        float diff = sqrt(diffVec.x * diffVec.x +
                          diffVec.y * diffVec.y +
                          diffVec.z * diffVec.z);
        glm::vec3 diffColor = lightCol[i];
        diffColor = {diffColor.x / diff, diffColor.y / diff, diffColor.z / diff};*/
        lightCol[i] = 0.5f * lightCol[i];
    }
    program.setUniform(1, 8, lightPos);
    program.setUniform(2, 8, lightCol);
}
}  // namespace graphics