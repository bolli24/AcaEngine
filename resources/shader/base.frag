#version 450

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 position;

layout(binding = 0) uniform sampler2D tx_color;
layout(location = 0) uniform vec3 lightPos;
layout(location = 4) uniform vec3 camPos;

layout(location = 0) out vec4 out_color;

void main() {

    //AmbientLight
    vec3 ambient = vec3(0.1f , 0.1f , 0.1f);

    //DiffuseLight
    vec3 norm = normalize(normal);
    vec3 lightDir = -normalize(position - lightPos);
    float diff = clamp(dot(lightDir , norm), 0.f, 1.f);
    vec3 diffuse = diff * vec3(1.f, 1.f , 1.f);

    //SpecularLight
    vec3 lightPosDir = -normalize(lightPos - position);
    vec3 reflectDir = normalize(reflect(lightPosDir , norm));
    vec3 viewDir = normalize(camPos - position);
    float spec = pow(max(dot(viewDir, reflectDir), 0.f), 30.f);
    vec3 specular = vec3(1.f, 1.f , 1.f) * spec;


    out_color = texture(tx_color, uv) * (vec4(ambient, 1.f) + vec4(diffuse, 1.f) + vec4(specular, 1.f));
    // out_color = vec4(uv, 0, 0);
}