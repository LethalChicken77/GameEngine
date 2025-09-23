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
    vec3 ior;
    float roughness;
    float metallic;
} materialInfo;

layout(set = 1, binding = 1) uniform sampler2D grassAlbedo;
layout(set = 1, binding = 2) uniform sampler2D grassRoughness;
layout(set = 1, binding = 3) uniform sampler2D grassNormalMap;
layout(set = 1, binding = 4) uniform sampler2D rockAlbedo;
layout(set = 1, binding = 5) uniform sampler2D rockRoughness;
layout(set = 1, binding = 6) uniform sampler2D rockNormalMap;
// layout(set = 1, binding = 3) uniform sampler2D heightMap;

const float PI = 3.14159265359;
const float INV_PI = 1.0 / PI;
const float EPSILON = 0.0001;

vec3 orenNayar(vec3 normal, vec3 lightDir, vec3 viewDir, float roughness, vec3 albedo) 
{
    // Clamp roughness to avoid invalid results
    roughness = clamp(roughness, 0.0, 1.0);

    // Compute angles and vectors
    float NdotL = max(dot(normal, lightDir), EPSILON);
    float NdotV = max(dot(normal, viewDir), EPSILON);
    float LdotV = max(dot(lightDir, viewDir), EPSILON);

    // if (NdotL <= 0.0 || NdotV <= 0.0) {
    //     return vec3(0.0);
    // }

    // Roughness squared
    float sigma2 = roughness * roughness;

    // Compute A and B terms based on roughness
    float A = 1.0 - (sigma2 / (2.0 * (sigma2 + 0.33)));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);

    // Compute angles for Oren-Nayar
    float thetaR = acos(NdotV);
    float thetaI = acos(NdotL);

    float alpha = max(thetaR, thetaI);
    float beta = min(thetaR, thetaI);

    // Final diffuse reflection calculation
    float diffuse = NdotL * (A + B * max(0.0, LdotV) * sin(alpha) * tan(beta));

    // Return the final diffuse term scaled by albedo
    return diffuse * albedo;
}

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

float smoothstep2(float a, float b, float t, float center, float width)
{
    float x = (t - center) / width + 0.5;
    x = clamp(x, 0.0, 1.0);
    return mix(a, b, x * x * (3.0 - 2.0 * x));
}

