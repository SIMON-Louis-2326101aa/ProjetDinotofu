// EN: RuntimeLog.hpp provides a light diagnostic log for beta feedback.
// FR: RuntimeLog.hpp fournit un journal diagnostic léger pour les retours bêta.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_DIAGNOSTIC_RUNTIMELOG_HPP
#define INCLUDE_DIAGNOSTIC_RUNTIMELOG_HPP

#include <string>
#include <vector>

class RuntimeLog
{
public:
    static void recordScreen(
        const std::string& title,
        const std::string& screenId,
        const std::vector<std::string>& lines
    );

    static std::string currentLogPath();
};

#endif
