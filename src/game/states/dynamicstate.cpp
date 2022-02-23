#include <game/states/dynamicstate.hpp>

using namespace graphics;

static const glm::vec3 cameraStartPosition = glm::vec3(0.f, -0.f, 5.f);
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

DynamicState::DynamicState(GLFWwindow* _window) : camera(90.0f, 0.1f, 100.0f),
                                                  sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                                                          Sampler::Filter::LINEAR, Sampler::Border::MIRROR),
                                                  cameraPosition(cameraStartPosition),
                                                  window(_window),
                                                  mesh(*utils::MeshLoader::get("/models/sphere.obj")),
                                                  texture(Texture2DManager::get("/textures/moon.jpg", sampler)) {
    camera.setView(glm::lookAt(cameraStartPosition, cameratStartLookAt, cameraUp));
    glfwSetKeyCallback(window, GameState::keyCallbackDispatch);

    for (auto& light : lights) {
        registry.getComponents<Light>().insert(registry.create(), light);
    }

    LightSystem::updateLights(registry, meshRenderer.getProgram());
}

void DynamicState::draw(float time, float deltaTime) {
    meshRenderer.clear();

    registry.execute<Position>([&](Position position) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position.value);
        meshRenderer.draw(mesh, *const_cast<Texture2D*>(texture), transform);
    });

    meshRenderer.present(camera, cameraPosition);
}

void DynamicState::update(float time, float deltaTime) {
    registry.execute<Position, Velocity>([&](Position& position, Velocity& velocity) {
        glm::vec3 position_temp = position.value + velocity.value;
        position.value = position_temp;
    });
}

void DynamicState::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
        createSphere();
}

void DynamicState::createSphere() {
    Entity newEntity = registry.create();
    registry.getComponents<Position>().insert(newEntity, {{0.0f, 0.0f, 0.0f}});
    registry.getComponents<Velocity>().insert(newEntity, {{rFloat(-0.01f, 0.01f), rFloat(-0.01f, 0.01f), rFloat(-0.01f, 0.01f)}});
}

float DynamicState::rFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}