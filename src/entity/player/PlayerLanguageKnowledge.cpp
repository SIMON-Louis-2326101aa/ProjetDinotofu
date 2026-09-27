#include "entity/Player.hpp"
#include "progression/language/LanguageSystem.hpp"
#include <algorithm>

const std::vector<PlayerLanguageKnowledge>& Player::getLanguageKnowledge() const
{
    return languageKnowledge;
}

int Player::getLanguageKnowledgeLevel(const std::string& languageId) const
{
    for (const PlayerLanguageKnowledge& knowledge : languageKnowledge)
    {
        if (knowledge.languageId == languageId) return knowledge.level;
    }
    return 0;
}

int Player::getLanguageStudyProgress(const std::string& languageId) const
{
    for (const PlayerLanguageKnowledge& knowledge : languageKnowledge)
    {
        if (knowledge.languageId == languageId) return knowledge.studyProgress;
    }
    return 0;
}

void Player::setLanguageKnowledgeLevel(const std::string& languageId, int level, bool nativeLanguage)
{
    if (languageId.empty()) return;
    level = std::max(0, std::min(level, 3));
    for (PlayerLanguageKnowledge& knowledge : languageKnowledge)
    {
        if (knowledge.languageId == languageId)
        {
            knowledge.level = std::max(knowledge.level, level);
            knowledge.nativeLanguage = knowledge.nativeLanguage || nativeLanguage;
            if (knowledge.level >= 3) knowledge.studyProgress = 100;
            return;
        }
    }
    PlayerLanguageKnowledge knowledge;
    knowledge.languageId = languageId;
    knowledge.level = level;
    knowledge.studyProgress = level >= 3 ? 100 : 0;
    knowledge.nativeLanguage = nativeLanguage;
    languageKnowledge.push_back(knowledge);
}

void Player::setLanguageStudyProgress(const std::string& languageId, int progress)
{
    if (languageId.empty()) return;
    progress = std::max(0, std::min(progress, 100));
    for (PlayerLanguageKnowledge& knowledge : languageKnowledge)
    {
        if (knowledge.languageId != languageId) continue;
        knowledge.studyProgress = progress;
        if (knowledge.studyProgress >= 100 && knowledge.level >= 2 && languageId != "anormal")
        {
            knowledge.level = 3;
        }
        return;
    }
    PlayerLanguageKnowledge knowledge;
    knowledge.languageId = languageId;
    knowledge.level = 0;
    knowledge.studyProgress = progress;
    knowledge.nativeLanguage = false;
    languageKnowledge.push_back(knowledge);
}

void Player::addLanguageStudyProgress(const std::string& languageId, int amount)
{
    if (amount <= 0) return;
    setLanguageStudyProgress(languageId, getLanguageStudyProgress(languageId) + amount);
}

void Player::setLoadedLanguageKnowledge(const std::vector<PlayerLanguageKnowledge>& loadedKnowledge)
{
    languageKnowledge.clear();
    for (PlayerLanguageKnowledge entry : loadedKnowledge)
    {
        if (entry.languageId.empty()) continue;
        entry.level = std::max(0, std::min(entry.level, 3));
        entry.studyProgress = std::max(0, std::min(entry.studyProgress, 100));
        if (entry.level >= 3) entry.studyProgress = 100;
        languageKnowledge.push_back(entry);
    }
    LanguageSystem::ensureStarterLanguages(*this);
}
