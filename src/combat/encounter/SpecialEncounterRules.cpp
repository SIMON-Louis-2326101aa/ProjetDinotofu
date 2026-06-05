// EN: SpecialEncounterRules.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: SpecialEncounterRules.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file belongs to Dinotofu. Code identifiers are written in English; player-facing text can stay in French.
// Français : Ce fichier appartient à Dinotofu. Les identifiants du code sont en anglais ; les textes affichés au joueur peuvent rester en français.
// Description: Implements future rules for special encounter intent and lethal combat permissions.

#include "combat/encounter/SpecialEncounterRules.hpp"

#include <algorithm>

// EN: getIntentForSpecialGroup declares or implements a focused behavior used by this module.
// FR: getIntentForSpecialGroup déclare ou implémente un comportement précis utilisé par ce module.
CombatIntent SpecialEncounterRules::getIntentForSpecialGroup(const std::vector<std::string>& names)
{
    bool hasHazak = containsName(names, "Hazak");
    bool hasSkuro = containsName(names, "Skuro");
    bool hasHestia = containsName(names, "Hestia");
    bool hasFail = containsName(names, "Fail");
    bool hasFireFlight = containsName(names, "Fire Flight");
    bool hasMattPro = containsName(names, "Matt (PRO)");
    bool hasMattzelda = containsName(names, "Mattzelda");
    bool hasLouis = containsName(names, "Louis");
    bool hasTrexof = containsName(names, "Trexof");
    bool hasAoi = containsName(names, "Aoi");
    bool hasKanade = containsName(names, "Kanadé");
    bool hasSanctus = containsName(names, "Sanctus");
    bool hasLesBrasCasses = hasHazak && hasFail && hasAoi && hasKanade && hasSanctus;

    if (hasLesBrasCasses)
    {
        return CombatIntent::Training;
    }

    if (hasHazak && hasHestia)
    {
        return CombatIntent::SeriousFight;
    }

    if (hasHazak && hasFail)
    {
        return CombatIntent::ExperimentalFight;
    }

    if (hasFireFlight && hasMattPro)
    {
        return CombatIntent::Training;
    }

    if (hasSkuro)
    {
        return CombatIntent::DangerousFight;
    }

    if (hasHazak)
    {
        return CombatIntent::DangerousFight;
    }

    if (hasMattzelda && hasLouis && hasTrexof)
    {
        return CombatIntent::FriendlyDuel;
    }

    if (hasAoi && hasKanade && hasSanctus)
    {
        return CombatIntent::Training;
    }

    return CombatIntent::Training;
}

