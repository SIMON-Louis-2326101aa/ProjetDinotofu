#include "quest/Quest.hpp"
#include "quest/language/QuestLanguageSystem.hpp"

#include <cassert>
#include <string>

int main()
{
    Quest quest;
    quest.guildQuest = true;
    quest.title = "Observer un groupe de gobelins";
    quest.objective = "Lire les marques gobelines puis confirmer leur trajet.";
    quest.targetFamily = "Gobelins";
    quest.location = "Route commerciale";

    bool found = false;
    for (int i = 0; i < 500; ++i)
    {
        Quest candidate = quest;
        candidate.id = "test_goblin_" + std::to_string(i);
        QuestLanguageSystem::assignOptionalForeignLanguage(candidate);
        if (!candidate.requiredLanguage.empty())
        {
            assert(candidate.requiredLanguage == "gobelin");
            assert(candidate.requiredLanguageLevel == 2);
            assert(!candidate.sourceLanguageText.empty());
            found = true;
            break;
        }
    }
    assert(found);

    Quest storyLike;
    storyLike.guildQuest = false;
    storyLike.id = "main_story_test";
    storyLike.title = "Parler à un démon";
    storyLike.objective = "Continuer l'histoire";
    QuestLanguageSystem::assignOptionalForeignLanguage(storyLike);
    assert(storyLike.requiredLanguage.empty());
    return 0;
}
