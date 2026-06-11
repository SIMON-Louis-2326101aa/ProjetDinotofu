// English: Persistent blessing data used by divine protections and lethal survival anomalies.
// Français : Donnée persistante de bénédiction utilisée par les protections divines et anomalies de survie léthale.
#include "progression/blessing/Blessing.hpp"

Blessing::Blessing()
    : id(""), name(""), description(""), survivalProtection(false)
{
}

Blessing::Blessing(
    const std::string& id,
    const std::string& name,
    const std::string& description,
    bool survivalProtection
)
    : id(id),
      name(name.empty() ? id : name),
      description(description),
      survivalProtection(survivalProtection)
{
}

const std::string& Blessing::getId() const
{
    return id;
}

const std::string& Blessing::getName() const
{
    return name;
}

const std::string& Blessing::getDescription() const
{
    return description;
}

bool Blessing::grantsLethalSurvival() const
{
    return survivalProtection;
}

bool Blessing::isValid() const
{
    return !id.empty();
}
