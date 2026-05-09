module shading_models;

import constants;
import colorspace;

// Diffuse Models
// Simple but inaccurate diffuse
public float DiffLambertian(float3 normal, float3 lightDir)
{
    return saturate(dot(normal, lightDir));
}

// OG Burley diffuse, does not conserve energy without F0 factor
public float DiffBurley(float3 normal, float3 lightDir, float3 viewDir, float roughness)
{
    float3 reflDir = -viewDir;
    float3 halfDir = normalize(lightDir - viewDir);

    float HdotL = dot(halfDir, lightDir);
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = dot(normal, reflDir);

    float F90 = 0.5 + 2 * roughness * HdotL * HdotL;
    float lightScatter = 1.0 + (F90 - 1.0) * pow(1 - NdotL, 5.0);
    float viewScatter = 1.0 + (F90 - 1.0) * pow(1 - NdotV, 5.0);

    float result = 1 / PI * lightScatter * viewScatter * NdotL;
    return max(result, 0);
}

// Accurate but most expensive diffuse
public float3 DiffOrenNayar(float3 normal, float3 lightDir, float3 viewDir, float roughness, float3 albedo)
{
    // clamp roughness to [0,1]
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

// Qualitative Oren-Nayar - Approximate but cheaper version of full Oren-Nayar
public float DiffQON(float3 normal, float3 lightDir, float3 viewDir, float roughness)
{
    float3 reflDir = -viewDir; // Reflectance direction

    // Keep only positive cosines (light/view above the surface)
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, reflDir), 0.0);

    if(NdotL <= 0.0) return 0.0;

    float sigma2 = roughness * roughness;
    float A = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    float B = 0.45 * (sigma2 / (sigma2 + 0.09));
    
    float3 L_perp = lightDir - normal * NdotL;
    float3 V_perp = -viewDir - normal * NdotV;
    float L_perp_len = max(1e-6, length(L_perp));
    float V_perp_len = max(1e-6, length(V_perp));

    float cos_phi_diff = (L_perp_len > 0.0 && V_perp_len > 0.0) 
                       ? saturate(dot(L_perp / L_perp_len, V_perp / V_perp_len)) 
                       : 1.0;
    
    float alpha = max(acos(NdotL), acos(NdotV));
    float beta  = min(acos(NdotL), acos(NdotV));
    
    float C = sin(alpha) * tan(beta);
    
    float oren_nayar = NdotL * (A + B * max(0.0, cos_phi_diff) * C);
    
    return oren_nayar / PI;
}

// Fujii Qualitative Oren-Nayar - Approximate but cheaper version of full Oren-Nayar, with improved coefficients.
public float3 DiffFON(float3 normal, float3 lightDir, float3 viewDir, float roughness, float3 albedo)
{
    float3 reflDir = -viewDir; // Reflectance direction

    // Keep only positive cosines (light/view above the surface)
    float NdotL = max(dot(normal, lightDir), 0.0);
    float NdotV = max(dot(normal, reflDir), 0.0);

    if(NdotL <= 0.0) return float3(0.0);

    float s = dot(lightDir, reflDir) - NdotL * NdotV;

    float t;
    if (s <= 0.0)
        t = 1.0;
    else
        t = max(NdotL, NdotV);

    // Option 1:
    // float sigma_prime = roughness;
    // float denom = PI + (PI/2.0 - 2.0/3.0) * sigma_prime;
    // float A = PI / denom;
    // float B = roughness * A;

    // Option 2: (closer to full ON)
    float sigma2 = roughness * roughness;
    float A = (1.0 - 0.5 * sigma2 / (sigma2 + 0.33) + 0.17 * luminance(albedo) * sigma2 / (sigma2 + 0.13));
    float B = 0.45 * sigma2 / (sigma2 + 0.09);
    
    float extra = B * (s / t);
    
    float oren_nayar = NdotL * (A + extra);
    
    return albedo * oren_nayar / PI;
}

static const float c1FON = 0.5 - 2.0 / (3.0 * PI);
static const float c2FON = 2.0 / 3.0 - 28.0 / (15.0 * PI);
float eFONexact(float NdotV, float roughness)
{
    float A = 1.0 / (1.0 + c1FON * roughness);
    float B = roughness * A;
    float Si = sqrt(1.0 - NdotV * NdotV);
    float G = Si * (acos(NdotV) - Si * NdotV) + (2.0 / 3.0) * ((Si / NdotV) * (1.0 - (Si * Si * Si)) - Si);
    return A + (B * INVPI) * G;
}

