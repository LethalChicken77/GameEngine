module shading_models;

import constants;
import colorspace;

// Diffuse Models
public float DiffLambertian(float3 normal, float3 lightDir)
{
    return saturate(dot(normal, lightDir));
}

public float3 DiffOrenNayar(float3 normal, float3 lightDir, float3 viewDir, float3 albedo, float roughness)
{
    // clamp roughness to [0,1]
    roughness = clamp(roughness, 0.0, 1.0);
    float3 reflDir = -viewDir; // Reflectance direction

    // Keep only positive cosines (light/view above the surface)
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, reflDir), 0.0);
    float LdotV = dot(lightDir, reflDir); // used only in cos(phi) calc

    float sigma2 = roughness * roughness;

    // angles
    float thetaI = acos(clamp(NdotL, -1.0, 1.0));
    float thetaR = acos(clamp(NdotV, -1.0, 1.0));

    float alpha = max(thetaR, thetaI);
    float beta = min(thetaR, thetaI);

    // cos(phi_i - phi_r)
    float sinThetaI = sqrt(max(0.0, 1.0 - NdotL * NdotL));
    float sinThetaR = sqrt(max(0.0, 1.0 - NdotV * NdotV));
    float cosPhi = 0.0;
    if (sinThetaI > 1e-5 && sinThetaR > 1e-5)
    {
        cosPhi = clamp((LdotV - NdotL * NdotV) / (sinThetaI * sinThetaR), -1.0, 1.0);
    }

    float C1 = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float C2 = 0.45 * sigma2 / (sigma2 + 0.09);
    if (cosPhi >= 0)
        C2 *= sin(alpha);
    else
        C2 *= sin(alpha) - pow(2 * beta / PI, 3);
    
    float C3 = 0.125 * sigma2 / (sigma2 + 0.09) * pow(4 * alpha * beta / (PI * PI), 2);

    float3 L1 = albedo * INVPI * NdotL * (C1 + C2 * cosPhi * tan(clamp(beta, -HALFPI + EPSILON, HALFPI - EPSILON)) + C3 * (1.0 - abs(cosPhi)) * tan((0.5 - EPSILON) * (alpha + beta)));
    float3 L2 = 0.17 * albedo * albedo * INVPI * NdotL * sigma2 / (sigma2 + 0.13) * (1 - cosPhi * pow(2 * beta * INVPI, 2));
    return L1 + L2;
}

public float3 SchlickFresnel(float3 F0, float3 halfDir, float3 reflDir)
{
    return F0 + (1.0 - F0) * pow(1.0 - dot(halfDir, reflDir), 5.0);
}

// Specular Models
public float SpecPhong(float3 normal, float3 lightDir, float3 viewDir, float exponent = 80)
{
    float3 reflDir = reflect(lightDir, normal);
    return clamp(pow((dot(reflDir, viewDir)), exponent), 0, 1000);
}

inline float G1(float NdotX, float alpha)
{
    float a = alpha * alpha;
    float b = NdotX * NdotX;
    return 2 * NdotX / (NdotX + sqrt(a + (1 - a) * b));
}

public float3 SpecCookTorrance(float3 normal, float3 lightDir, float3 viewDir, float roughness, float3 F0)
{
    float3 reflDir = -viewDir;
    float3 halfDir = normalize(lightDir + reflDir);

    float NdotH = saturate(dot(normal, halfDir));
    float HdotR = dot(halfDir, reflDir);
    float NdotL = clamp(dot(normal, lightDir), EPSILON, 1.0);;
    float NdotR = clamp(dot(normal, reflDir), EPSILON, 1.0);
    // float D = (1.0 / (PI * roughness * roughness)) * pow(dot(halfDir, normal), 2.0 / (roughness * roughness) - 2.0); // Beckmann
    float alpha = roughness * roughness;
    float D = alpha * alpha / (PI * pow(NdotH * NdotH * (alpha * alpha - 1.0) + 1.0, 2.0)); // GGX

    // float G = min(1.0, min(2.0 * NdotH * dot(normal, reflDir) / dot(reflDir, halfDir), 2.0 * NdotH * dot(normal, lightDir) / dot(reflDir, halfDir))); // Cook-Torrance
    
    float G = G1(NdotL, alpha) * G1(NdotR, alpha);

    float3 F = SchlickFresnel(F0, halfDir, reflDir);
    // float3 specular = D * G * F / (4.0 * dot(normal, lightDir) * dot(normal, reflDir));
    float3 specular = D * G * F / (4.0 * NdotL * NdotR);
    specular *= NdotL;
    return max(specular, 0.0);
    // return float3(G);
}

