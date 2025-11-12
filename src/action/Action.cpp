#include <random>
#include "action/Action.hpp"

static int randInclusive(int lo,int hi){ static std::random_device rd; static std::mt19937 g(rd()); std::uniform_int_distribution<int>d(lo,hi); return d(g); }

int rollD20(){ return randInclusive(1,20); }

IssueAttaque evaluerIssue(int jet){
    if (jet<=3) return IssueAttaque::Miss;
    if (jet<=16) return IssueAttaque::Hit;
    return IssueAttaque::Crit;
}

int calculDegats(const Entite& a, IssueAttaque issue, int bonusPlat){
    if (issue==IssueAttaque::Miss) return 0;
    int base = (issue==IssueAttaque::Crit) ? a.getDgtCrit() : randInclusive(a.getDgtMin(), a.getDgtMax());
    long long t = (long long)base + bonusPlat;
    if (t < 0) {
    t = 0;
    }
    if (t > INT32_MAX) {
        t = INT32_MAX;
    }
    return (int)t;
}

ResultatAttaque executerAttaque(const Entite& att, Entite& def, int bonusPlat){
    IssueAttaque iss = evaluerIssue(rollD20());
    int dmg = calculDegats(att, iss, bonusPlat);
    if (dmg>0) def.prendreDegats(dmg);
    return {iss, dmg};
}
