module utils;
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