#ifndef ACTION_HPP
#define ACTION_HPP

#include <cstdint>
#include "entite/Entite.hpp"

enum class IssueAttaque : uint8_t { Miss=0, Hit=1, Crit=2 };

struct ResultatAttaque {
    IssueAttaque issue;
    int degatsInfliges;
};

int rollD20();                 // 1..20
IssueAttaque evaluerIssue(int jetD20);
int calculDegats(const Entite& attaquant, IssueAttaque issue, int bonusPlat=0);
ResultatAttaque executerAttaque(const Entite& attaquant, Entite& defenseur, int bonusPlat=0);

#endif
