#include "world/crime/CrimeContactDialogueSystem.hpp"

#include "entity/Player.hpp"
#include "world/crime/CrimeContactRelationshipSystem.hpp"
#include "world/crime/VaultTheftNetworkCatalog.hpp"

CrimeContactDialogueSnapshot CrimeContactDialogueSystem::build(const Player& player, const std::string& contactId)
{
    CrimeContactDialogueSnapshot result;
    const PlayerCrimeContactState* state = player.findCrimeContactState(contactId);
    if (state == nullptr) return result;

    const CrimeContactRelationshipAssessment relation = CrimeContactRelationshipSystem::assess(player, contactId);
    result.known = relation.known;
    result.available = relation.available;
    result.ruptured = relation.ruptured;
    result.contactName = state->displayName.empty() ? contactId : state->displayName;

    if (state->arrested)
    {
        result.lines.push_back(result.contactName + " n'est plus disponible comme relais : une arrestation durable a été prononcée sur un dossier suffisamment étayé.");
        const std::string replacementId = CrimeContactRelationshipSystem::chooseKnownReplacementContact(player, contactId);
        if (!replacementId.empty())
        {
            const PlayerCrimeContactState* replacement = player.findCrimeContactState(replacementId);
            if (replacement != nullptr)
                result.lines.push_back("Un relais déjà connu du même réseau reste disponible : " + replacement->displayName + ".");
        }
        return result;
    }
    if (state->detainedUntilDay > player.getWorldDaysElapsed())
    {
        result.lines.push_back(result.contactName + " est encore en détention provisoire. Ce n'est pas encore une condamnation définitive.");
        const std::string replacementId = CrimeContactRelationshipSystem::chooseKnownReplacementContact(player, contactId);
        if (!replacementId.empty())
        {
            const PlayerCrimeContactState* replacement = player.findCrimeContactState(replacementId);
            if (replacement != nullptr)
                result.lines.push_back("Un relais déjà connu du même réseau peut temporairement prendre le relais : " + replacement->displayName + ".");
        }
        return result;
    }
    if (state->fledUntilDay > player.getWorldDaysElapsed())
    {
        result.lines.push_back(result.contactName + " a quitté le secteur pour le moment. Les échanges directs sont coupés jusqu'à son retour probable.");
        const std::string replacementId = CrimeContactRelationshipSystem::chooseKnownReplacementContact(player, contactId);
        if (!replacementId.empty())
        {
            const PlayerCrimeContactState* replacement = player.findCrimeContactState(replacementId);
            if (replacement != nullptr)
                result.lines.push_back("Un relais déjà connu du même réseau reste joignable : " + replacement->displayName + ".");
        }
        return result;
    }
    if (relation.ruptured)
    {
        result.lines.push_back(result.contactName + " refuse encore les échanges directs après la rupture précédente.");
        result.lines.push_back("La relation pourra être réévaluée après le jour " + std::to_string(state->ruptureUntilDay) + ", sans effacer ce qui s'est passé.");
        const std::string replacementId = CrimeContactRelationshipSystem::chooseKnownReplacementContact(player, contactId);
        if (!replacementId.empty())
        {
            const PlayerCrimeContactState* replacement = player.findCrimeContactState(replacementId);
            if (replacement != nullptr)
                result.lines.push_back("Tu connais déjà un autre relais du même réseau : " + replacement->displayName + ".");
        }
        return result;
    }

    if (relation.rivalry >= 70)
        result.lines.push_back(result.contactName + " ne cache plus vraiment l'hostilité : chaque phrase ressemble à une négociation sous menace.");
    else if (relation.rivalry >= 35)
        result.lines.push_back(result.contactName + " reste tendu et choisit ses mots comme s'il s'attendait à un nouveau conflit.");
    else if (relation.cooperation >= 35 && relation.reliability >= 60)
        result.lines.push_back(result.contactName + " te parle sans détour inhabituel : plusieurs échanges propres ont créé une confiance limitée mais réelle.");
    else if (relation.cooperation <= -35)
        result.lines.push_back(result.contactName + " répond au strict minimum. La coopération existe encore sur le papier, pas vraiment dans le ton.");
    else
        result.lines.push_back(result.contactName + " garde une distance prudente et attend de voir ce que tu veux réellement.");

    if (relation.favorBalance >= 35)
        result.lines.push_back("Il te doit une faveur importante et le sait très bien.");
    else if (relation.favorBalance >= 10)
        result.lines.push_back("Tu disposes encore d'un petit levier : un service précédent n'a pas été oublié.");
    else if (relation.favorBalance <= -35)
        result.lines.push_back("Ta dette envers lui devient difficile à ignorer ; il la rappelle avant même de parler du reste.");
    else if (relation.favorBalance <= -10)
        result.lines.push_back("Il glisse rapidement que tu lui dois encore quelque chose.");

    if (relation.pressure >= 75)
        result.lines.push_back("La pression autour du réseau est très forte : il surveille la porte, les fenêtres et la durée de la conversation.");
    else if (relation.pressure >= 45)
        result.lines.push_back("Le réseau est sous pression et le contact évite les détails qui pourraient voyager trop loin.");

    if (state->reportedToGuard)
        result.lines.push_back("Il sait qu'au moins un dossier officiel de la garde mentionne déjà son identité.");

    const VaultTheftNetworkContact* definition = VaultTheftNetworkCatalog::findById(contactId);
    if (definition != nullptr && !definition->networkId.empty())
        result.lines.push_back("Réseau connu : " + definition->networkId + ". Cette information vient de tes contacts déjà identifiés, pas d'une carte omnisciente.");

    return result;
}
