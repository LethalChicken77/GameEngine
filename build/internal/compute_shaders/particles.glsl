#version 450

struct Particle
{
    vec2 position;
    vec2 velocity;
    float sediment; // Sediment to pick up and deposit
    float lifeTime;
};

// Input buffer (bound at set 0, binding 0)
layout(set = 0, binding = 0, std140) buffer DataBuffer
{
    Particle data[];
};

layout(local_size_x = 16, local_size_y = 1, local_size_z = 1) in;
void main()
{
    uint index = gl_GlobalInvocationID.x; // Example index mapping
    if (index >= data.length()) return;

    Particle p = data[index];
    p.position += p.velocity;
    p.lifeTime -= 0.01;
    data[index] = p;
}