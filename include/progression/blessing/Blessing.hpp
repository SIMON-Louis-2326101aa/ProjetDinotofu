// English: Persistent blessing data used by divine protections and lethal survival anomalies.
// Français : Donnée persistante de bénédiction utilisée par les protections divines et anomalies de survie léthale.
#ifndef INCLUDE_PROGRESSION_BLESSING_BLESSING_HPP
#define INCLUDE_PROGRESSION_BLESSING_BLESSING_HPP

#include <string>

class Blessing
{
private:
    std::string id;
    std::string name;
    std::string description;
    bool survivalProtection;

public:
    Blessing();
    Blessing(
        const std::string& id,
        const std::string& name,
        const std::string& description,
        bool survivalProtection = false
    );

    const std::string& getId() const;
    const std::string& getName() const;
    const std::string& getDescription() const;
    bool grantsLethalSurvival() const;
    bool isValid() const;
};

#endif
