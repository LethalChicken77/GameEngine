module shading_models;

import constants;

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

    // compute cos(phi_i - phi_r) robustly
    float sinThetaI = sqrt(max(0.0, 1.0 - NdotL * NdotL));
    float sinThetaR = sqrt(max(0.0, 1.0 - NdotV * NdotV));
    float cosPhi = 0.0;
    if (sinThetaI > 1e-5 && sinThetaR > 1e-5)
    {
        cosPhi = clamp((LdotV - NdotL * NdotV) / (sinThetaI * sinThetaR), -1.0, 1.0);
    }

    // Oren-Nayar A and B
    float C1 = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float C2 = 0.45 * sigma2 / (sigma2 + 0.09);
    if (cosPhi >= 0)
        C2 *= sin(alpha);
    else
        C2 *= sin(alpha) - pow(2 * beta / PI, 3);
    
    float C3 = 0.125 * sigma2 / (sigma2 + 0.09) * pow(4 * alpha * beta / (PI * PI), 2);
    
    float3 L1 = albedo * INVPI * NdotL * (C1 + C2 * cosPhi * tan(beta) + C3 * (1.0 - abs(cosPhi)) * tan(0.5 * (alpha + beta)));
    float3 L2 = 0.17 * albedo * albedo * INVPI * NdotL * sigma2 / (sigma2 + 0.13) * (1 - cosPhi * pow(2 * beta * INVPI, 2));
    return L1 + L2;
}

public float SpecPhong(float3 normal, float3 lightDir, float3 viewDir, float exponent = 80)
{
    float3 reflDir = reflect(lightDir, normal);
    return clamp(pow((dot(reflDir, viewDir)), exponent), 0, 1000);
}

public float3 SchlickFresnel(float3 F0, float3 halfDir, float3 reflDir)
{
    return F0 + (1.0 - F0) * pow(1.0 - dot(halfDir, reflDir), 5.0);
}


public float3 SpecCookTorrance(float3 normal, float3 lightDir, float3 viewDir, float roughness, float3 F0)
{
    float3 reflDir = -viewDir;
    float3 halfDir = normalize(lightDir + reflDir);

    float NdotH = saturate(dot(normal, halfDir));
    // float D = (1.0 / (PI * roughness * roughness)) * pow(dot(halfDir, normal), 2.0 / (roughness * roughness) - 2.0); // Beckmann
    float alpha = roughness * roughness;
    float D = alpha * alpha / (PI * pow(NdotH * NdotH * (alpha * alpha - 1.0) + 1.0, 2.0)); // GGX
    float G = min(1.0, min(2.0 * NdotH * dot(normal, reflDir) / abs(dot(reflDir, halfDir)), 2.0 * NdotH * dot(normal, lightDir) / abs(dot(reflDir, halfDir))));

    float3 F = SchlickFresnel(F0, halfDir, reflDir);
    float3 specular = D * G * F / (4.0 * dot(normal, lightDir) * dot(normal, reflDir));
    specular *= dot(normal, lightDir);
    return max(specular, 0.0);
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
    float3 specular = lightColor * SpecPhong(normal, lightDir, viewDir, 80.0);
    float3 ambient = ambientLight * albedo;

    return diffuse + specular + ambient;
}

public float3 pbrBRDF(
    float3 albedo,
    float roughness,
    float metallic,

    float3 lightColor,
    float3 ambientColor,
    
    float3 normal, 
    float3 lightDir,
    float3 viewDir
)
{
    // float3 F0 = (ior - 1.0) * (ior - 1.0) / ((ior + 1.0) * (ior + 1.0));
    float3 F0 = float3(0.04); // Typical for dielectrics
    F0 = lerp(F0, albedo, metallic); // Metals use albedo for F0

    float3 F_L = SchlickFresnel(F0, normal, lightDir); // Reflectance with respect to the light direction
    float3 F_diffuse = lerp(F0, F_L, 1.0 - roughness); // Effect on diffuse based on roughness

    float3 diffuse = DiffOrenNayar(normal, lightDir, viewDir, albedo, roughness);
    float3 specular = SpecCookTorrance(normal, lightDir, viewDir, roughness, F0);
    return lightColor * ((1 - metallic) * (1 - F_diffuse) * diffuse + specular) + ambientColor;
}