#include <game/states/physicsstate.hpp>

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

PhysicsState::PhysicsState(GLFWwindow* _window) : camera(90.0f, 0.1f, 100.0f),
                                                  sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                                                          Sampler::Filter::LINEAR, Sampler::Border::MIRROR),
                                                  cameraPosition(cameraStartPosition),
                                                  window(_window),
                                                  mesh(*utils::MeshLoader::get("/models/crate.obj")),
                                                  texture(Texture2DManager::get("/textures/moon.jpg", sampler)) {
    camera.setView(glm::lookAt(cameraStartPosition, cameratStartLookAt, cameraUp));
    glfwSetKeyCallback(window, GameState::keyCallbackDispatch);
    glfwSetMouseButtonCallback(window, GameState::mouseButtonCallbackDispatch);

    for (auto& light : lights) {
        registry.getComponents<Light>().insert(registry.create(), light);
    }

    LightSystem::updateLights(registry, meshRenderer.getProgram());

    Entity crate = registry.create();
    registry.getComponents<Transform>().insert(crate, {{0.0f, 0.0f, 0.0f}});
    registry.getComponents<MeshRender>().insert(crate, {&mesh, texture});
    registry.getComponents<PhysicsObject>().insert(crate, {10.0f, 10.0f});
    CollisionSystem::addMeshCollider(registry, crate, mesh.meshData);

    Entity crate2 = registry.create();
    registry.getComponents<Transform>().insert(crate2, {{10.2f, 0.0f, 0.25f}, {-0.04f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.5f}});
    registry.getComponents<MeshRender>().insert(crate2, {&mesh, texture});
    registry.getComponents<PhysicsObject>().insert(crate2, {6.0f, 6.0f});
    CollisionSystem::addMeshCollider(registry, crate2, mesh.meshData);
}

void PhysicsState::draw(float time, float deltaTime) {
    RenderSystem::draw(registry, meshRenderer);
    meshRenderer.present(camera, cameraPosition);
}

const float maxDistance = 10.0f;

void PhysicsState::update(float time, float deltaTime) {
    registry.execute<Entity, Transform>([&](Entity& entity, Transform& transform) {
        transform.position += transform.velocity;
        transform.rotation += transform.angularVelocity;

        /* if (glm::distance(transform.position, cameraStartPosition) >= maxDistance) {
            registry.erase(entity);
        }*/
    });

    CollisionSystem::updateMeshCollsions(registry);
}

void PhysicsState::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
}

void PhysicsState::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
}
