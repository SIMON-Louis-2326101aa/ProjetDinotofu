#include "world/npc/LivingNpcProfile.hpp"
#include <algorithm>
#include <cctype>

namespace {
std::string low(std::string v) { std::transform(v.begin(), v.end(), v.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); }); return v; }
bool has(const std::string& v, const std::string& token) { return v.find(token) != std::string::npos; }
}

LivingNpcProfile LivingNpcProfileSystem::infer(const std::string& name, const std::string& role, const std::string& raceName)
{
    LivingNpcProfile p;
    const std::string text = low(name + " " + role);
    const std::string race = low(raceName);
    if (has(text, "garde") || has(text, "capitaine")) { p.profession = "garde"; p.temperament = "discipliné"; p.informationNetwork = "reseau_garde"; }
    else if (has(text, "marchand") || has(text, "vendeur")) { p.profession = "marchand"; p.temperament = "calculateur"; p.informationNetwork = "reseau_commercial"; }
    else if (has(text, "bibli") || has(text, "scribe")) { p.profession = "érudit"; p.temperament = "curieux"; p.informationNetwork = "reseau_savant"; }
    else if (has(text, "forger")) { p.profession = "artisan"; p.temperament = "franc"; p.informationNetwork = "reseau_artisans"; }
    else if (has(text, "auberg") || has(text, "tavern")) { p.profession = "aubergiste"; p.temperament = "sociable"; p.informationNetwork = "reseau_auberges"; }
    else if (has(text, "guilde") || has(text, "maître") || has(text, "maitre")) { p.profession = "guilde"; p.temperament = "pragmatique"; p.informationNetwork = "reseau_guilde"; }
    else if (has(text, "contact") || has(text, "informateur")) { p.profession = "contact"; p.temperament = "observateur"; p.informationNetwork = "reseau_contacts"; }
    else if (has(text, "prêtre") || has(text, "pretresse") || has(text, "prêtresse") || has(text, "église") || has(text, "eglise")) { p.profession = "religieux"; p.temperament = "posé"; p.informationNetwork = "reseau_temple"; }

    if (has(race, "démon") || has(race, "tieff")) p.nativeLanguage = "infernal";
    else if (has(race, "gobelin")) p.nativeLanguage = "gobelin";
    else if (has(race, "orc")) p.nativeLanguage = "orc";
    else if (has(race, "kitsune")) p.nativeLanguage = "kitsune";
    else if (has(race, "elfe noir")) p.nativeLanguage = "elfique_noir";
    else if (has(race, "elfe")) p.nativeLanguage = "elfique";
    else if (has(race, "nain")) p.nativeLanguage = "nain";
    else if (has(race, "gnome")) p.nativeLanguage = "gnome";
    else if (has(race, "halfelin")) p.nativeLanguage = "halfelin";
    else if (has(race, "fée") || has(race, "fee")) p.nativeLanguage = "feerique";
    else if (has(race, "dragon")) p.nativeLanguage = "draconique";
    else if (has(race, "ange") || has(race, "aasimar")) p.nativeLanguage = "celeste";
    return p;
}

