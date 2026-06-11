// EN: BossCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: BossCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "boss/BossCatalog.hpp"

#include "interface/menu/common/MessageScreen.hpp"

#include <vector>
#include <string>
#include <array>

namespace
{

    int getRecommendedLevelInternal(int id)
    {
        switch (id)
        {
            case 1: return 15;
            case 2: return 18;
            case 3: return 22;
            case 4: return 26;
            case 5: return 30;
            case 6: return 34;
            case 7: return 38;
            case 8: return 42;
            case 9: return 46;
            case 10: return 50;
            case 11: return 55;
            case 12: return 60;
            case 13: return 65;
            case 14: return 70;
            case 15: return 75;
            case 16: return 80;
            case 17: return 85;
            case 18: return 90;
            case 19: return 95;
            case 20: return 100;
            case 21: return 108;
            case 22: return 116;
            case 23: return 124;
            case 24: return 132;
            case 25: return 140;
            case 26: return 150;
            case 27: return 255;
            case 28: return 158;
            case 29: return 166;
            case 30: return 176;
            case 31: return 184;
            case 32: return 192;
            case 33: return 200;
            case 34: return 208;
            case 35: return 220;
            case 36: return 248;
            default: return 15;
        }
    }

    std::string getRecommendedLevelTextInternal(int id)
    {
        return "[niv. conseillé " + std::to_string(getRecommendedLevelInternal(id)) + "]";
    }

    BossHierarchyInfo getHierarchyInfoInternal(int id)
    {
        const int level = getRecommendedLevelInternal(id);
        if (level <= 30)
        {
            return {1, "Palier I — Menaces fondatrices", "Boss locaux et premières entités majeures", 15, 30};
        }
        if (level <= 60)
        {
            return {2, "Palier II — Menaces régionales", "Rois locaux, esprits, reflets et anomalies stabilisées", 31, 60};
        }
        if (level <= 100)
        {
            return {3, "Palier III — Avatars et fragments", "Manifestations divines limitées et forces conceptuelles", 61, 100};
        }
        if (level <= 150)
        {
            return {4, "Palier IV — Gardiens majeurs", "Verrous de progression, souverains oubliés et entités primordiales affaiblies", 101, 150};
        }
        if (level <= 220)
        {
            return {5, "Palier V — Souverains et horreurs", "Rois, reines, manifestations du destin et créatures presque légendaires", 151, 220};
        }
        return {6, "Palier VI — Sources terminales", "Sources presque finales et avatar ultime actuellement connu", 221, 255};
    }

    std::string getRegistryDisplayNameInternal(int id)
    {
        if (id == 1) return "Fitoria";
        if (id == 2) return "Zelef";
        if (id == 3) return "Atlas";
        return "???";
    }

    std::string getRegistryHintInternal(int id)
    {
        if (id == 1) return "Entité de base déjà identifiée : ange lumineux. Le reste reste brouillé.";
        if (id == 2) return "Entité de base déjà identifiée : démon brutal. Le reste reste brouillé.";
        if (id == 3) return "Entité de base déjà identifiée : protecteur universel déchu. Le reste reste brouillé.";
        return "Une variation d'énergie anormale a été détectée, mais son identité refuse encore le registre.";
    }

    void appendBossLine(std::vector<std::string>& lines, int id)
    {
        lines.push_back(std::to_string(id) + " : " + getRegistryDisplayNameInternal(id) + " " + getRecommendedLevelTextInternal(id));
        lines.push_back("    " + getRegistryHintInternal(id));
        lines.push_back("    Niveau recommandé non bloquant : " + std::to_string(getRecommendedLevelInternal(id)) + ". Tu peux tenter avant, mais à tes risques et périls.");
        lines.push_back("");
    }
}

// EN: displayAvailableBosses declares or implements a focused behavior used by this module.
// FR: displayAvailableBosses déclare ou implémente un comportement précis utilisé par ce module.
std::vector<std::string> BossCatalog::getAvailableBossLines()
{
    return getAvailableBossLines({1, 2, 3});
}

std::vector<std::string> BossCatalog::getAvailableBossLines(const std::vector<int>& bossIds)
{
    std::vector<std::string> lines;
    for (int id : bossIds)
    {
        appendBossLine(lines, id);
    }
    return lines;
}

void BossCatalog::displayAvailableBosses()
{
    MessageScreen::show("BOSS DISPONIBLES", "catalog.boss.available_basic", getAvailableBossLines(), false);
}

