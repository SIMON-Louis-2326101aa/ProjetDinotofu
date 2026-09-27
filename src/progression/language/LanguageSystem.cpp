#include "progression/language/LanguageSystem.hpp"
#include "entity/Player.hpp"
#include <algorithm>
#include <unordered_map>

namespace
{
    const std::unordered_map<std::string, std::pair<std::string, int>>& studyItems()
    {
        static const std::unordered_map<std::string, std::pair<std::string, int>> items = {
            {"language_goblin_primer", {"gobelin", 1}}, {"language_goblin_course", {"gobelin", 2}},
            {"language_orc_primer", {"orc", 1}}, {"language_orc_course", {"orc", 2}},
            {"language_infernal_primer", {"infernal", 1}}, {"language_infernal_course", {"infernal", 2}},
            {"language_draconic_primer", {"draconique", 1}}, {"language_draconic_course", {"draconique", 2}},
            {"language_elven_primer", {"elfique", 1}}, {"language_elven_course", {"elfique", 2}},
            {"language_dark_elven_primer", {"elfique_noir", 1}}, {"language_dark_elven_course", {"elfique_noir", 2}},
            {"language_celestial_primer", {"celeste", 1}}, {"language_celestial_course", {"celeste", 2}},
            {"language_fey_primer", {"feerique", 1}}, {"language_fey_course", {"feerique", 2}},
            {"language_kitsune_primer", {"kitsune", 1}}, {"language_kitsune_course", {"kitsune", 2}},
            {"language_dwarven_primer", {"nain", 1}}, {"language_dwarven_course", {"nain", 2}},
            {"language_gnomish_primer", {"gnome", 1}}, {"language_gnomish_course", {"gnome", 2}},
            {"language_halfling_primer", {"halfelin", 1}}, {"language_halfling_course", {"halfelin", 2}},
            {"language_vampiric_primer", {"vampirique", 1}}, {"language_vampiric_course", {"vampirique", 2}},
            {"language_spirit_primer", {"spirituel", 1}}, {"language_spirit_course", {"spirituel", 2}},
            {"language_anomaly_notation", {"anormal", 1}}
        };
        return items;
    }
}

const std::vector<LanguageDefinition>& LanguageSystem::getCatalog()
{
    static const std::vector<LanguageDefinition> catalog = {
        {"commun", "Commun", "Langue véhiculaire des routes, guildes et villes.", false, false},
        {"gobelin", "Gobelin", "Parler rapide, contracté et très contextuel des bandes gobelines.", true, false},
        {"orc", "Orc", "Langue martiale aux consonnes lourdes, riche en notions de rang et de défi.", true, false},
        {"infernal", "Infernal", "Langue des démons, pactes et lignées infernales.", true, false},
        {"draconique", "Draconique", "Langue ancienne des dragons et lignées draconiques.", true, false},
        {"elfique", "Elfique", "Langue fluide des elfes, souvent liée aux noms de lieux anciens.", true, false},
        {"elfique_noir", "Elfique noir", "Branche plus sèche et codée de l'elfique, utilisée par les lignées sombres.", true, false},
        {"celeste", "Céleste", "Langue liturgique des anges, aasimar et certains ordres sacrés.", true, false},
        {"feerique", "Féerique", "Langue de rythme, d'allusions et de promesses des fées.", true, false},
        {"kitsune", "Kitsune", "Langue imagée des lignées kitsune, riche en doubles sens.", true, false},
        {"nain", "Nain", "Langue des clans nains, précise pour la pierre, le métal et les contrats.", true, false},
        {"gnome", "Gnome", "Langue vive des gnomes, remplie d'abréviations techniques.", true, false},
        {"halfelin", "Halfelin", "Langue domestique et commerciale des communautés halfelines.", true, false},
        {"vampirique", "Vampirique", "Langue de cour et de lignage utilisée par certaines familles vampiriques.", true, false},
        {"spirituel", "Spirituel", "Langage de motifs, souvenirs et intentions des esprits conscients.", true, false},
        {"anormal", "Notation anormale", "Signes impossibles et fragments sonores des anomalies. On peut en reconnaître des motifs, jamais la parler proprement.", true, true}
    };
    return catalog;
}