float eFONapprox(float NdotV, float roughness)
{
    float mucomp = 1.0 - NdotV;
    const float g1 = 0.0571085289;
    const float g2 = 0.491881867;
    const float g3 = -0.332181442;
    const float g4 = 0.0714429953;
    float GoverPi = mucomp * (g1 + mucomp * (g2 + mucomp * (g3 + mucomp * g4)));
    return (1.0 + roughness * GoverPi) / (1.0 + c1FON * roughness);
}

// Energy conserving Oren-Nayar - Improved ON
public float3 DiffEON(float3 normal, float3 lightDir, float3 viewDir, float roughness, float3 albedo, bool exact)
{
    float3 reflDir = -viewDir; // Reflectance direction

    // float sigma = roughness * PI / 2;
    float sigma = roughness;

    // Keep only positive cosines (light/view above the surface)
    float NdotL = dot(normal, lightDir);
    float NdotV = dot(normal, reflDir);

    if(NdotL <= 0.0) return 0.0;

    float s = dot(lightDir, reflDir) - NdotL * NdotV;
    float sovertF = s > 0.0 ? s / max(NdotL, NdotV) : s;
    float A = 1.0 / (1.0 + c1FON * roughness);
    float3 f_ss = albedo * INVPI * A * (1.0 + roughness * sovertF);

    float EFo = exact ? eFONexact(NdotL, roughness) : eFONapprox(NdotL, roughness);
    float EFi = exact ? eFONexact(NdotV, roughness) : eFONapprox(NdotV, roughness);
    float avgEF = A * (1.0 + c2FON * roughness);
    float3 rho_ms = (albedo * albedo) * avgEF / (float3(1.0) - albedo * (1.0 - avgEF));
    float3 f_ms = rho_ms * INVPI * max(EPSILON, 1.0 - EFo) * max(EPSILON, 1.0 - EFi) / max(EPSILON, 1.0 - avgEF);
    // return float3(sovertF);
    return (f_ss + f_ms) * NdotL;

    // float sigma2 = sigma * sigma;
    // float A = 1.0 - 0.5 * (sigma2 / (sigma2 + 0.33));
    // float B = 0.45 * (sigma2 / (sigma2 + 0.09));

    // float3 L_perp = normalize(lightDir - normal * NdotL);
    // float3 V_perp = normalize(reflDir - normal * NdotV);
    // float cos_phi_diff = dot(L_perp, V_perp);

    // float alpha = max(acos(NdotL), acos(NdotV));
    // float beta  = min(acos(NdotL), acos(NdotV));
    
    // float C = sin(alpha) * tan(beta);
    // float f_FON = NdotL * (A + B * max(0.0, cos_phi_diff) * C);

    // float EFo = exact ? eFONexact(NdotV, roughness) : eFONapprox(NdotV, roughness);
    

    // return f_FON;
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

    // float3 F_diffuse = lerp(F0, F_L, 1.0 - roughness); // Effect on diffuse based on roughness
    float3 F = SchlickFresnel(F0, normal, lightDir);             // Reflectance with respect to the light direction
    float3 kD = (1.0 - metallic) * (1.0 - F);
    
    float3 diffuse = kD * DiffEON(normal, lightDir, viewDir, roughness, albedo, true);
    // float3 diffuse = kD * DiffFON(normal, lightDir, viewDir, roughness, albedo);
    // float3 diffuse = kD * DiffOrenNayar(normal, lightDir, viewDir, roughness, albedo);
    // float3 diffuse = kD * DiffBurley(normal, lightDir, viewDir, roughness) * albedo;
    // float3 diffuse = DiffBurley(normal, lightDir, viewDir, roughness) * albedo;
    // float3 diffuse = DiffLambertian(normal, lightDir) * albedo / PI;
    float3 specular = SpecCookTorrance(normal, lightDir, viewDir, roughness, F0);
    // float3 specTint = lerp(float3(1), albedo, 0.5);
    // specular *= specTint;
    
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
    // metallic = 1;
    F0 = lerp(F0, albedo, metallic); // Metals use albedo for F0

    float3 F_V = saturate(SchlickFresnel(F0, normal, -viewDir)); // Reflectance with respect to the view direction
    F_V *= lerp(1.0, F0, metallic);
    float alpha = roughness * roughness;
    float3 F_ambient = lerp(F0, F_V, (1.0 - roughness) * (1.0 - roughness) * (1.0 - roughness) * (1.0 - roughness)); // Metallic term is more realistic, but looks worse without reflection map

    float3 ambientDiffuse = ambientColor * albedo * (1.0 - F0.r) * (1.0 - metallic);
    float3 ambientSpecular = reflectSample * F_ambient;

    return ambientDiffuse + ambientSpecular;
    // return float3(0);
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