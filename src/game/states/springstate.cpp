#include <game/states/springstate.hpp>

using namespace graphics;

static const glm::vec3 cameraStartPosition = glm::vec3(0.f, -0.f, 5.f);
static const glm::vec3 cameratStartLookAt = glm::vec3(0.f, -0.f, 0.f);
static const glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

static const float speedFactor = 0.25f;

SpringState::SpringState() : camera(90.0f, 0.1f, 100.0f),
                             sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                                     Sampler::Filter::LINEAR, Sampler::Border::MIRROR),
                             cameraPosition(cameraStartPosition),
                             mesh(*utils::MeshLoader::get("/models/sphere.obj")),
                             texture(Texture2DManager::get("/textures/planet1.png", sampler)),
                             meshTransform(glm::mat4(1.f)) {
    camera.setView(glm::lookAt(cameraStartPosition, cameratStartLookAt, cameraUp));
    meshTransform = glm::translate(meshTransform, glm::vec3(-3.0f, 0.0f, 0.0f));
}

void SpringState::draw(float time, float deltaTime) {
    meshRenderer.clear();
    meshRenderer.draw(mesh, *const_cast<Texture2D*>(texture), meshTransform);
    meshRenderer.present(camera, cameraPosition);
}

void SpringState::update(float time, float deltaTime) {
    acceleration = -meshTransform[3][0];
    speed += acceleration * deltaTime * speedFactor;
    meshTransform = glm::translate(meshTransform, speed * glm::vec3(1.0f, 0.0f, 0.0f));
}