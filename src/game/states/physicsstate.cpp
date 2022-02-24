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

    ConvexHull::getConvexHull(utils::MeshLoader::get("/models/crate.obj")->positions);
}

void PhysicsState::draw(float time, float deltaTime) {
    meshRenderer.clear();

    registry.execute<Transform>([&](Transform& transform) {
        glm::mat4 newTransform = glm::translate(glm::mat4(1.0f), transform.position);
        newTransform = glm::rotate(newTransform, transform.rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        newTransform = glm::rotate(newTransform, transform.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        newTransform = glm::rotate(newTransform, transform.rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        newTransform = glm::scale(newTransform, transform.scale);

        meshRenderer.draw(mesh, *const_cast<Texture2D*>(texture), newTransform);
    });

    meshRenderer.present(camera, cameraPosition);
}

const float maxDistance = 10.0f;


void PhysicsState::update(float time, float deltaTime) {
    registry.execute<Entity, Transform>([&](Entity& entity, Transform& transform) {
        transform.position += transform.velocity;
        transform.rotation += transform.angularVelocity;

        if (glm::distance(transform.position, cameraStartPosition) >= maxDistance) {
            registry.erase(entity);
        }
    });

    CollisionSystem::update(registry);
}

void PhysicsState::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
}

void PhysicsState::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
}
