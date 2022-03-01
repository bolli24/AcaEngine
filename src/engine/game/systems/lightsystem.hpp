#pragma once

#include "../../graphics/core/shader.hpp"
#include "../components.hpp"
#include "../registry.hpp"
#include "glm/glm.hpp"

class LightSystem {
   public:
    static void updateLights(Registry& registry, graphics::Program& program) {
        std::vector<glm::vec3> lightPos;
        std::vector<glm::vec3> lightCol;

        registry.execute<Light>([&](Light& light) {
            lightPos.push_back(light.position);
            lightCol.push_back(light.color);
        });

        program.setUniform(1, (int)lightPos.size(), lightPos.data());
        program.setUniform(2, (int)lightCol.size(), lightCol.data());
    };

    static void addLights(Registry& registry, const std::vector<Light>& lights) {
        for (auto& light : lights) {
            registry.getComponents<Light>().insert(registry.create(), light);
        }
    }
};