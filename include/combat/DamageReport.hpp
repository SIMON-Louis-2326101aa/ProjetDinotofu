// EN: DamageReport.hpp briefly defines this Dinotofu module and its responsibilities.
// FR: DamageReport.hpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#ifndef INCLUDE_COMBAT_DAMAGEREPORT_HPP
#define INCLUDE_COMBAT_DAMAGEREPORT_HPP

struct DamageReport
{
    int rawDamage;
    int armorAbsorbedDamage;
    int classReducedDamage;
    int receivedDamage;
    bool armorUsed;
    bool armorBrokenDuringImpact;
    bool armorBlockedByBossSeal;

    DamageReport()
        : rawDamage(0),
          armorAbsorbedDamage(0),
          classReducedDamage(0),
          receivedDamage(0),
          armorUsed(false),
          armorBrokenDuringImpact(false),
          armorBlockedByBossSeal(false)
    {
    }
};

#endif
