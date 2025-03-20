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

layout(set = 1, binding = 4) uniform sampler2D heightMap;

void main()
{
    float heightScale = 1.0;
    vec3 heightMapPos = vec3(positions.x, texture(heightMap, uv).r * heightScale, positions.z);
    gl_Position = cameraData.projection * inverse(cameraData.view) * pushConstants.model * vec4(heightMapPos, 1.0);
    positionOut = heightMapPos;

    // normalOut = normalize(mat3(pushConstants.model) * normals);
    float texelSize = 1.0 / 512.0 / 2.0;
    float gridSize = 50.0;
    // Sample height values from adjacent texels
    float heightCenter = texture(heightMap, uv).r * heightScale;
    float heightR = texture(heightMap, uv + vec2(texelSize, 0.0)).r * heightScale;
    float heightU = texture(heightMap, uv + vec2(0.0, texelSize)).r * heightScale;

    float dx = (heightR - heightCenter);
    float dy = (heightU - heightCenter);

    float worldStep = gridSize * texelSize;

    float slopeX = dx / worldStep;
    float slopeZ = dy / worldStep;

    // Compute tangent and bitangent vectors
    vec3 tangent   = normalize(vec3(1.0, slopeX, 0.0));
    vec3 bitangent = normalize(vec3(0.0, slopeZ, 1.0));

    // Compute normal via cross product and normalize
    vec3 normal = normalize(cross(bitangent, tangent));

    mat3 normalMatrix = transpose(inverse(mat3(pushConstants.model)));
    normalOut = normalize(normalMatrix * normal);
    colorOut = colors;
    uvOut = uv;

}