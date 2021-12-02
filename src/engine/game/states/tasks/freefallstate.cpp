#include <engine/game/states/tasks/freefallstate.hpp>

using namespace graphics;

static const glm::vec3 cameraStartPosition = glm::vec3(0.f, -3.f, 10.f);
static const glm::vec3 cameratStartLookAt = glm::vec3(0.f, -3.f, 0.f);
static const glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

static const float gravity = .02f;

FreeFallState::FreeFallState() : camera(90.0f, 0.1f, 100.0f),
                                 sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                                         Sampler::Filter::LINEAR, Sampler::Border::MIRROR),
                                 cameraPosition(cameraStartPosition),
                                 mesh(*utils::MeshLoader::get("/models/sphere.obj")),
                                 texture(Texture2DManager::get("/textures/planet1.png", sampler)),
                                 meshTransform(glm::mat4(1.f)) {
    camera.setView(glm::lookAt(cameraStartPosition, cameratStartLookAt, cameraUp));
    meshRenderer.draw(mesh, *texture, meshTransform);
}

void FreeFallState::draw(float time, float deltaTime) {
    //camera.setView(glm::lookAt(cameraStartPosition, glm::vec3(meshTransform[3]), cameraUp));
    meshRenderer.present(camera, cameraPosition);
}

void FreeFallState::update(float time, float deltaTime) {
    speed -= gravity * deltaTime;
    meshTransform = glm::translate(meshTransform, speed * glm::vec3(0.0f, 1.0f, 0.0f));
}