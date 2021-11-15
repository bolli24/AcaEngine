#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec2 in_uv;

layout(location = 1) uniform mat4 projection;

layout(location = 0) out vec2 uv;

void main()
{
	gl_Position = projection * vec4(in_position, 1);
	uv = in_uv;
}