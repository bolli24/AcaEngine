#include <engine/game/states/statemanager.hpp>
#include <game/states/dynamicstate.hpp>
#include <game/states/physicsstate.hpp>
#include <game/states/springstate.hpp>

using namespace graphics;

static const glm::vec3 cameraStartPosition = glm::vec3(0.f, 0.f, 5.f);
static const glm::vec3 cameratStartLookAt = glm::vec3(0.f, -0.f, 0.f);
static const glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

static const std::vector<Light> lights = {{glm::vec3(1.2f, -3.f, 2.f), glm::vec3(1.f, 1.f, 1.f)},
                                          {glm::vec3(0.f, 0.f, 2.f), glm::vec3(1.f, 1.f, 1.f)},
                                          {glm::vec3(0.7f, 0.2f, 2.0f), glm::vec3(0.f, 1.f, 0.f)},
                                          {glm::vec3(1.5f, -2.f, 2.0f), glm::vec3(0.f, 1.f, 0.f)},
                                          {glm::vec3(-4.0f, 2.0f, -1.0f), glm::vec3(1.f, 0.f, 0.f)},
                                          {glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(1.f, 0.f, 0.f)},
                                          {glm::vec3(2.f, -2.f, 1.f), glm::vec3(0.f, 0.f, 1.f)},
                                          {glm::vec3(1.f, -3.f, 1.f), glm::vec3(0.f, 0.f, 1.f)}};

PhysicsState::PhysicsState() : camera(90.0f, 0.1f, 100.0f),
                               cameraPosition(cameraStartPosition),
                               mesh(*utils::MeshLoader::get("/models/crate.obj")),
                               texture(Texture2DManager::get("/textures/cratetex.png", *StateManager::sampler)) {
    camera.setView(glm::lookAt(cameraStartPosition, cameratStartLookAt, cameraUp));

    LightSystem::addLights(registry, lights);
    LightSystem::updateLights(registry, meshRenderer.getProgram());

    Entity crate = registry.create();
    registry.getComponents<Transform>().insert(crate, {{0.0f, 0.0f, 0.0f}});
    registry.getComponents<MeshRender>().insert(crate, {&mesh, texture});
    float mass1 = 10.0f;
    registry.getComponents<PhysicsObject>().insert(crate, {mass1, CollisionSystem::getCuboidInertiaTensor(2, 4, 2, mass1)});
    CollisionSystem::addMeshCollider(registry, crate, mesh.meshData);

    Entity crate2 = registry.create();
    registry.getComponents<Transform>().insert(crate2, {{10.2f, -3.5f, 0.25f}, {-0.04f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}});
    registry.getComponents<MeshRender>().insert(crate2, {&mesh, texture});
    float mass2 = 10.0f;
    registry.getComponents<PhysicsObject>().insert(crate2, {mass2, CollisionSystem::getCuboidInertiaTensor(2, 4, 2, mass2)});
    CollisionSystem::addMeshCollider(registry, crate2, mesh.meshData);
}

void PhysicsState::draw(float time, float deltaTime) {
    RenderSystem::draw(registry, meshRenderer, camera, cameraPosition);
}

void PhysicsState::update(float time, float deltaTime) {
   
    TransformSystem::updateTransforms(registry);
    CollisionSystem::updateMeshCollsions(registry);
}

void PhysicsState::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        std::unique_ptr<GameState> newState = std::make_unique<SpringState>();
        finished = true;
        StateManager::instance->addNewState(newState);
    }
    if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
        std::unique_ptr<GameState> newState = std::make_unique<DynamicState>();
        finished = true;
        StateManager::instance->addNewState(newState);
    }
}
