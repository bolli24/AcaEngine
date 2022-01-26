#version 450

#define NR_POINT_LIGHTS 8

layout(location = 0) in vec2 uv;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 position;

layout(binding = 0) uniform sampler2D tx_color;
layout(location = 4) uniform vec3 camPos;

uniform vec3 pointLightsPos[NR_POINT_LIGHTS];
uniform vec3 pointLightsCol[NR_POINT_LIGHTS];

layout(location = 0) out vec4 out_color;

vec3 CalcLight( vec3 lightPos, vec3 normal, vec3 position, 
                vec3 camPos, vec3 lightDiff, vec3 lightSpec ){

    //DiffuseLight
    vec3 norm = normalize(normal);
    vec3 lightDir = -normalize(position - lightPos);
    float diff = clamp(dot(lightDir , norm), 0.f, 1.f);
    vec3 diffuse = diff * lightDiff;

    //SpecularLight
    vec3 lightPosDir = -normalize(lightPos - position);
    vec3 reflectDir = normalize(reflect(lightPosDir , norm));
    vec3 viewDir = normalize(camPos - position);
    float spec = pow(max(dot(viewDir, reflectDir), 0.f), 30.f);
    vec3 specular = lightSpec * spec;

    return diffuse + specular;
}

void main() {

    vec3 result;

    for(int i = 0; i < NR_POINT_LIGHTS; i++){
        result += CalcLight(pointLightsPos[i], normal, position, camPos, 
                            pointLightsCol[i], pointLightsCol[i]);
    }

    result += vec3(0.1f , 0.1f , 0.1f); //AmbientLight

    out_color = texture(tx_color, uv) * vec4(result, 1.f);
    // out_color = vec4(uv, 0, 0);
}