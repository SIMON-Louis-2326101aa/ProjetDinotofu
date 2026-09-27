#ifndef INCLUDE_PROGRESSION_LANGUAGE_LANGUAGEKNOWLEDGE_HPP
#define INCLUDE_PROGRESSION_LANGUAGE_LANGUAGEKNOWLEDGE_HPP

#include <string>

struct PlayerLanguageKnowledge
{
    std::string languageId;
    int level = 0; // 0 inconnu, 1 notions, 2 conversation, 3 courant/natif
    int studyProgress = 0;
    bool nativeLanguage = false;
};

#endif