void main()
{
    debugPrintfEXT("Hello from vertex shader\n");
    vec3 _normal = normalize(fragNormal);

    // Compute screen-space derivatives of position and UV
    vec3 dPosdx = dFdx(fragPosition);
    vec3 dPosdy = dFdy(fragPosition);
    vec2 dUVdx = dFdx(uv);
    vec2 dUVdy = dFdy(uv);


    // Compute the normal map and roughness map
    vec3 texColor = vec3(0.0);
    float roughness = 0.0;
    vec3 normalMapSample = vec3(0.5, 0.5, 1.0);

    float blend = smoothstep2(1, 0, dot(_normal, vec3(0,1,0)), 0.65, 0.2);

    texColor = texture(grassAlbedo, uv).rgb;
    roughness = texture(grassRoughness, uv).r;
    normalMapSample = texture(grassNormalMap, uv).xyz;

    texColor = mix(texColor, texture(rockAlbedo, uv).rgb, blend);
    roughness = mix(roughness, texture(rockRoughness, uv).r, blend);
    normalMapSample = mix(normalMapSample, texture(rockNormalMap, uv).xyz, blend);


    normalMapSample = pow(normalMapSample, vec3(0.45454545454)) * 2.0 - 1.0;
    normalMapSample = normalize(normalMapSample);
    roughness = pow(roughness, 0.45454545454) * 2.0 - 1.0;

    // Compute tangent and bitangent
    float det = dUVdx.x * dUVdy.y - dUVdx.y * dUVdy.x;
    float invDet = 1.0 / det;

    vec3 tangent = normalize((dPosdx * dUVdy.y - dPosdy * dUVdx.y) * invDet);
    vec3 bitangent = -normalize((dPosdy * dUVdx.x - dPosdx * dUVdy.x) * invDet);

    mat3 TBN = mat3(tangent, bitangent, _normal);
    vec3 normal = normalize(TBN * normalMapSample);
    float normalStrength = 0.3;
    // normal = normalize(_normal + normal * normalStrength);
    normal = _normal;

    // vec3 normal = _normal;

    vec3 position = (pushConstants.model * vec4(fragPosition, 1.0)).xyz;
    vec3 camPos = cameraData.view[3].xyz;
    vec3 lightPos = vec3(3.0, 7.0, -1.0);
    // lightPos = camPos;
    vec3 lightDir = normalize(vec3(2.0, 2.0, -0.5));
    // vec3 lightDir = normalize(vec3(1.0, 0.0, 1.0));
    // vec3 lightDir = normalize(lightPos - position);
    // float lightDist = length(lightPos - position);
    // float lightAttenuation = 1.0 / (lightDist * lightDist);
    float lightAttenuation = 1.0;
    // float lightStrength = 60;
    float lightStrength = 5;
    vec3 ambientColor = vec3(0.04f, 0.08f, 0.2f);

    vec3 viewDir = normalize(camPos - position);
    // float spec = pow(max(dot(reflDir, viewDir), 0.0), 32);
    // vec3 ior = vec3(0.27105, 0.67693, 1.31640); // Copper
    // vec3 ior = vec3(0.18299, 0.42108, 1.37340); // Gold
    // vec3 ior = vec3(1.5, 1.5, 1.5);
    vec3 ior = materialInfo.ior;

    // float cameraDist = length(cameraData.view[3].xyz - position);

    vec3 halfDir = normalize(lightDir + viewDir);
    vec3 F0 = (ior - 1.0) * (ior - 1.0) / ((ior + 1.0) * (ior + 1.0));
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - dot(halfDir, viewDir), 5.0);


    // roughness = roughness * roughness * roughness;
    // Energy conservation term: (1.0 - F) * 
    vec3 diffuse = (1.0 - F) * orenNayar(normal, lightDir, viewDir, roughness, color * materialInfo.color) * lightAttenuation * lightStrength * texColor;
    vec3 ambient = color * materialInfo.color * ambientColor * texColor;
    // vec3 ambient = orenNayarAmbient(normal, materialInfo.roughness, color * materialInfo.color, 1.0, vec3(1.0)) +
    //     cookTorrance(normal, viewDir, viewDir, materialInfo.roughness, ior);
    // ambient *= 0.1;
    // vec3 outColor = (diffuse + ambient) * color * vec3(1.0, 0.8, 0.2);
    // outColor = vec3(0);
    // vec3 spec = cookTorrance(normal, lightDir, viewDir, materialInfo.roughness, ior) * lightStrength;
    vec3 spec = cookTorrance(normal, lightDir, viewDir, roughness, ior) * lightStrength;

    vec3 specularAmbient = (1 - materialInfo.metallic) * (1 - orenNayar(normal, viewDir, viewDir, roughness, vec3(1.0))) * ambientColor * materialInfo.color * color;
    specularAmbient = max(specularAmbient, 0.0);
    
    vec3 outColor = diffuse + ambient;// * vec3(0.4, 0.2, 0.1);
    outColor *= (1 - materialInfo.metallic);
    // outColor += spec * mix(vec3(1.0, 1.0, 1.0), materialInfo.color * color, materialInfo.metallic);
    // outColor += spec + specularAmbient;
    outColor += spec;
    // outColor += F0 * materialInfo.metallic * (1 - orenNayar(normal, viewDir, viewDir, materialInfo.roughness, vec3(1.0))) * ambientColor;
    // outColor = vec3(uv, 0);

    // fragColor = vec4(1.0, 0.8, 0.2, 1.0);
    float linearDepth = gl_FragCoord.z / gl_FragCoord.w;
    // outColor = mix(outColor, ambientColor * 0.8, clamp(linearDepth / 100.0, 0.0, 1.0));
    outColor = mix(outColor, ambientColor * 0.8, (1 - exp(-linearDepth * 0.02)));
    // outColor = vec3(texture(heightMap, uv).r);
    fragColor = vec4(outColor, 1.0);
}