std::string SpecialEncounterRules::getIntentText(
    CombatIntent intent,
    const std::vector<std::string>& names
)
{
    bool hasHazak = containsName(names, "Hazak");
    bool hasSkuro = containsName(names, "Skuro");
    bool hasHestia = containsName(names, "Hestia");
    bool hasFail = containsName(names, "Fail");
    bool hasFireFlight = containsName(names, "Fire Flight");
    bool hasMattPro = containsName(names, "Matt (PRO)");
    bool hasMattzelda = containsName(names, "Mattzelda");
    bool hasLouis = containsName(names, "Louis");
    bool hasTrexof = containsName(names, "Trexof");
    bool hasAoi = containsName(names, "Aoi");
    bool hasKanade = containsName(names, "Kanadé");
    bool hasSanctus = containsName(names, "Sanctus");
    bool hasHenrique = containsName(names, "Henrique");
    bool hasLesBrasCasses = hasHazak && hasFail && hasAoi && hasKanade && hasSanctus;

    if (hasLesBrasCasses)
    {
        return "Intention : rencontre PvE héroïque. Les bras cassés testent ton niveau en groupe complet, sans chercher la mort par défaut, mais avec beaucoup trop d'ego pour promettre un combat propre.";
    }

    if (hasHazak && hasHestia && hasSanctus)
    {
        return "Intention : protection sacrée. Hazak refuse le massacre, Sanctus absorbe la menace, Hestia ne doit pas revivre son pire souvenir.";
    }

    if (hasFail && hasAoi && hasKanade)
    {
        return "Intention : laboratoire magique. Fail teste, Kanadé surcharge, Aoi stabilise juste assez pour éviter la catastrophe totale.";
    }

    if (hasLouis && hasFireFlight && hasTrexof)
    {
        return "Intention : duel d'improvisation entre amis. Fire Flight commande, Louis improvise, Trexof cherche la faille utile.";
    }

    if (hasHenrique && hasMattzelda && hasSkuro)
    {
        return "Intention : charge instable. Henrique tient debout, Mattzelda attire l'attention, Skuro rend chaque erreur dangereuse.";
    }

    if (hasHazak && hasHestia)
    {
        return "Intention : combat sérieux, mais pas à mort. Hazak refuse de traumatiser Hestia avec un assassinat.";
    }

    if (hasHazak && hasFail)
    {
        return "Intention : expérience sous contrat. Fail respecte Hazak, mais l'arène risque quand même de prendre feu.";
    }

    if (hasFireFlight && hasMattPro)
    {
        return "Intention : test de commandement. Fire Flight mène, Matt (PRO) agit en silence et respecte l'adversaire.";
    }

    if (hasSkuro)
    {
        return "Intention : combat très dangereux. Skuro ne promet jamais que toutes les têtes resteront attachées.";
    }

    if (hasHazak)
    {
        return "Intention : combat dangereux. Hazak est aussi un meneur, et il sait transformer un duel en exécution propre.";
    }

    switch (intent)
    {
        case CombatIntent::FriendlyDuel:
            return "Intention : duel amical brutal. Ça tape fort, mais le groupe retient encore les coups mortels.";

        case CombatIntent::Training:
            return "Intention : entraînement sérieux. Le groupe veut tester ton niveau sans forcément te tuer.";

        case CombatIntent::Warning:
            return "Intention : avertissement. Le groupe veut te faire comprendre que tu es allé trop loin.";

        case CombatIntent::ExperimentalFight:
            return "Intention : expérimentation. Quelqu'un veut apprendre quelque chose, et tu es malheureusement le terrain de test.";

        case CombatIntent::DangerousFight:
            return "Intention : combat dangereux. Une erreur peut transformer l'entraînement en blessure permanente.";

        case CombatIntent::DeathMatch:
            return "Intention : combat à mort. L'arène ne promet aucun retour.";

        case CombatIntent::StoryLocked:
            return "Intention : rencontre liée au récit. Quelque chose dépasse le simple hasard de l'arène.";

        case CombatIntent::SeriousFight:
        default:
            return "Intention : combat sérieux. La mort n'est pas recherchée, sauf si tu pousses le groupe à franchir la limite.";
    }
}

// EN: canBecomeDeathMatch declares or implements a focused behavior used by this module.
// FR: canBecomeDeathMatch déclare ou implémente un comportement précis utilisé par ce module.
bool SpecialEncounterRules::canBecomeDeathMatch(const std::vector<std::string>& names)
{
    bool hasHazak = containsName(names, "Hazak");
    bool hasSkuro = containsName(names, "Skuro");
    bool hasHestia = containsName(names, "Hestia");
    bool hasLesBrasCasses = hasHazak
        && containsName(names, "Fail")
        && containsName(names, "Aoi")
        && containsName(names, "Kanadé")
        && containsName(names, "Sanctus");

    if (hasLesBrasCasses)
    {
        return false;
    }

    if (hasHazak && hasHestia)
    {
        return false;
    }

    return hasHazak || hasSkuro;
}

bool SpecialEncounterRules::containsName(
    const std::vector<std::string>& names,
    const std::string& expectedName
)
{
    return std::find(names.begin(), names.end(), expectedName) != names.end();
}
