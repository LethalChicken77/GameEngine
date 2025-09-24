module utils;
import constants;
// TODO: Add support for 2x2 and 4x4 matrix inverse
public float3x3 inverse(float3x3 m)
{
    float det = determinant(m);
    return transpose(float3x3(
        cross(m[1], m[2]),
        cross(m[2], m[0]),
        cross(m[0], m[1])
    )) / det;
}

public float2 cartesianToSphericalNorm(float3 dir)
{
    float theta = -atan2(dir.z, dir.x);     // Azimuthal angle
    float phi = acos(dir.y / length(dir)); // Polar angle
    theta = theta / (2.0 * PI) + 0.5;      // Normalize to [0,1]
    phi = phi / PI;                        // Normalize to [0,1]
    return float2(theta, phi);
}

public float3 cartesianToSpherical(float3 dir)
{
    float rho = length(dir);
    if (rho < EPSILON) return float3(0, 0, 0); // Avoid division by zero
    float theta = atan2(dir.z, dir.x); // Azimuthal angle
    float phi = acos(dir.y / rho); // Polar angle
    theta = theta / (2.0 * PI) + 0.5; // Normalize to [0,1]
    phi = phi / PI; // Normalize to [0,1]
    return float3(rho, theta, phi);
}

public float angleBetween(float3 v1, float3 v2)
{
    return acos(clamp(dot(v1, v2) / (length(v1) * length(v2)), -1.0f + 1e-6f, 1.0f - 1e-6f));
}

public float Hash(float2 p)
{
    // Use a dot product with irrational numbers, then apply sin+frac for mixing
    float h = dot(p, float2(127.1, 311.7));
    return frac(sin(h) * 43758.5453);
}