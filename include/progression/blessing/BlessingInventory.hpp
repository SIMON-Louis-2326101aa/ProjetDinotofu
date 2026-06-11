// English: Persistent container for active character blessings.
// Français : Conteneur persistant des bénédictions actives d'un personnage.
#ifndef INCLUDE_PROGRESSION_BLESSING_BLESSINGINVENTORY_HPP
#define INCLUDE_PROGRESSION_BLESSING_BLESSINGINVENTORY_HPP

#include "progression/blessing/Blessing.hpp"

#include <string>
#include <vector>

class BlessingInventory
{
private:
    std::vector<Blessing> blessings;

public:
    const std::vector<Blessing>& getAll() const;
    int count() const;
    bool empty() const;
    bool contains(const std::string& blessingId) const;
    bool hasLethalSurvivalProtection() const;
    bool add(const Blessing& blessing);
    void clear();
    void setLoaded(const std::vector<Blessing>& loadedBlessings);
};

#endif
