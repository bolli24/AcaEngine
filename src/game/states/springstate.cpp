#include <engine/game/states/statemanager.hpp>
#include <game/states/dynamicstate.hpp>
#include <game/states/physicsstate.hpp>
#include <game/states/springstate.hpp>

using namespace graphics;

static const glm::vec3 cameraStartPosition = glm::vec3(0.f, -0.f, 5.f);
static const glm::vec3 cameratStartLookAt = glm::vec3(0.f, -0.f, 0.f);
static const glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

static const float speedFactor = 0.25f;

static const std::vector<Light> lights = {{glm::vec3(1.2f, -3.f, 2.f), glm::vec3(1.f, 1.f, 1.f)},
                                          {glm::vec3(0.f, 0.f, 2.f), glm::vec3(1.f, 1.f, 1.f)},
                                          {glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(0.f, 1.f, 0.f)},
                                          {glm::vec3(1.5f, -2.f, 2.0f), glm::vec3(0.f, 1.f, 0.f)}};

SpringState::SpringState() : camera(90.0f, 0.1f, 100.0f),
                             cameraPosition(cameraStartPosition),
                             mesh(*utils::MeshLoader::get("/models/sphere.obj")),
                             texture(Texture2DManager::get("/textures/planet1.png", *StateManager::sampler)),
                             meshTransform(glm::mat4(1.f)) {
    camera.setView(glm::lookAt(cameraStartPosition, cameratStartLookAt, cameraUp));
    meshTransform = glm::translate(meshTransform, glm::vec3(-3.0f, 0.0f, 0.0f));

    LightSystem::addLights(registry, lights);
    LightSystem::updateLights(registry, meshRenderer.getProgram());
}

void SpringState::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        std::unique_ptr<GameState> newState = std::make_unique<PhysicsState>();
        finished = true;
        StateManager::instance->addNewState(newState);
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        std::unique_ptr<GameState> newState = std::make_unique<DynamicState>();
        finished = true;
        StateManager::instance->addNewState(newState);
    }
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