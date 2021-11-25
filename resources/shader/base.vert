#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec3 in_normal;

layout(location = 1) uniform mat4 projection;
layout(location = 2) uniform mat4 view;
layout(location = 3) uniform mat4 model;

layout(location = 0) out vec2 uv;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 position;

void main()
{
    uv = in_uv;
    normal = mat3(model) * in_normal;
    position = vec4(model * vec4(in_position, 1.f)).xyz;

    gl_Position = projection * view * model * vec4(in_position, 1.f);
}