// EN: displayAvailableBosses declares or implements a focused behavior used by this module.
// FR: displayAvailableBosses déclare ou implémente un comportement précis utilisé par ce module.
void BossCatalog::displayAvailableBosses(const std::vector<int>& bossIds)
{
    MessageScreen::show("BOSS DISPONIBLES", "catalog.boss.available", getAvailableBossLines(bossIds), false);
}

std::string BossCatalog::getRegistryDisplayName(int bossId)
{
    return getRegistryDisplayNameInternal(bossId);
}

std::string BossCatalog::getRegistryHint(int bossId)
{
    return getRegistryHintInternal(bossId);
}

int BossCatalog::getRecommendedLevel(int bossId)
{
    return getRecommendedLevelInternal(bossId);
}

std::string BossCatalog::getRecommendedLevelText(int bossId)
{
    return getRecommendedLevelTextInternal(bossId);
}

BossHierarchyInfo BossCatalog::getHierarchyInfo(int bossId)
{
    return getHierarchyInfoInternal(bossId);
}

std::string BossCatalog::getHierarchyLabel(int bossId)
{
    const BossHierarchyInfo info = getHierarchyInfoInternal(bossId);
    return info.title;
}


// EN: getMaximumBossId declares or implements a focused behavior used by this module.
// FR: getMaximumBossId déclare ou implémente un comportement précis utilisé par ce module.
int BossCatalog::getMaximumBossId()
{
    return 36;
}

