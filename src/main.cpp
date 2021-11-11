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

struct Face {
    std::array<int, 3> indices;
};

struct Vertex {

    glm::vec3 position;
    glm::vec2 uv;
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

    {
        Camera camera(90.0f, 0.1f, 100.0f);
        camera.setView(glm::lookAt(
            glm::vec3(0, 0, 2),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)));

        auto mesh = utils::MeshLoader::get("models/sphere.obj");
        glm::mat4 meshTransform = glm::mat4(1.0f);
        glm::mat4 meshProjection = meshTransform * camera.getViewProjection();

        std::vector<Face> faces;

        for (auto face : mesh->faces) {
            auto indices = face.indices;
            faces.push_back({indices[0].positionIdx, indices[1].positionIdx, indices[2].positionIdx});
        }

        std::vector<Vertex> vertices;

        for (size_t i = 0; i < mesh->positions.size(); i++) {
            vertices.push_back({mesh->positions[i], mesh->textureCoordinates[i]});
        }

        const VertexAttribute attributes[] = {{PrimitiveFormat::FLOAT, 3}, {PrimitiveFormat::FLOAT, 2}};
        GeometryBuffer geometryBuffer(GLPrimitiveType::TRIANGLES, attributes, 2, 1);
        // geometryBuffer.setData(mesh->positions.data(), mesh->positions.size() * sizeof(glm::vec3));
        geometryBuffer.setData(vertices.data(), vertices.size() * sizeof(Vertex));
        geometryBuffer.setIndexData(faces.data(), faces.size() * sizeof(Face));
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
