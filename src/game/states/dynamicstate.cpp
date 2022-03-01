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

DynamicState::DynamicState() : camera(90.0f, 0.1f, 100.0f),
                               cameraPosition(cameraStartPosition),
                               mesh(*utils::MeshLoader::get("/models/sphere.obj")),
                               texture(Texture2DManager::get("/textures/moon.jpg", *StateManager::sampler)) {
    camera.setView(glm::lookAt(cameraStartPosition, cameratStartLookAt, cameraUp));

    LightSystem::addLights(registry, lights);
    LightSystem::updateLights(registry, meshRenderer.getProgram());
}

void DynamicState::draw(float time, float deltaTime) {
    RenderSystem::draw(registry, meshRenderer, camera, cameraPosition);
}

const float maxDistance = 10.0f;

float interval = 0;

void DynamicState::update(float time, float deltaTime) {
    TransformSystem::updateTransforms(registry);

    registry.execute<Entity, Transform>([&](Entity& entity, Transform& transform) {
        if (glm::distance(transform.position, cameraStartPosition) >= maxDistance) {
            registry.erase(entity);
        }
    });

    if (interval <= 0) {
        interval = 0.320f;
        createSphere();
    }

    interval -= deltaTime;

    CollisionSystem::updateAABBCollisions(registry);
}

void DynamicState::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        std::unique_ptr<GameState> newState = std::make_unique<SpringState>();
        finished = true;
        StateManager::instance->addNewState(newState);
    }
    if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
        std::unique_ptr<GameState> newState = std::make_unique<PhysicsState>();
        finished = true;
        StateManager::instance->addNewState(newState);
    }
}

void DynamicState::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        shootProjectile(window);
}

void DynamicState::createSphere() {
    Entity newEntity = registry.create();

    glm::vec3 initialPos = {rFloat(-3.0f, 3.0f), rFloat(-2.0f, 2.0f), rFloat(-1.0f, 1.0f)};

    registry.getComponents<Transform>().insert(newEntity, {initialPos,
                                                           {rFloat(-0.01f, 0.01f), rFloat(-0.01f, 0.01f), rFloat(-0.01f, 0.01f)},
                                                           {rFloat(-1.00f, 1.0f), rFloat(-1.0f, 1.0f), rFloat(-1.0f, 1.0f)},
                                                           {rFloat(-0.02f, 0.02f), rFloat(-0.02f, 0.02f), rFloat(-0.02f, 0.02f)}});

    registry.getComponents<AABBCollider>().insert(newEntity,
                                                  {ColliderType::Target, math::Box(math::HyperSphere<3, float>(initialPos, 1.0f))});
    registry.getComponents<MeshRender>().insert(newEntity, {&mesh, texture});
}

void DynamicState::shootProjectile(GLFWwindow* window) {
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
    registry.getComponents<MeshRender>().insert(newEntity, {&mesh, texture});
}

float DynamicState::rFloat(float a, float b) {
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = b - a;
    float r = random * diff;
    return a + r;
}