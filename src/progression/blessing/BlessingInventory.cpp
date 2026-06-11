// English: Persistent container for active character blessings.
// Français : Conteneur persistant des bénédictions actives d'un personnage.
#include "progression/blessing/BlessingInventory.hpp"

const std::vector<Blessing>& BlessingInventory::getAll() const
{
    return blessings;
}

int BlessingInventory::count() const
{
    return static_cast<int>(blessings.size());
}

bool BlessingInventory::empty() const
{
    return blessings.empty();
}

bool BlessingInventory::contains(const std::string& blessingId) const
{
    for (const Blessing& blessing : blessings)
    {
        if (blessing.getId() == blessingId)
        {
            return true;
        }
    }
    return false;
}

bool BlessingInventory::hasLethalSurvivalProtection() const
{
    for (const Blessing& blessing : blessings)
    {
        if (blessing.grantsLethalSurvival())
        {
            return true;
        }
    }
    return false;
}

bool BlessingInventory::add(const Blessing& blessing)
{
    if (!blessing.isValid() || contains(blessing.getId()))
    {
        return false;
    }
    blessings.push_back(blessing);
    return true;
}

void BlessingInventory::clear()
{
    blessings.clear();
}

void BlessingInventory::setLoaded(const std::vector<Blessing>& loadedBlessings)
{
    blessings.clear();
    for (const Blessing& blessing : loadedBlessings)
    {
        add(blessing);
    }
}