const LanguageDefinition* LanguageSystem::find(const std::string& languageId)
{
    for (const LanguageDefinition& language : getCatalog())
    {
        if (language.id == languageId) return &language;
    }
    return nullptr;
}

std::string LanguageSystem::displayName(const std::string& languageId)
{
    const LanguageDefinition* language = find(languageId);
    return language ? language->name : languageId;
}

std::string LanguageSystem::languageForMonsterRace(Race race)
{
    switch (race)
    {
        case Race::Gobelin: case Race::Hobgobelin: return "gobelin";
        case Race::Orc: return "orc";
        case Race::Demon: case Race::Tieffelin: return "infernal";
        case Race::Dragon: case Race::Draconide: case Race::SemiDragon: return "draconique";
        case Race::Elfe: return "elfique";
        case Race::ElfeNoir: return "elfique_noir";
        case Race::Fee: return "feerique";
        case Race::Kitsune: return "kitsune";
        case Race::Ange: case Race::Aasimar: return "celeste";
        case Race::Nain: return "nain";
        case Race::Gnome: return "gnome";
        case Race::Halfelin: return "halfelin";
        case Race::Esprit: return "spirituel";
        case Race::Aberration: case Race::AnomalieArcanique: return "anormal";
        default: return "commun";
    }
}

std::string LanguageSystem::nativeLanguageForPlayerRace(CharacterRace race)
{
    switch (race)
    {
        case CharacterRace::DarkElf: return "elfique_noir";
        case CharacterRace::Elf: return "elfique";
        case CharacterRace::Dwarf: return "nain";
        case CharacterRace::Gnome: return "gnome";
        case CharacterRace::Halfling: return "halfelin";
        case CharacterRace::Tiefling: case CharacterRace::Demon: return "infernal";
        case CharacterRace::Aasimar: return "celeste";
        case CharacterRace::Kitsune: return "kitsune";
        case CharacterRace::Fairy: return "feerique";
        case CharacterRace::HalfDragon: return "draconique";
        case CharacterRace::Orc: return "orc";
        case CharacterRace::Vampire: return "vampirique";
        default: return "commun";
    }
}

void LanguageSystem::ensureStarterLanguages(Player& player)
{
    player.setLanguageKnowledgeLevel("commun", 3, true);
    const std::string native = nativeLanguageForPlayerRace(player.getRace());
    player.setLanguageKnowledgeLevel(native, 3, true);
}

int LanguageSystem::getKnowledgeLevel(const Player& player, const std::string& languageId)
{
    return player.getLanguageKnowledgeLevel(languageId);
}

bool LanguageSystem::understands(const Player& player, const std::string& languageId, int requiredLevel)
{
    return getKnowledgeLevel(player, languageId) >= requiredLevel;
}

std::string LanguageSystem::knowledgeLabel(int level)
{
    if (level >= 3) return "courant / natif";
    if (level == 2) return "conversation";
    if (level == 1) return "notions";
    return "inconnue";
}

std::string LanguageSystem::studyHint(const std::string& languageId, int level)
{
    if (languageId == "commun") return "Le commun est maîtrisé dès le départ dans la version actuelle.";
    if (level >= 3) return "Langue maîtrisée naturellement ou à un niveau équivalent.";
    if (level == 2) return "Tu comprends une conversation et la majorité des quêtes, mais pas toutes les nuances littéraires.";
    if (level == 1) return "Tu reconnais des mots, des avertissements simples et quelques objectifs. Un cours avancé est encore nécessaire.";
    return "Une bibliothèque peut fournir un manuel d'initiation si cette langue est documentée.";
}

std::string LanguageSystem::languageIdForStudyItem(const std::string& itemId)
{
    auto it = studyItems().find(itemId);
    return it == studyItems().end() ? "" : it->second.first;
}

int LanguageSystem::studyTargetLevelForItem(const std::string& itemId)
{
    auto it = studyItems().find(itemId);
    return it == studyItems().end() ? 0 : it->second.second;
}

