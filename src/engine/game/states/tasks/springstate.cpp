#include <engine/game/states/tasks/springstate.hpp>

using namespace graphics;

static const glm::mat4 meshTransform = glm::mat4(1.f);

void SpringState::draw(float time, float deltaTime) {
    meshRenderer.clear();
    meshRenderer.draw(mesh, *texture, meshTransform);
    meshRenderer.present(camera);
}