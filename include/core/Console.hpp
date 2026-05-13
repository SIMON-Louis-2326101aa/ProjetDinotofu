#ifndef CONSOLE_HPP
#define CONSOLE_HPP

#include <string>

class Console
{
public:
    static void clear();
    static void pauseSecondes(int secondes);
    static void attendreEntree();

    static int demanderNombreEntre(int min, int max, const std::string& messageErreur);

    static void viderBufferEntreeDisponible();
};

#endif