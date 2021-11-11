#version 450

layout(location = 0) in vec2 uv;
layout(binding = 0) uniform sampler2D tx_color;
layout(location = 0) out vec4 out_color;

void main() {

	out_color = texture(tx_color, uv);
	// out_color = vec4(uv, 0, 0);
}