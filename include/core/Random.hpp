#ifndef RANDOM_HPP
#define RANDOM_HPP

#include <random>

class Random
{
private:
    std::random_device rd;
    std::mt19937 generateur;

public:
    Random();

    int entre(int min, int max);
    int lancerD20();
    int choisirPremierTour();
};

#endif