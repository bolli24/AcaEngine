#include <engine/graphics/renderer/mesh.hpp>
#include <engine/graphics/renderer/meshrenderer.hpp>
#include <engine/utils/meshloader.hpp>
#include <engine/graphics/core/device.hpp>
#include <engine/graphics/core/geometrybuffer.hpp>
#include <engine/input/inputmanager.hpp>
#include <engine/utils/meshloader.hpp>
#include <engine/graphics/core/opengl.hpp>
#include <engine/graphics/resources.hpp>
#include <GLFW/glfw3.h>
#include <iostream>

#include <thread>

// CRT's memory leak detection
#ifndef NDEBUG
#if defined(_MSC_VER)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

using namespace std::chrono_literals;
using namespace graphics;

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
};

int main(int argc, char* argv[]) {
#ifndef NDEBUG
#if defined(_MSC_VER)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
    //	_CrtSetBreakAlloc(2760);
#endif
#endif

    graphics::Device::initialize(1366, 768, false);
    GLFWwindow* window = graphics::Device::getWindow();
    glCall(glEnable, GL_DEPTH_TEST);

    {
        Camera camera(90.0f, 0.1f, 100.0f);
        camera.setView(glm::lookAt(
            glm::vec3(0, 2, 2),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)));

        auto mesh = utils::MeshLoader::get("models/sphere.obj");
        glm::mat4 meshTransform = glm::mat4(1.0f);
        glm::mat4 meshProjection = meshTransform * camera.getViewProjection();

        std::vector<Vertex> vertices;

        const auto& pos = mesh->positions;
        const auto& uvs = mesh->textureCoordinates;
        const auto& normals = mesh->normals;

        for (const auto& face : mesh->faces) {
            const auto& idx = face.indices;

            vertices.push_back({pos[idx[0].positionIdx], uvs[idx[0].textureCoordinateIdx.value()], normals[idx[0].normalIdx.value()]});
            vertices.push_back({pos[idx[1].positionIdx], uvs[idx[1].textureCoordinateIdx.value()], normals[idx[1].normalIdx.value()]});
            vertices.push_back({pos[idx[2].positionIdx], uvs[idx[2].textureCoordinateIdx.value()], normals[idx[2].normalIdx.value()]});
        }

        const std::array attributes = {VertexAttribute{PrimitiveFormat::FLOAT, 3},
                                       VertexAttribute{PrimitiveFormat::FLOAT, 2},
                                       VertexAttribute{PrimitiveFormat::FLOAT, 3}};

        GeometryBuffer geometryBuffer(GLPrimitiveType::TRIANGLES, attributes.data(), attributes.size(), 0);
        geometryBuffer.setData(vertices.data(), vertices.size() * sizeof(Vertex));
        geometryBuffer.bind();

        // sampler
        // needs to be stored somewhere for as long as the texture exists!
        static Sampler sampler(Sampler::Filter::LINEAR, Sampler::Filter::LINEAR,
                               Sampler::Filter::LINEAR, Sampler::Border::MIRROR);
        // loading
        auto texture = Texture2DManager::get("textures/planet1.png", sampler);
        // binding
        texture->bind(0);

        const auto* fragmentShader = ShaderManager::get("shader/demo.frag", ShaderType::FRAGMENT);
        const auto* vertexShader = ShaderManager::get("shader/demo.vert", ShaderType::VERTEX);

        graphics::Program program;
        program.attach(vertexShader);
        program.attach(fragmentShader);
        program.link();
        program.setUniform(1, meshProjection);
        program.use();

        while (!glfwWindowShouldClose(window)) {
            glCall(glClear, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            geometryBuffer.draw();

            glfwPollEvents();
            glfwSwapBuffers(window);
        }

        Texture2DManager::clear();
        ShaderManager::clear();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return EXIT_SUCCESS;
}
