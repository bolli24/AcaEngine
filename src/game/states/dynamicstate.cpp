#include <game/states/dynamicstate.hpp>

using namespace graphics;

static const glm::vec3 cameraStartPosition = glm::vec3(0.f, -0.f, 5.f);
static const glm::vec3 cameratStartLookAt = glm::vec3(0.f, -0.f, 0.f);
static const glm::vec3 cameraUp = glm::vec3(0.f, 1.f, 0.f);

static const float speedFactor = 0.25f;

DynamicState::DynamicState(GLFWwindow* _window) : camera(90.0f, 0.1f, 100.0f),
                                                  sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                                                          Sampler::Filter::LINEAR, Sampler::Border::MIRROR),
                                                  cameraPosition(cameraStartPosition),
                                                  window(_window),
                                                  mesh(*utils::MeshLoader::get("/models/sphere.obj")),
                                                  texture(Texture2DManager::get("/textures/planet1.png", sampler)) {
    camera.setView(glm::lookAt(cameraStartPosition, cameratStartLookAt, cameraUp));
}

void DynamicState::draw(float time, float deltaTime) {
    meshRenderer.clear();

    registry.execute<Position>([&](Position position) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position.position);
        meshRenderer.draw(mesh, *const_cast<Texture2D*>(texture), transform);
    });

    meshRenderer.present(camera, cameraPosition);
}

void DynamicState::update(float time, float deltaTime) {
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        Entity newEntity = registry.create();
        registry.getComponents<Position>().insert(newEntity, {{1.0f, 2.0f, 3.0f}});
    }
}

float rFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}