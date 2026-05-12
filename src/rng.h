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
            std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
            return dist(_generator);
        }

        template <std::floating_point T>
        T Get()
        {
            std::uniform_real_distribution<T> dist(std::numeric_limits<T>::min(), std::numeric_limits<T>::max());
            return dist(_generator);
        }

        template <std::integral T>
        T Range(T min_inclusive, T max_inclusive)
        {
            std::uniform_int_distribution<T> dist(min_inclusive, max_inclusive);
            return dist(_generator);
        }

        template <std::floating_point T>
        T Range(T min_inclusive, T max_inclusive)
        {
            std::uniform_real_distribution<T> dist(min_inclusive, max_inclusive);
            return dist(_generator);
        }
};