// EN: createBoss declares or implements a focused behavior used by this module.
// FR: createBoss déclare ou implémente un comportement précis utilisé par ce module.
Boss BossCatalog::createBoss(int choice)
{
    if (choice == 1)
    {
        return Boss(1, "Fitoria", "Ange", 980, 18, 32, 48, 3, 5, 3, 14);
    }

    if (choice == 2)
    {
        return Boss(2, "Zelef", "Démon", 920, 22, 40, 60, 5, 4, 4, 10);
    }

    if (choice == 3)
    {
        return Boss(3, "Atlas", "Protecteur universel déchu", 1450, 16, 30, 44, 2, 5, 3, 12);
    }

    if (choice == 4)
    {
        return Boss(4, "Écho de Lyknir", "Écho de divinité de la Meute", 980, 9, 19, 30, 2, 2, 3, 5);
    }

    if (choice == 5)
    {
        return Boss(5, "Grinka", "Reine gobeline bénie par l'Avarice", 920, 7, 17, 24, 3, 3, 2, 5);
    }

    if (choice == 6)
    {
        return Boss(6, "Avatar d'Azelanos", "Avatar affaibli du Roi démon", 1350, 8, 18, 28, 2, 3, 3, 6);
    }

    if (choice == 7)
    {
        return Boss(7, "Fragment de Thamarys", "Fragment draconique divin affaibli", 1500, 10, 22, 34, 2, 3, 3, 7);
    }

    if (choice == 8)
    {
        return Boss(8, "Mojo", "Esprit mythique de la Forêt", 1050, 6, 17, 25, 4, 2, 3, 8);
    }

    if (choice == 9)
    {
        return Boss(9, "Reflet d'Inakari", "Reflet kitsune autonome", 980, 8, 21, 33, 3, 3, 3, 6);
    }

    if (choice == 10)
    {
        return Boss(10, "Le Jugement Silencieux", "Manifestation affaiblie liée à Lexior", 1180, 9, 24, 36, 2, 3, 3, 7);
    }

    if (choice == 11)
    {
        return Boss(11, "Émanation de l'Anomalie", "Fragment glitch lié à la Source", 1020, 10, 26, 40, 2, 3, 3, 6);
    }

    if (choice == 12)
    {
        return Boss(12, "L'Horloge des Chuchotements", "Entité temporelle liée à l'Avarice", 1120, 8, 23, 35, 2, 4, 3, 7);
    }

    if (choice == 13)
    {
        return Boss(13, "L'Enfant des Os Enfouis", "Épreuve funéraire antique", 1160, 8, 22, 34, 2, 3, 3, 7);
    }

    if (choice == 14)
    {
        return Boss(14, "Avatar de Boros", "Avatar affaibli du dieu de la Guerre", 1380, 12, 30, 46, 1, 4, 3, 6);
    }

    if (choice == 15)
    {
        return Boss(15, "Avatar d'Anastasia", "Avatar affaibli de la déesse de l'Amour", 1240, 9, 25, 38, 3, 3, 3, 7);
    }

    if (choice == 16)
    {
        return Boss(16, "Avatar de Lexior", "Avatar affaibli du dieu de la Justice", 1320, 10, 28, 42, 2, 3, 3, 7);
    }

    if (choice == 17)
    {
        return Boss(17, "Fragment de Luna / Onyrae", "Fragment onirique lunaire instable", 1180, 9, 27, 41, 3, 4, 3, 6);
    }

    if (choice == 18)
    {
        return Boss(18, "Fragment de Syvaranelya", "Fragment élémentaire supérieur", 1420, 11, 29, 44, 2, 3, 3, 6);
    }

    if (choice == 19)
    {
        return Boss(19, "Écho d'Hitogami", "Écho affaibli du demi-dieu humain", 1280, 10, 30, 45, 2, 3, 3, 6);
    }

    if (choice == 20)
    {
        return Boss(20, "Écho de Sérendys", "Écho de chance et de destin", 1150, 9, 28, 42, 3, 4, 3, 6);
    }

    if (choice == 21)
    {
        return Boss(21, "Le Gardien du Seuil", "Gardien provisoire des paliers", 1500, 11, 31, 46, 2, 3, 3, 7);
    }

    if (choice == 22)
    {
        return Boss(22, "Le Roi Sans Salle", "Roi oublié sans royaume", 1560, 12, 33, 48, 2, 3, 3, 7);
    }

    if (choice == 23)
    {
        return Boss(23, "La Bête qui n'avait pas de nom", "Créature impossible à classer", 1480, 13, 35, 52, 1, 3, 3, 6);
    }

    if (choice == 24)
    {
        return Boss(24, "Aldebaroth", "Démon primordial affaibli", 1620, 13, 36, 54, 2, 4, 3, 7);
    }

    if (choice == 25)
    {
        return Boss(25, "Asterion et Nihilon", "Jumeaux Création / Destruction", 1760, 12, 34, 52, 2, 4, 3, 8);
    }

    if (choice == 26)
    {
        return Boss(26, "Écho fragmenté d'Obérion", "Fragment du dieu universel", 1880, 14, 40, 60, 2, 4, 3, 9);
    }

    if (choice == 27)
    {
        return Boss(27, "Avatar affaibli de FireFlight", "Avatar limité du créateur", 2300, 17, 48, 72, 2, 4, 3, 8);
    }

    if (choice == 28)
    {
        return Boss(28, "Le Souffle sans Visage", "Entité respiratoire inconnue", 1540, 13, 37, 55, 1, 3, 3, 7);
    }

    if (choice == 29)
    {
        return Boss(29, "La Marionnette aux Mille Clous", "Création cousue de douleurs anciennes", 1600, 12, 38, 56, 2, 3, 3, 7);
    }

    if (choice == 30)
    {
        return Boss(30, "Manifestation de Moiran", "Manifestation affaiblie du Destin", 1720, 13, 40, 60, 2, 4, 3, 8);
    }

    if (choice == 31)
    {
        return Boss(31, "Le Cerf des Âmes Égarées", "Gardien funéraire sylvestre", 1660, 12, 39, 58, 3, 3, 3, 7);
    }

    if (choice == 32)
    {
        return Boss(32, "Gorvald", "Roi des Orcs", 1780, 14, 42, 62, 1, 3, 3, 8);
    }

    if (choice == 33)
    {
        return Boss(33, "Serana", "Reine Vampire", 1640, 13, 40, 60, 3, 4, 3, 8);
    }

    if (choice == 34)
    {
        return Boss(34, "Draiite", "Reine Araignée", 1700, 12, 39, 58, 2, 4, 3, 8);
    }

    if (choice == 35)
    {
        return Boss(35, "Les Jumelles du Miroir Fendu", "Vérité et mensonge liés à Velyssia", 1820, 13, 43, 64, 2, 4, 3, 8);
    }

    if (choice == 36)
    {
        return Boss(36, "Source stable de l'Anomalie", "Source d'interface presque stabilisée", 2050, 15, 46, 70, 2, 4, 3, 8);
    }

    return Boss(3, "Atlas", "Protecteur universel déchu", 1450, 16, 30, 44, 2, 5, 3, 12);
}
