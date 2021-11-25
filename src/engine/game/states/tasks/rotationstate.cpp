#include <engine/game/states/tasks/rotationstate.hpp>

using namespace graphics;

static const glm::vec3 cameraStartPosition = glm::vec3(0.f, 0.f, 2.f);
static const glm::vec3 cameratStartLookAt = glm::vec3(0.f, 0.f, 0.f);
static const glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);
static const float rotationSpeed = 0.01f;

RotationState::RotationState() : camera(90.0f, 0.1f, 100.0f),
                                 sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                                         Sampler::Filter::LINEAR, Sampler::Border::MIRROR),
                                 mesh(*utils::MeshLoader::get("/models/sphere.obj")),
                                 texture(Texture2DManager::get("/textures/planet1.png", sampler)),
                                 meshTransform(glm::mat4(1.f)) {
    camera.setView(glm::lookAt(cameraStartPosition, cameratStartLookAt, cameraUp));
    meshRenderer.draw(mesh, *texture, meshTransform);
}

void RotationState::draw(float time, float deltaTime) {
    meshTransform = glm::rotate(meshTransform, glm::radians(rotationSpeed) / deltaTime, glm::vec3(0.0f, 1.0f, 0.0f));
    meshRenderer.present(camera);
}