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
    float roughness;
} materialInfo;

const float PI = 3.14159265359;
const float INV_PI = 1.0 / PI;

vec3 cookTorrance(vec3 normal, vec3 lightDir, vec3 viewDir, float roughness, vec3 ior) 
{
    vec3 halfDir = normalize(lightDir + viewDir);
    // float D = (1.0 / (PI * roughness * roughness)) * pow(dot(halfDir, normal), 2.0 / (roughness * roughness) - 2.0); // Beckmann
    float alpha = roughness * roughness;
    float D = alpha * alpha / (PI * pow(dot(normal, halfDir) * dot(normal, halfDir) * (alpha * alpha - 1.0) + 1.0, 2.0)); // GGX
    float G = min(1.0, min(2.0 * dot(normal, halfDir) * dot(normal, viewDir) / dot(viewDir, halfDir), 2.0 * dot(normal, halfDir) * dot(normal, lightDir) / dot(viewDir, halfDir)));
    vec3 F0 = (ior - 1.0) * (ior - 1.0) / ((ior + 1.0) * (ior + 1.0));
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - dot(halfDir, viewDir), 5.0);

    vec3 specular = D * G * F / (4.0 * dot(normal, lightDir) * dot(normal, viewDir));
    specular *= dot(normal, lightDir);
    return max(specular, 0.0);
}

void main()
{
    debugPrintfEXT("Hello from vertex shader\n");
    vec3 normal = normalize(fragNormal);
    vec3 position = (pushConstants.model * vec4(fragPosition, 1.0)).xyz;
    vec3 lightDir = normalize(vec3(2.0, 3.0, 1.0));

    float diffuse = max(dot(normal, lightDir), 0.0);
    float ambient = 0.1;
    // vec3 outColor = (diffuse + ambient) * color * vec3(1.0, 0.8, 0.2);
    vec3 outColor = (diffuse + ambient) * color * materialInfo.color;// * vec3(0.4, 0.2, 0.1);
    // outColor = vec3(0);

    vec3 viewDir = normalize(cameraData.view[3].xyz - position);
    // float spec = pow(max(dot(reflDir, viewDir), 0.0), 32);
    // vec3 ior = vec3(0.27105, 0.67693, 1.31640); // Copper
    // vec3 ior = vec3(0.18299, 0.42108, 1.37340); // Gold
    vec3 ior = vec3(0.04, 0.04, 0.04);
    vec3 spec = cookTorrance(normal, lightDir, viewDir, materialInfo.roughness, ior);

    outColor += spec;

    // fragColor = vec4(1.0, 0.8, 0.2, 1.0);
    fragColor = vec4(outColor, 1.0);
}