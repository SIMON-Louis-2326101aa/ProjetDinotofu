#include "core/Random.hpp"

Random::Random() : generateur(rd())
{
}

int Random::entre(int min, int max)
{
    std::uniform_int_distribution<> distribution(min, max);
    return distribution(generateur);
}

int Random::lancerD20()
{
    return entre(1, 20);
}

int Random::choisirPremierTour()
{
    return entre(1, 2);
}