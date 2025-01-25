#version 450
#extension GL_EXT_debug_printf : enable

layout(location = 0) in vec3 positions;
layout(location = 1) in vec3 normals;
layout(location = 2) in vec3 colors;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 positionOut;
layout(location = 1) out vec3 normalOut;
layout(location = 2) out vec3 colorOut;
layout(location = 3) out vec2 uvOut;

layout(push_constant) uniform PushConstants
{
    mat4 model;
} pushConstants;

layout(set = 0, binding = 0) uniform CameraUbo
{
    mat4 view;
    mat4 projection;
} cameraData;

layout(set = 1, binding = 0) uniform MaterialInfo
{
    vec3 color;
    vec3 ior;
    float roughness;
    float metallic;
} materialInfo;

void main()
{
    gl_Position = cameraData.projection * inverse(cameraData.view) * pushConstants.model * vec4(positions, 1.0);
    positionOut = positions;

    normalOut = normalize(mat3(pushConstants.model) * normals);
    colorOut = colors;
    uvOut = uv;

}