bool LanguageSystem::applyStudyItem(Player& player, const std::string& itemId, std::vector<std::string>* notes)
{
    const std::string languageId = languageIdForStudyItem(itemId);
    const int target = studyTargetLevelForItem(itemId);
    if (languageId.empty() || target <= 0) return false;

    const int before = player.getLanguageKnowledgeLevel(languageId);
    if (target <= before)
    {
        if (notes) notes->push_back("Le manuel confirme surtout des notions déjà maîtrisées en " + displayName(languageId) + ".");
        return true;
    }

    player.setLanguageKnowledgeLevel(languageId, target, false);
    player.recordHistoricalEvent(
        "apprentissage_langue",
        languageId,
        "Étude de " + displayName(languageId) + " : " + knowledgeLabel(target),
        false
    );
    if (notes)
    {
        notes->push_back("Langue étudiée : " + displayName(languageId) + " — niveau " + knowledgeLabel(target) + ".");
        if (languageId == "anormal") notes->push_back("Même étudiée, la notation anormale reste fragmentaire : tu reconnais des motifs, pas une grammaire stable.");
    }
    return true;
}

bool LanguageSystem::canPracticeTowardFluency(const Player& player, const std::string& languageId)
{
    if (languageId.empty() || languageId == "commun" || languageId == "anormal") return false;
    const LanguageDefinition* language = find(languageId);
    if (!language || !language->learnableAtLibrary || language->unstable) return false;
    return player.getLanguageKnowledgeLevel(languageId) == 2;
}

bool LanguageSystem::applyGuidedPractice(Player& player, const std::string& languageId, int practiceQuality, std::vector<std::string>* notes)
{
    if (!canPracticeTowardFluency(player, languageId))
    {
        if (notes)
        {
            const int level = player.getLanguageKnowledgeLevel(languageId);
            if (languageId == "anormal") notes->push_back("La notation anormale ne possède pas de niveau courant stable : ses motifs restent fragmentaires.");
            else if (level < 2) notes->push_back("Il faut d'abord atteindre un niveau de conversation avant le travail de fluidité.");
            else notes->push_back("Cette langue n'a plus besoin de séance guidée pour le moment.");
        }
        return false;
    }

    practiceQuality = std::max(1, std::min(practiceQuality, 3));
    const int before = player.getLanguageStudyProgress(languageId);
    const int gain = 18 + practiceQuality * 7;
    player.addLanguageStudyProgress(languageId, gain);
    const int after = player.getLanguageStudyProgress(languageId);
    const bool mastered = player.getLanguageKnowledgeLevel(languageId) >= 3;

    player.recordHistoricalEvent(
        mastered ? "maitrise_langue" : "pratique_langue",
        languageId,
        mastered
            ? "Maîtrise courante obtenue en " + displayName(languageId) + " après pratique guidée"
            : "Pratique guidée de " + displayName(languageId) + " : " + std::to_string(after) + "% vers la fluidité",
        false
    );

    if (notes)
    {
        notes->push_back("Séance guidée : " + displayName(languageId) + ".");
        notes->push_back("Progression de fluidité : " + std::to_string(before) + "% -> " + std::to_string(after) + "%.");
        if (mastered) notes->push_back("Niveau atteint : courant. Les nuances, tournures rapides et textes exigeants deviennent accessibles.");
        else notes->push_back("Il faudra encore pratiquer : le niveau 3 n'est pas acheté instantanément.");
    }
    return true;
}

std::string LanguageSystem::renderSpeechForKnowledge(
    const Player& player,
    const std::string& languageId,
    const std::string& foreignText,
    const std::string& translatedText
)
{
    const int level = getKnowledgeLevel(player, languageId);
    if (languageId == "commun" || level >= 2) return translatedText;
    if (level == 1)
    {
        std::string clue = translatedText;
        if (clue.size() > 58) clue = clue.substr(0, 55) + "...";
        return foreignText + "  [Tu saisis quelques mots : « " + clue + " »]";
    }
    return foreignText;
}
