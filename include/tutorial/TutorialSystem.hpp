// EN: TutorialSystem.hpp defines the isolated, replayable onboarding route used from character selection.
// FR: TutorialSystem.hpp définit le parcours d'apprentissage isolé et rejouable depuis la sélection des personnages.

#ifndef INCLUDE_TUTORIAL_TUTORIALSYSTEM_HPP
#define INCLUDE_TUTORIAL_TUTORIALSYSTEM_HPP

#include <string>

class TutorialSystem
{
public:
    static const std::string& dummyName();
    static const std::string& startedTitle();
    static const std::string& completedTitle();

    // EN: Runs an isolated tutorial that never saves the dummy character or modifies world progression.
    // FR: Lance un tutoriel isolé qui ne sauvegarde jamais le cobaye et ne modifie pas la progression du monde.
    static bool run(const std::string& accountName);
};

#endif