std::string LivingNpcProfileSystem::reactionToKnownFact(const LivingNpcProfile& p, const std::string& factType, const std::string& subject)
{
    if (factType == "rival_seen") {
        if (p.profession == "garde") return "Le garde demande où " + subject + " a été vu et qui peut confirmer la trace.";
        if (p.profession == "marchand") return "Le marchand baisse la voix : si " + subject + " rôde vraiment, il ne laissera pas ses caisses dehors ce soir.";
        if (p.temperament == "curieux") return "La nouvelle de " + subject + " l'intéresse, mais il demande d'abord si tu as une preuve ou seulement une rumeur.";
        return "À l'évocation de " + subject + ", la réaction reste prudente : personne ici ne veut transformer une rumeur en certitude.";
    }
    if (factType == "debt_paid") return p.profession == "marchand" ? "Le compte est soldé. Le ton du marchand se détend immédiatement." : "La dette réglée circule comme un signe de parole tenue.";
    if (factType == "local_attack") return p.profession == "garde" ? "Le garde compare ton récit aux témoignages déjà recueillis." : "L'attaque a laissé des traces visibles ; la conversation revient vite sur les dégâts et les absents.";
    if (factType == "quest_declined") {
        if (p.profession == "marchand" || p.profession == "marchand itinérant") return "Le marchand se souvient surtout que tu n'as pas pris l'affaire cette fois-ci ; il ne confond pas un refus clair avec une promesse rompue.";
        if (p.profession == "garde") return "Le garde préfère un refus net à un engagement abandonné en route ; il cherchera simplement quelqu'un d'autre pour cette affaire.";
        if (p.profession == "guilde" || p.profession == "intendance") return "Le refus est noté comme un choix de disponibilité, pas comme un échec. La fiche peut être confiée à quelqu'un d'autre.";
        return "Ce PNJ se souvient que tu avais décliné cette demande, sans en faire automatiquement un grief.";
    }
    if (factType == "quest_accepted") {
        if (p.profession == "garde") return "Le garde retient que tu as accepté cette affaire, mais ne considère pas encore le problème comme résolu.";
        if (p.profession == "marchand") return "Le marchand note surtout qu'un engagement a été pris ; il attend de voir s'il sera tenu.";
        return "Ce PNJ se souvient t'avoir confié ou vu accepter cette affaire ; pour lui, la promesse compte encore.";
    }
    if (factType == "quest_failed") {
        if (p.profession == "garde") return "Le garde n'efface pas l'affaire du registre : il veut surtout savoir si le danger existe encore et s'il faut réassigner la tâche.";
        if (p.profession == "marchand" || p.profession == "marchand itinérant") return "Le marchand se souvient que l'engagement n'a pas été tenu à temps ; il reste poli, mais devient plus prudent avant de confier une urgence.";
        if (p.profession == "guilde") return "La guilde classe l'échec comme un fait, pas comme une condamnation : les prochains contrats urgents seront simplement observés avec plus d'attention.";
        if (p.temperament == "curieux" || p.temperament == "méthodique") return "Le résultat manqué compte, mais les raisons l'intéressent aussi : retard, fausse piste ou difficulté imprévue ne racontent pas la même histoire.";
        return "Ce PNJ se souvient que cette affaire n'a pas été menée à temps ; son ton devient un peu plus réservé sans transformer un échec en hostilité automatique.";
    }
    if (factType == "quest_completed") {
        if (p.profession == "garde") return "Le garde associe désormais ton nom à une affaire réellement terminée, pas seulement à une promesse.";
        if (p.profession == "marchand") return "Le résultat concret pèse davantage qu'une rumeur : le marchand devient sensiblement plus réceptif.";
        if (p.temperament == "curieux" || p.temperament == "méthodique") return "Le résultat l'intéresse autant que la méthode ; il garde surtout les détails qu'il peut recouper.";
        return "Ce PNJ se souvient d'un service mené jusqu'au bout et ajuste son ton en conséquence.";
    }
    if (factType == "discovered_writing") {
        if (p.profession == "érudit") return "L'érudit demande où l'écrit a été trouvé, dans quelle langue et si l'original ou une copie peut être examiné.";
        if (p.profession == "garde") return "Le garde écoute, mais distingue clairement un texte découvert d'un ordre officiel authentifié.";
        return "L'écrit attire l'attention, sans devenir automatiquement une vérité tant que personne ici ne l'a vérifié.";
    }
    if (factType == "unusual_monster") {
        if (p.profession == "garde") return "Le garde veut une description exploitable : taille, direction, blessures visibles et témoins éventuels.";
        if (p.profession == "érudit") return "L'érudit compare mentalement la description aux espèces connues et demande ce qui rendait vraiment la créature inhabituelle.";
        if (p.profession == "marchand") return "Le marchand pense d'abord aux routes que cette créature pourrait rendre moins sûres.";
        return "La créature inhabituelle nourrit la conversation, mais le récit reste séparé des observations confirmées.";
    }
    return "La réaction dépend surtout de ce que ce PNJ a réellement vu, entendu ou pu vérifier.";
}
