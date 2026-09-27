#include "quest/language/QuestLanguageSystem.hpp"

#include "entity/Player.hpp"
#include "progression/language/LanguageSystem.hpp"
#include "quest/Quest.hpp"

#include <algorithm>
#include <cctype>
#include <functional>
#include <string>

namespace
{
    std::string lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    std::string thematicLanguage(const Quest& quest)
    {
        const std::string text = lower(quest.title + " " + quest.objective + " " + quest.targetFamily + " " + quest.location);
        if (text.find("démon") != std::string::npos || text.find("infer") != std::string::npos) return "infernal";
        if (text.find("gobelin") != std::string::npos) return "gobelin";
        if (text.find("orc") != std::string::npos) return "orc";
        if (text.find("dragon") != std::string::npos || text.find("drake") != std::string::npos) return "draconique";
        if (text.find("fée") != std::string::npos || text.find("fee") != std::string::npos || text.find("mana") != std::string::npos) return "feerique";
        if (text.find("vamp") != std::string::npos) return "vampirique";
        if (text.find("spect") != std::string::npos || text.find("esprit") != std::string::npos || text.find("mort-vivant") != std::string::npos) return "spirituel";
        if (text.find("elfe noir") != std::string::npos) return "elfique_noir";
        if (text.find("elf") != std::string::npos) return "elfique";
        if (text.find("nain") != std::string::npos || text.find("mine") != std::string::npos) return "nain";
        if (text.find("kitsun") != std::string::npos || text.find("renard") != std::string::npos) return "kitsune";
        return "";
    }

    std::string sampleText(const std::string& languageId)
    {
        if (languageId == "gobelin") return "Grakka tik, vor nakka. Trel mokh!";
        if (languageId == "orc") return "Gor makh dur. Thrak var osh.";
        if (languageId == "infernal") return "Zhar vel'kor, neth arax. Vesh tal.";
        if (languageId == "draconique") return "Saar ven drakhal. Ith karesh vor.";
        if (languageId == "elfique") return "Lethiel vael enor, sil mae thir.";
        if (languageId == "elfique_noir") return "Vel shae nyr, khael dor ven.";
        if (languageId == "feerique") return "Lumi vae, tiri sel, niel an.";
        if (languageId == "vampirique") return "Vhal sereth, nox vel arin.";
        if (languageId == "spirituel") return "Ahn... vel... saïr... memora.";
        if (languageId == "nain") return "Khar dum vek, barag tor.";
        if (languageId == "kitsune") return "Kiyo na, hoshi rei, tsu ven.";
        return "[glyphes étrangers difficiles à identifier]";
    }
}

void QuestLanguageSystem::assignOptionalForeignLanguage(Quest& quest)
{
    if (!quest.guildQuest || !quest.requiredLanguage.empty())
    {
        return;
    }

    const std::string language = thematicLanguage(quest);
    if (language.empty())
    {
        return;
    }

    const std::size_t roll = std::hash<std::string>{}(quest.id + "|" + quest.title + "|" + quest.objective) % 100U;
    if (roll >= 24U)
    {
        return;
    }

    quest.requiredLanguage = language;
    quest.requiredLanguageLevel = 2;
    quest.sourceLanguageText = sampleText(language);
}

bool QuestLanguageSystem::canRead(const Player& player, const Quest& quest)
{
    return quest.requiredLanguage.empty()
        || LanguageSystem::getKnowledgeLevel(player, quest.requiredLanguage) >= std::max(1, quest.requiredLanguageLevel);
}

std::string QuestLanguageSystem::requirementLine(const Player& player, const Quest& quest)
{
    if (quest.requiredLanguage.empty())
    {
        return "Langue : commun.";
    }

    const int known = LanguageSystem::getKnowledgeLevel(player, quest.requiredLanguage);
    const std::string language = LanguageSystem::displayName(quest.requiredLanguage);
    if (canRead(player, quest))
    {
        return "Langue annexe : " + language + " — comprise (" + LanguageSystem::knowledgeLabel(known) + ").";
    }

    return "Langue annexe : " + language + " — compréhension insuffisante (" + LanguageSystem::knowledgeLabel(known)
        + "). La bibliothèque peut fournir un apprentissage adapté.";
}

std::string QuestLanguageSystem::readableObjective(const Player& player, const Quest& quest)
{
    if (canRead(player, quest))
    {
        return quest.objective;
    }

    if (LanguageSystem::getKnowledgeLevel(player, quest.requiredLanguage) == 1)
    {
        return (quest.sourceLanguageText.empty() ? std::string("[texte étranger]") : quest.sourceLanguageText)
            + " — tu reconnais quelques termes, mais pas assez pour accepter le contrat sans risquer un contresens.";
    }

    return quest.sourceLanguageText.empty() ? std::string("[texte étranger non compris]") : quest.sourceLanguageText;
}
