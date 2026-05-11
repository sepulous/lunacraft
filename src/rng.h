#pragma once

#include <random>
#include <concepts>
#include <limits>

class RNG
{
    private:
        std::mt19937_64 _generator;

    public:
        RNG()
        {
            _generator.seed(std::random_device{}());
        }

        RNG(uint64_t seed)
        {
            _generator.seed(seed);
        }

        void Seed(uint64_t seed)
        {
            _generator.seed(seed);
        }

        template <std::integral T>
        T Get()
        {
            std::uniform_int_distribution<> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
            return dist(_generator);
        }

        template <std::floating_point T>
        T Get()
        {
            std::uniform_real_distribution<> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
            return dist(_generator);
        }

        template <std::integral T, std::integral U>
        T Range(T min_inclusive, U max_inclusive)
        {
            std::uniform_int_distribution<> dist(min_inclusive, max_inclusive);
            return dist(_generator);
        }

        template <std::floating_point T, std::floating_point U>
        T Range(T min_inclusive, U max_inclusive)
        {
            std::uniform_real_distribution<> dist(min_inclusive, max_inclusive);
            return dist(_generator);
        }
};
