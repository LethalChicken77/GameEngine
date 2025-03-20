#pragma once
#include <random>
#include <bit>

namespace core
{
    extern std::mt19937 rng;
    class Random
    {
        public:
            static float getRandom01(uint32_t seed)
            {
                rng.seed(seed);
                static std::uniform_real_distribution<float> distribution;
                return distribution(rng);
            }

            static float getRandom01()
            {
                std::random_device rd;
                return getRandom01(rd());
            }

            static int getRandomInt(int min, int max, uint32_t seed)
            {
                rng.seed(seed);
                std::uniform_int_distribution<int> distribution(min, max);
                return distribution(rng);
            }

            static int getRandomInt(int min, int max)
            {
                std::random_device rd;
                return getRandomInt(min, max, rd());
            }

            static float getRandomFloat(float min, float max, uint32_t seed)
            {
                rng.seed(seed);
                std::uniform_real_distribution<float> distribution(min, max);
                return distribution(rng);
            }

            static float getRandomFloat(float min, float max)
            {
                std::random_device rd;
                return getRandomFloat(min, max, rd());
            }

            // MurmurHash3-like function for better hash distribution
            static constexpr uint32_t MurmurHash3(uint32_t h)
            {
                h ^= h >> 16;
                h *= 0x85ebca6b;
                h ^= h >> 13;
                h *= 0xc2b2ae35;
                h ^= h >> 16;
                return h;
            }

            // 2D Seed Function
            static uint32_t SeedFrom2D(int x, int y, int baseSeed = 0)
            {
                uint32_t hash = MurmurHash3(x) ^ MurmurHash3(y + 1) ^ MurmurHash3(baseSeed);
                return hash;
            }

            // 3D Seed Function
            static uint32_t SeedFrom3D(int x, int y, int z, int baseSeed = 0)
            {
                uint32_t hash = MurmurHash3(x) ^ MurmurHash3(y + 1) ^ MurmurHash3(z + 2) ^ MurmurHash3(baseSeed);
                return hash;
            }

            // 4D Seed Function
            static uint32_t SeedFrom4D(int x, int y, int z, int w, int baseSeed = 0)
            {
                uint32_t hash = MurmurHash3(x) ^ MurmurHash3(y + 1) ^ MurmurHash3(z + 2) ^ MurmurHash3(w + 3) ^ MurmurHash3(baseSeed);
                return hash;
            }

            // 2D Seed Function
            static uint32_t SeedFrom2D(float x, float y, int baseSeed = 0)
            {
                return SeedFrom2D(std::bit_cast<int>(x), std::bit_cast<int>(y), baseSeed);
            }

            // 3D Seed Function
            static uint32_t SeedFrom3D(float x, float y, float z, int baseSeed = 0)
            {
                return SeedFrom3D(std::bit_cast<int>(x), std::bit_cast<int>(y), std::bit_cast<int>(z), baseSeed);
            }

            // 4D Seed Function
            static uint32_t SeedFrom4D(float x, float y, float z, float w, int baseSeed = 0)
            {
                return SeedFrom4D(std::bit_cast<int>(x), std::bit_cast<int>(y), std::bit_cast<int>(z), std::bit_cast<int>(w), baseSeed);
            }
    };
} // namespace core