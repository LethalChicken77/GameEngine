#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) out vec4 fragColor;
layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec3 color;
layout(location = 3) in vec2 uv;

layout(push_constant) uniform PushConstants {
    mat4 model;
} pushConstants;

layout(set = 0, binding = 0) uniform CameraUbo {
    mat4 view;
    mat4 projection;
} cameraData;

layout(set = 1, binding = 0) uniform MaterialInfo
{
    vec3 color;
} materialInfo;

const float PI = 3.14159265359;
const float INV_PI = 1.0 / PI;
const float EPSILON = 0.0001;

void main()
{
    vec3 outColor = color * materialInfo.color;
    fragColor = vec4(outColor, 1.0);
}