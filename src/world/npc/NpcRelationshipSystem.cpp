#include "world/npc/NpcRelationshipSystem.hpp"

#include <algorithm>
#include <cctype>
#include <unordered_map>

namespace
{
std::string normalize(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string keyFor(const std::string& a, const std::string& b)
{
    std::string x = normalize(a);
    std::string y = normalize(b);
    if (x > y) std::swap(x, y);
    return x + "|" + y;
}

const std::unordered_map<std::string, NpcRelationship>& relationships()
{
    static const std::unordered_map<std::string, NpcRelationship> values = {
        {keyFor("Mira", "Orren"), {true, "confiance_professionnelle", "Ils se contredisent parfois, mais se transmettent vite les faits de route vérifiables.", 10}},
        {keyFor("Mira", "Bram"), {true, "coordination_locale", "Ils travaillent régulièrement sur les urgences matérielles de la ville.", 7}},
        {keyFor("Mira", "Soryn"), {true, "prudence_reciproque", "Mira veut agir vite, Soryn veut vérifier ; l'information circule mais garde ses réserves.", 4}},
        {keyFor("Orren", "Nell"), {true, "confiance_de_route", "Orren prend les retours de Nell au sérieux quand elle distingue ce qu'elle a vu de ce qu'on lui a raconté.", 9}},
        {keyFor("Bram", "Eda"), {true, "logistique", "Pièces, délais et stocks passent souvent par leurs deux registres.", 6}},
        {keyFor("Soryn", "Meron"), {true, "réseau_savant", "Ils privilégient les copies, traces et formulations exactes plutôt que les récits embellis.", 11}},
        {keyFor("Bob", "Maurice"), {true, "partenariat", "Ils ont l'habitude de travailler ensemble et comparent spontanément leurs versions.", 14}},
        {keyFor("Prunigil", "Meron"), {true, "méfiance_utile", "Ils partagent certains faits, mais chacun conserve volontiers ses réserves sur l'interprétation de l'autre.", -2}}
    };
    return values;
}
}

NpcRelationship NpcRelationshipSystem::between(const std::string& firstNpc, const std::string& secondNpc)
{
    if (firstNpc.empty() || secondNpc.empty() || normalize(firstNpc) == normalize(secondNpc)) return {};
    const auto it = relationships().find(keyFor(firstNpc, secondNpc));
    return it == relationships().end() ? NpcRelationship{} : it->second;
}

int NpcRelationshipSystem::relayModifier(const std::string& firstNpc, const std::string& secondNpc)
{
    return between(firstNpc, secondNpc).relayModifier;
}
