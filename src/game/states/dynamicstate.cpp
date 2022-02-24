#include <game/states/dynamicstate.hpp>

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

DynamicState::DynamicState(GLFWwindow* _window) : camera(90.0f, 0.1f, 100.0f),
                                                  sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                                                          Sampler::Filter::LINEAR, Sampler::Border::MIRROR),
                                                  cameraPosition(cameraStartPosition),
                                                  window(_window),
                                                  mesh(*utils::MeshLoader::get("/models/sphere.obj")),
                                                  texture(Texture2DManager::get("/textures/moon.jpg", sampler)) {
    camera.setView(glm::lookAt(cameraStartPosition, cameratStartLookAt, cameraUp));
    glfwSetKeyCallback(window, GameState::keyCallbackDispatch);
    glfwSetMouseButtonCallback(window, GameState::mouseButtonCallbackDispatch);

    for (auto& light : lights) {
        registry.getComponents<Light>().insert(registry.create(), light);
    }

    LightSystem::updateLights(registry, meshRenderer.getProgram());
}

void DynamicState::draw(float time, float deltaTime) {
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

float interval = 0;

void DynamicState::update(float time, float deltaTime) {
    registry.execute<Entity, Transform>([&](Entity& entity, Transform& transform) {
        transform.position += transform.velocity;
        transform.rotation += transform.angularVelocity;

        if (glm::distance(transform.position, cameraStartPosition) >= maxDistance) {
            registry.erase(entity);
        }
    });

    if (interval <= 0) {
        interval = 0.320;
        createSphere();
    }

    interval -= deltaTime;

    CollisionSystem::update(registry);
}

void DynamicState::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_E && action == GLFW_PRESS)
        createSphere();
}

void DynamicState::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        shootProjectile();
}

void DynamicState::createSphere() {
    Entity newEntity = registry.create();

    glm::vec3 initialPos = {0.0f, 0.0f, 0.0f};

    registry.getComponents<Transform>().insert(newEntity, {initialPos,
                                                           {rFloat(-0.01f, 0.01f), rFloat(-0.01f, 0.01f), rFloat(-0.01f, 0.01f)},
                                                           {rFloat(-1.00f, 1.0f), rFloat(-1.0f, 1.0f), rFloat(-1.0f, 1.0f)},
                                                           {rFloat(-0.02f, 0.02f), rFloat(-0.02f, 0.02f), rFloat(-0.02f, 0.02f)}});

    registry.getComponents<AABBCollider>().insert(newEntity,
                                                  {ColliderType::Target, math::Box(math::HyperSphere<3, float>(initialPos, 1.0f))});
}

void DynamicState::shootProjectile() {
    double xPos, yPos;
    glfwGetCursorPos(window, &xPos, &yPos);
    glm::vec3 mousePostion = camera.toWorldSpace({xPos, yPos});

    // World Space: positive x -> rechts, positive y -> oben, positive z -> richtung camera

    glm::vec3 direction = cameraStartPosition - mousePostion;
    Entity newEntity = registry.create();

    float scale = 0.1f;

    glm::vec3 initialPos = cameraStartPosition - 2.0f * direction;
    Transform transform = {initialPos, {-0.5f * direction}};
    transform.scale *= scale;

    registry.getComponents<Transform>().insert(newEntity, transform);
    registry.getComponents<AABBCollider>().insert(newEntity, {ColliderType::Projectile, math::Box(math::HyperSphere<3, float>(initialPos, scale))});
}

float DynamicState::rFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}