public float3 PhongBRDF(
    float3 albedo,
    float specularExponent,

    float3 lightColor,
    float3 ambientLight,

    float3 normal, 
    float3 lightDir, 
    float3 viewDir
    )
{

    float3 diffuse = albedo * lightColor * DiffLambertian(normal, lightDir);
    float3 specular = lightColor * SpecPhong(normal, lightDir, viewDir, specularExponent);
    float3 ambient = ambientLight * albedo;

    return diffuse + specular + ambient;
}

public float3 pbrBRDF(
    float3 albedo,
    float roughness,
    float metallic,
    float3 F0,

    float3 lightColor,
    
    float3 normal, 
    float3 lightDir,
    float3 viewDir
)
{
    // float3 F0 = (ior - 1.0) * (ior - 1.0) / ((ior + 1.0) * (ior + 1.0));
    F0 = lerp(F0, albedo, metallic); // Metals use albedo for F0
    float alpha = roughness * roughness;
    float3 reflDir = reflect(viewDir, normal);

    float3 F_L = SchlickFresnel(F0, normal, lightDir);             // Reflectance with respect to the light direction
    float3 F_diffuse = lerp(F0, F_L, 1.0 - roughness); // Effect on diffuse based on roughness
    
    float3 diffuse = (1 - metallic) * (1 - F_diffuse) * DiffOrenNayar(normal, lightDir, viewDir, albedo, roughness);
    float3 specular = SpecCookTorrance(normal, lightDir, viewDir, roughness, F0);
    
    return lightColor * (diffuse + specular);
}

public float3 pbrAmbient(
    float3 albedo,
    float roughness,
    float metallic,
    float3 F0,

    float3 ambientColor,
    float3 reflectSample, // Specular ambient color

    float3 normal,
    float3 viewDir
)
{
    F0 = lerp(F0, albedo, metallic); // Metals use albedo for F0

    float3 F_V = saturate(SchlickFresnel(float3(length(F0)), normal, -viewDir)); // Reflectance with respect to the view direction
    F_V *= lerp(1.0, F0, metallic);
    float3 F_ambient = lerp(F0, F_V, (1.0 - roughness)); // Metallic term is more realistic, but looks worse without reflection map

    float3 ambientDiffuse = ambientColor * albedo * (1.0 - F_ambient) * (1.0 - metallic);
    float3 ambientSpecular = reflectSample * F_ambient;

    return ambientDiffuse + ambientSpecular;
}

public float3 goochBRDF(
    float3 cool,
    float3 warm,
    float4 outline,
    float outlinePower,
    float roughness,
    float3 F0,

    float3 lightColor,

    float3 normal,
    float3 lightDir,
    float3 viewDir)
{
    // float3 F0 = (ior - 1.0) * (ior - 1.0) / ((ior + 1.0) * (ior + 1.0));
    float NdotL = dot(normal, lightDir) * 0.5 + 0.5;
    float alpha = roughness * roughness;
    float3 reflDir = reflect(viewDir, normal);

    // float3 F_V = saturate(SchlickFresnel(float3(length(F0)), normal, -viewDir));
    float3 F_V = saturate(dot(normal, viewDir) + 1);

    float3 specular = SpecCookTorrance(normal, lightDir, viewDir, roughness, F0);
    float3 coolOKLab = LinearToOklab(cool);
    float3 warmOKLab = LinearToOklab(warm);
    float3 diffuse = lerp(coolOKLab, warmOKLab, NdotL);
    diffuse = OklabToLinear(diffuse);
    F_V = pow(F_V, outlinePower);

    return lerp((lightColor * specular + diffuse), outline.xyz, F_V * outline.w);
}