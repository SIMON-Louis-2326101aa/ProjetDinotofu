// EN: Church diagnosis, oath/curse cases and exorcism services split from ShopMenu.cpp.
// FR: Diagnostics, cas de malédiction/serment et exorcismes de l'église extraits de ShopMenu.cpp.
#include "interface/menu/shop/ChurchServiceMenu.hpp"
#include "interface/menu/shop/ShopServiceSupport.hpp"
#include "entity/Player.hpp"
#include "core/Console.hpp"
#include "combat/modes/pve/MonsterPveMode.hpp"
#include "combat/system/CombatClassSystem.hpp"
#include "entity/Monster.hpp"
#include "interface/menu/EquipmentMenu.hpp"
#include "interface/menu/progression/LanguagePracticeMenu.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "item/armor/ArmorCatalog.hpp"
#include "item/consumable/ConsumableCatalog.hpp"
#include "economy/shop/ShopCatalog.hpp"
#include "economy/shop/ShopItemCategory.hpp"
#include "economy/shop/ShopPriceRules.hpp"
#include "economy/shop/ShopRotationSystem.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "economy/Money.hpp"
#include "interface/menu/quest/QuestMenu.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "lore/LegendTriggerSystem.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/material/Material.hpp"
#include "quest/Quest.hpp"
#include "progression/bestiary/BestiaryRuntimeProgress.hpp"
#include "story/StoryCampaign.hpp"
#include "world/LocalReputationSystem.hpp"
#include "world/npc/NpcKnowledgeSystem.hpp"
#include "world/npc/NpcInformationPropagationSystem.hpp"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include <cstdint>

namespace
{
    using ShopServiceSupport::askShopConfirmation;
    using ShopServiceSupport::payServiceWithVoucherOrGold;
    using ShopServiceSupport::showLocalServiceResult;
    using ShopServiceSupport::showShopResult;

    std::string worldTimeLineForPlayer(const Player& player)
    {
        return player.formatWorldDateTimeLine();
    }

    std::string serviceCostLine(const Player& player, const std::string& voucherId, const std::string& voucherName, int fallbackPrice)
    {
        const int voucherCount = player.getInventory().countMaterialById(voucherId);
        if (voucherCount > 0) return "Coût prévu : " + voucherName + " x1 déjà présent dans l'inventaire.";
        return "Coût prévu : " + Money::formatGoldWithRaw(fallbackPrice) + " si aucun bon/ticket n'est présenté.";
    }

    const PlayerCurse* findActiveCurseById(const Player& player, const std::string& curseId)
    {
        const std::vector<PlayerCurse>& curses = player.getActiveCurses();
        for (const PlayerCurse& curse : curses)
        {
            if (curse.id == curseId)
            {
                return &curse;
            }
        }

        return nullptr;
    }

    struct CurseSymptomCategory
    {
        std::string id;
        std::string label;
        std::string detail;
    };

    std::vector<std::string> splitChurchTokenList(const std::string& value)
    {
        std::vector<std::string> tokens;
        std::string current;
        for (char c : value)
        {
            if (c == ',')
            {
                current.erase(std::remove_if(current.begin(), current.end(), [](unsigned char ch) { return std::isspace(ch) != 0; }), current.end());
                if (!current.empty() && std::find(tokens.begin(), tokens.end(), current) == tokens.end()) tokens.push_back(current);
                current.clear();
            }
            else
            {
                current.push_back(c);
            }
        }
        current.erase(std::remove_if(current.begin(), current.end(), [](unsigned char ch) { return std::isspace(ch) != 0; }), current.end());
        if (!current.empty() && std::find(tokens.begin(), tokens.end(), current) == tokens.end()) tokens.push_back(current);
        return tokens;
    }

    bool churchTokenListContains(const std::string& value, const std::string& token)
    {
        const std::vector<std::string> tokens = splitChurchTokenList(value);
        return std::find(tokens.begin(), tokens.end(), token) != tokens.end();
    }

    std::vector<CurseSymptomCategory> getChurchSymptomCategories()
    {
        return {
            {"health", "Santé", "fatigue, malaise, récupération, douleur vague"},
            {"attack", "Attaque", "force offensive, impact, rage, coups moins naturels"},
            {"mana", "Mana / magie", "circulation magique, réserve, instabilité de sort"},
            {"precision", "Précision", "gestes, visée, concentration, coordination"},
            {"defense", "Défense", "résistance, posture, sensation d'être exposé"},
            {"sleep", "Sommeil", "rêves, repos, cauchemars, réveils étranges"},
            {"luck", "Chance", "petits hasards, mauvaises séries, objets qui tombent mal"},
            {"equipment", "Équipement", "réaction des armes, armures, runes ou objets portés"},
            {"spirit", "Esprit", "peur, présence, voix, pensées pas tout à fait claires"},
            {"corruption", "Corruption", "trace sombre, sensation de souillure, anomalie interne"},
            {"travel", "Voyage", "route, orientation, poursuite, odeur, impression d'être suivi"},
            {"social", "Présence sociale", "regards des PNJ, gêne, aura, réactions autour de soi"},
            {"interface", "Interface", "choix affichés, vision de combat, menus qui mentent ou clignotent"},
            {"hallucination", "Hallucinations", "fausses cibles, faux PvE, voix, doubles ou silhouettes impossibles"}
        };
    }

    std::string churchSymptomCategoryLabel(const std::string& id)
    {
        for (const CurseSymptomCategory& category : getChurchSymptomCategories())
        {
            if (category.id == id) return category.label;
        }
        return id;
    }

    std::string curseKnownName(const PlayerCurse& curse)
    {
        return curse.diagnosisLevel <= 0 ? "?????" : curse.name;
    }

    std::string curseDiagnosisLevelText(const PlayerCurse& curse)
    {
        if (curse.diagnosisLevel <= 0) return "inconnue — ?????";
        if (curse.diagnosisLevel == 1) return "niveau 1/3 — globale et vague";
        if (curse.diagnosisLevel == 2) return "niveau 2/3 — approfondie";
        return "niveau 3/3 — totale";
    }

    std::string churchCurseDetailText(const PlayerCurse& curse)
    {
        if (curse.diagnosisLevel <= 0)
        {
            return "Trace inconnue : ?????. Diagnostic nécessaire avant tout exorcisme.";
        }

        std::string detail = "Connaissance : " + curseDiagnosisLevelText(curse) + ". ";
        detail += "Niveau de malédiction : " + std::to_string(std::max(1, curse.curseLevel)) + "/" + std::to_string(std::max(1, curse.maxCurseLevel)) + ". ";
        const std::vector<std::string> discovered = splitChurchTokenList(curse.discoveredSymptomCategories);
        if (!discovered.empty())
        {
            detail += "Symptômes confirmés : ";
            for (std::size_t i = 0; i < discovered.size(); ++i)
            {
                if (i > 0) detail += ", ";
                detail += churchSymptomCategoryLabel(discovered[i]);
            }
            detail += ". ";
        }
        else
        {
            detail += "Symptômes encore vagues. ";
        }

        if (curse.diagnosisLevel >= 2)
        {
            if (curse.evolvesOverTime && curse.curseLevel < curse.maxCurseLevel && curse.nextEscalationDay >= 0)
            {
                detail += "Risque d'aggravation : après le jour " + std::to_string(curse.nextEscalationDay + 1) + ". ";
            }
            if (curse.expiresAtDay >= 0)
            {
                detail += "Expiration naturelle : fin du jour " + std::to_string(curse.expiresAtDay + 1) + ". ";
            }
            else if (curse.lifeLong)
            {
                detail += "Durée : vie entière si rien de spécial n'est fait. ";
            }
            else
            {
                detail += "Durée : indéfinie. ";
            }
        }
        else
        {
            detail += "Durée : ?????. ";
        }

        if (curse.bossIdRequiredToBreak > 0)
        {
            detail += curse.diagnosisLevel >= 3
                ? "Verrou de source : " + (curse.removalHint.empty() ? "vaincre la source." : curse.removalHint)
                : "Verrou de source soupçonné : diagnostic total conseillé.";
        }
        else if (curse.removableByChurch)
        {
            if (curse.evolvesOverTime && curse.becomesSpecialRemovalWhenTooHigh && curse.curseLevel <= curse.churchRemovalMaxLevel)
            {
                detail += "Attention : à trop haut niveau, l'église seule ne suffira plus. ";
            }
            if (curse.exorcismRequiredVisits > 1)
            {
                detail += "Exorcisme progressif : " + std::to_string(curse.exorcismProgress) + "/" + std::to_string(curse.exorcismRequiredVisits) + " passage(s).";
            }
            else
            {
                detail += "Exorcisme court possible ici.";
            }
        }
        else
        {
            detail += curse.diagnosisLevel >= 3 && !curse.removalHint.empty() ? curse.removalHint : "Condition spéciale encore floue.";
        }

        return detail;
    }

    bool rollChurchDiagnosisFailure()
    {
        static std::mt19937 generator(std::random_device{}());
        std::uniform_int_distribution<int> distribution(1, 100);
        return distribution(generator) <= 10;
    }

    int chooseCurseForChurchService(const Player& player, const std::string& title, const std::string& prompt, bool requireCurse)
    {
        const std::vector<PlayerCurse>& curses = player.getActiveCurses();
        if (curses.empty())
        {
            showShopResult(title, "shop.church.no_curse", {"Aucune trace active à analyser."});
            return -1;
        }

        MenuScreen screen(title, "shop.church.choose_curse");
        screen.addLine("Choisis la trace à étudier. Les traces inconnues restent volontairement affichées en ?????.");
        screen.addOption(0, "Retour", "Revenir aux services d'église.", true, "shop.church.choose_curse.back");
        for (std::size_t i = 0; i < curses.size(); ++i)
        {
            const PlayerCurse& curse = curses[i];
            screen.addOption(
                static_cast<int>(i + 1),
                curseKnownName(curse) + " | " + curseDiagnosisLevelText(curse),
                churchCurseDetailText(curse),
                true,
                "shop.church.choose_curse." + std::to_string(i)
            );
        }
        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, prompt);
        if (choice <= 0 || choice > static_cast<int>(curses.size()))
        {
            return -1;
        }
        (void)requireCurse;
        return choice - 1;
    }

    std::string chooseSymptomCategoryForChurchDiagnosis(const PlayerCurse& curse)
    {
        const std::vector<CurseSymptomCategory> categories = getChurchSymptomCategories();
        MenuScreen screen("CIBLER UN SYMPTÔME", "shop.church.symptom_category");
        screen.addLine("Choisis une catégorie volontairement vague. L'église ne te spoil pas un -20% dégâts ou un effet exact.");
        screen.addLine("Mauvaise piste : elle sera écartée, avec environ 20% d'autres mauvaises pistes pour ne pas rendre la recherche abusive.");
        screen.addOption(0, "Retour", "Ne rien diagnostiquer pour l'instant.", true, "shop.church.symptom.back");
        for (std::size_t i = 0; i < categories.size(); ++i)
        {
            const CurseSymptomCategory& category = categories[i];
            const bool alreadyConfirmed = churchTokenListContains(curse.discoveredSymptomCategories, category.id);
            const bool excluded = churchTokenListContains(curse.excludedSymptomCategories, category.id);
            std::string label = category.label;
            if (alreadyConfirmed) label += " (déjà confirmé)";
            if (excluded) label += " (piste écartée)";
            screen.addOption(
                static_cast<int>(i + 1),
                label,
                category.detail,
                !excluded && !alreadyConfirmed,
                "shop.church.symptom." + category.id
            );
        }
        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Quelle catégorie veux-tu faire tester ?");
        if (choice <= 0 || choice > static_cast<int>(categories.size()))
        {
            return "";
        }
        return categories[static_cast<std::size_t>(choice - 1)].id;
    }

    void addChurchDiagnosisLines(Player& player, std::vector<std::string>& lines)
    {
        std::vector<std::string> curseLines = player.describeActiveCurses();
        lines.push_back("État des traces :");
        lines.insert(lines.end(), curseLines.begin(), curseLines.end());
        if (player.getActiveCurseCount() == 0)
        {
            lines.push_back("Frère Calixte ajoute que les PNJ pourront eux aussi porter des traces plus tard, surtout après certains événements ou boss.");
        }
        else
        {
            lines.push_back("Règle de Maëlys : l'église refuse d'exorciser une trace tant qu'elle n'est pas connue au moins au niveau 1.");
        }
    }

    void runTargetedChurchDiagnosis(Player& player)
    {
        const int curseIndex = chooseCurseForChurchService(player, "DIAGNOSTIC CIBLÉ", "Quelle trace veux-tu étudier ?", true);
        if (curseIndex < 0) return;
        const PlayerCurse target = player.getActiveCurses()[static_cast<std::size_t>(curseIndex)];
        const std::string categoryId = chooseSymptomCategoryForChurchDiagnosis(target);
        if (categoryId.empty()) return;

        std::vector<std::string> lines;
        if (!payServiceWithVoucherOrGold(player, "sanctuary_candle", "Cierge de veille", 32, lines))
        {
            showShopResult("DIAGNOSTIC REFUSÉ", "shop.church.diagnosis.targeted.failed_cost", lines);
            return;
        }

        if (rollChurchDiagnosisFailure())
        {
            lines.push_back("Sœur Maëlys ferme les yeux, puis secoue la tête : le signe se brouille au dernier moment.");
            lines.push_back("Diagnostic échoué : rien n'est confirmé ni écarté. Cela arrive environ une fois sur dix.");
            showLocalServiceResult("DIAGNOSTIC ÉCHOUÉ", "shop.church.diagnosis.targeted.failed_roll", player, lines, 1);
            return;
        }

        if (player.revealCurseSymptomCategory(target.id, categoryId))
        {
            lines.push_back("Piste confirmée : la catégorie " + churchSymptomCategoryLabel(categoryId) + " réagit à la trace.");
            lines.push_back("Connaissance minimale atteinte : niveau 1/3 si elle ne l'était pas déjà.");
            lines.push_back("Maëlys reste prudente : elle sait où chercher, pas encore tout ce que la malédiction fait exactement.");
        }
        else
        {
            player.excludeCurseSymptomCategory(target.id, categoryId);
            const int removed = player.autoExcludeWrongCurseSymptomCategories(target.id, 20);
            lines.push_back("Piste saine : rien n'indique que la catégorie " + churchSymptomCategoryLabel(categoryId) + " soit touchée.");
            lines.push_back("Maëlys ajoute : \"Cela ne veut pas dire que tout va bien. Le problème peut être ailleurs.\"");
            lines.push_back("Aide au diagnostic : " + std::to_string(std::max(1, removed)) + " autre(s) mauvaise(s) piste(s) ont été écartée(s).");
        }
        showLocalServiceResult("DIAGNOSTIC CIBLÉ", "shop.church.diagnosis.targeted.result", player, lines, 1);
    }

    void revealFirstUnknownCategory(Player& player, const PlayerCurse& target, std::vector<std::string>& lines)
    {
        const std::vector<std::string> categories = splitChurchTokenList(target.symptomCategories);
        for (const std::string& category : categories)
        {
            if (churchTokenListContains(target.discoveredSymptomCategories, category))
            {
                continue;
            }
            if (player.revealCurseSymptomCategory(target.id, category))
            {
                lines.push_back("Symptôme vague confirmé : " + churchSymptomCategoryLabel(category) + ".");
                return;
            }
        }
    }

    void runGeneralChurchDiagnosis(Player& player)
    {
        const int curseIndex = chooseCurseForChurchService(player, "DIAGNOSTIC GÉNÉRAL", "Quelle trace veux-tu faire lire globalement ?", true);
        if (curseIndex < 0) return;
        const PlayerCurse target = player.getActiveCurses()[static_cast<std::size_t>(curseIndex)];
        std::vector<std::string> lines;
        if (!payServiceWithVoucherOrGold(player, "exorcism_incense", "Encens d'exorcisme", 86, lines))
        {
            showShopResult("DIAGNOSTIC REFUSÉ", "shop.church.diagnosis.general.failed_cost", lines);
            return;
        }
        if (rollChurchDiagnosisFailure())
        {
            lines.push_back("L'encens tourne dans le mauvais sens. Père Orwan refuse de conclure sur une lecture sale.");
            lines.push_back("Diagnostic échoué : aucun niveau gagné. Chance d'échec : 10%.");
            showLocalServiceResult("DIAGNOSTIC GÉNÉRAL ÉCHOUÉ", "shop.church.diagnosis.general.failed_roll", player, lines, 1);
            return;
        }

        player.setCurseDiagnosisLevel(target.id, 1);
        revealFirstUnknownCategory(player, target, lines);
        lines.push_back("Diagnostic général : la trace est maintenant connue au niveau 1/3.");
        lines.push_back("Ce niveau suffit pour autoriser un exorcisme si la malédiction est retirable par l'église, mais il ne révèle pas encore toute l'origine ni les détails.");
        showLocalServiceResult("DIAGNOSTIC GÉNÉRAL", "shop.church.diagnosis.general.success", player, lines, 1);
    }

    void runDeepChurchDiagnosis(Player& player)
    {
        const int curseIndex = chooseCurseForChurchService(player, "DIAGNOSTIC APPROFONDI", "Quelle trace veux-tu approfondir ?", true);
        if (curseIndex < 0) return;
        const PlayerCurse target = player.getActiveCurses()[static_cast<std::size_t>(curseIndex)];
        std::vector<std::string> lines;
        if (target.diagnosisLevel < 1)
        {
            showShopResult("RECHERCHE REFUSÉE", "shop.church.diagnosis.deep.locked", {"Maëlys refuse : il faut d'abord connaître la trace au niveau 1."});
            return;
        }
        if (!payServiceWithVoucherOrGold(player, "exorcism_incense", "Encens d'exorcisme", 128, lines))
        {
            showShopResult("DIAGNOSTIC REFUSÉ", "shop.church.diagnosis.deep.failed_cost", lines);
            return;
        }
        if (rollChurchDiagnosisFailure())
        {
            lines.push_back("Le cercle tient, puis se coupe net. Frère Calixte note : lecture instable, aucun résultat fiable.");
            lines.push_back("Diagnostic échoué : aucun niveau gagné. Chance d'échec : 10%.");
            showLocalServiceResult("DIAGNOSTIC APPROFONDI ÉCHOUÉ", "shop.church.diagnosis.deep.failed_roll", player, lines, 1);
            return;
        }

        player.setCurseDiagnosisLevel(target.id, 2);
        revealFirstUnknownCategory(player, target, lines);
        lines.push_back("Diagnostic approfondi : niveau 2/3 atteint.");
        lines.push_back("Origine mieux cernée : " + (target.origin.empty() ? "source encore floue" : target.origin) + ".");
        lines.push_back("La durée devient lisible, mais les effets exacts et conditions complètes demandent encore une recherche totale.");
        showLocalServiceResult("DIAGNOSTIC APPROFONDI", "shop.church.diagnosis.deep.success", player, lines, 1);
    }

    void runTotalChurchDiagnosis(Player& player)
    {
        const int curseIndex = chooseCurseForChurchService(player, "DIAGNOSTIC TOTAL", "Quelle trace veux-tu comprendre totalement ?", true);
        if (curseIndex < 0) return;
        const PlayerCurse target = player.getActiveCurses()[static_cast<std::size_t>(curseIndex)];
        std::vector<std::string> lines;
        if (target.diagnosisLevel < 2)
        {
            showShopResult("RECHERCHE REFUSÉE", "shop.church.diagnosis.total.locked", {"Père Orwan refuse : il faut d'abord atteindre le diagnostic approfondi niveau 2."});
            return;
        }
        if (!payServiceWithVoucherOrGold(player, "white_bone_chalk", "Craie d'os blanc", 185, lines))
        {
            showShopResult("DIAGNOSTIC REFUSÉ", "shop.church.diagnosis.total.failed_cost", lines);
            return;
        }
        if (rollChurchDiagnosisFailure())
        {
            lines.push_back("La craie se fend avant la fin du cercle. Maëlys efface tout : mieux vaut rater que mentir.");
            lines.push_back("Diagnostic échoué : aucun niveau gagné. Chance d'échec : 10%.");
            showLocalServiceResult("DIAGNOSTIC TOTAL ÉCHOUÉ", "shop.church.diagnosis.total.failed_roll", player, lines, 1);
            return;
        }

        player.setCurseDiagnosisLevel(target.id, 3);
        const std::vector<std::string> categories = splitChurchTokenList(target.symptomCategories);
        for (const std::string& category : categories)
        {
            player.revealCurseSymptomCategory(target.id, category);
        }
        lines.push_back("Diagnostic total : niveau 3/3 atteint.");
        lines.push_back("Lecture complète : " + (target.description.empty() ? "aucun détail exact n'est lisible pour l'instant." : target.description));
        if (!target.removalHint.empty())
        {
            lines.push_back("Condition de retrait : " + target.removalHint);
        }
        showLocalServiceResult("DIAGNOSTIC TOTAL", "shop.church.diagnosis.total.success", player, lines, 1);
    }

    struct ChurchCaseOption
    {
        int menuChoice = 0;
        int type = 0;
        std::string label;
        std::string detail;
        std::string sceneIntro;
        std::string investigationLine;
        std::string expectedClueLine;
        std::string screenId;
        std::string requiredProofId;
        std::string requiredProofName;
        std::string rewardProofId;
        std::string rewardProofName;
    };

    bool playerHasMaterial(const Player& player, const std::string& materialId, int quantity = 1)
    {
        return materialId.empty() || player.getInventory().countMaterialById(materialId) >= std::max(1, quantity);
    }

    std::string churchMaterialStatusLine(const Player& player, const std::string& materialId, const std::string& label, int quantity = 1)
    {
        if (materialId.empty())
        {
            return "Aucune preuve préalable demandée.";
        }
        return label + " : " + std::to_string(player.getInventory().countMaterialById(materialId)) + "/" + std::to_string(std::max(1, quantity));
    }

    std::vector<ChurchCaseOption> getChurchCaseOptions(const Player& player)
    {
        (void)player;
        return {
            {
                1,
                1,
                "Lysa parle en dormant — veiller avec sa mère",
                "Sœur Maëlys demande une première veillée discrète : on écoute avant de conclure.",
                "Mira, la mère de Lysa, jure que sa fille prononce des phrases qu'elle n'a jamais apprises.",
                "Tu dois apporter de l'encens pour que Maëlys reste dans la chambre sans effrayer l'enfant.",
                "Indice attendu : les mots exacts entendus pendant la nuit.",
                "shop.church.requests.sleep.start",
                "exorcism_incense",
                "Encens d'exorcisme",
                "church_night_testimony",
                "Témoignage nocturne signé"
            },
            {
                2,
                2,
                "Lysa parle en dormant — lire la chambre",
                "La chambre doit être examinée avant de viser la personne. Le patient n'est pas forcément la source.",
                "Maëlys relit le témoignage de Mira et refuse de poser la main sur Lysa sans preuve plus sûre.",
                "La veillée a donné assez de détails pour inspecter le lit, les murs et les objets proches.",
                "Indice attendu : savoir si la trace vient de la chambre ou de Lysa elle-même.",
                "shop.church.requests.sleep.diagnosis",
                "church_night_testimony",
                "Témoignage nocturne signé",
                "church_sleep_diagnosis",
                "Diagnostic de chambre endormie"
            },
            {
                3,
                3,
                "Lysa parle en dormant — trouver l'objet accroché",
                "La lecture pointe vers un objet banal. Il faut l'isoler sans accuser la famille.",
                "Père Orwan prépare une fiole et demande de rester calme : détruire le mauvais objet empirerait tout.",
                "Le diagnostic de chambre donne une zone, pas encore le coupable exact.",
                "Indice attendu : l'objet qui porte réellement la trace.",
                "shop.church.requests.sleep.resolve",
                "church_sleep_diagnosis",
                "Diagnostic de chambre endormie",
                "identified_source_object",
                "Objet source identifié"
            },
            {
                4,
                4,
                "Ronan s'étouffe quand il ment — nommer le serment",
                "Un charretier panique dès qu'il parle d'une promesse ancienne. Il faut d'abord retrouver les mots exacts.",
                "Ronan répète qu'il n'a rien promis, mais son souffle se coupe toujours au même moment.",
                "Père Orwan demande un sceau pour recueillir sa parole sans qu'elle soit contestée plus tard.",
                "Indice attendu : le nom du serment ou de la personne liée.",
                "shop.church.requests.oath.named",
                "sanctuary_wax_seal",
                "Sceau de cire sanctuaire",
                "named_oath_testimony",
                "Témoignage de serment nommé"
            },
            {
                5,
                5,
                "Le dortoir fait le même rêve — comparer les récits",
                "Plusieurs apprentis décrivent un rêve identique. Calixte veut comparer les détails sans choisir un coupable.",
                "Ivo, Nelle et Sali dessinent tous la même porte, mais aucun ne se souvient l'avoir vue éveillé.",
                "Une note de bénédiction sert de prétexte propre pour recueillir les récits sans affoler le dortoir.",
                "Indice attendu : le motif commun du rêve.",
                "shop.church.requests.dream.pattern",
                "blessing_note",
                "Note de bénédiction",
                "shared_dream_pattern",
                "Motif de rêve partagé"
            },
            {
                6,
                6,
                "Le dortoir fait le même rêve — chercher la contre-légende",
                "Le motif existe peut-être déjà dans les archives. Une histoire ancienne a souvent une fin ancienne.",
                "Frère Calixte reconnaît une porte dessinée dans un vieux conte, mais il lui manque la version qui la referme.",
                "Le motif partagé sert de clé de recherche, pas de preuve définitive.",
                "Indice attendu : une version de la légende qui sait comment se terminer.",
                "shop.church.requests.dream.legend",
                "shared_dream_pattern",
                "Motif de rêve partagé",
                "copied_counter_legend",
                "Contre-légende copiée"
            },
            {
                7,
                8,
                "Elian regrette un pacte — nommer la contrepartie",
                "Un aventurier a accepté une aide trop facile. L'église ne rompt rien tant que le prix exact n'est pas avoué.",
                "Elian affirme qu'il a seulement 'accepté un coup de chance'. Maëlys entend surtout une dette qui respire derrière ses mots.",
                "La craie d'os sert à tracer une limite pendant qu'il raconte ce qu'il a promis, sans que la promesse lui ferme la bouche.",
                "Indice attendu : la contrepartie exacte du pacte volontaire.",
                "shop.church.requests.pact.witness",
                "white_bone_chalk",
                "Craie d'os blanc",
                "pact_break_witness",
                "Témoin de pacte rompu"
            },
            {
                8,
                7,
                "Le vieux seuil répond — dessiner ce qu'il faut sceller",
                "Un seuil ou un puits réagit aux passages. L'église veut un croquis précis avant de parler de scellement.",
                "Père Orwan insiste : sceller un lieu sans le comprendre, c'est juste enfermer la peur avec la prochaine victime.",
                "Une note d'exorciste permet de préparer le cercle, mais pas de deviner le lieu à la place du joueur.",
                "Indice attendu : l'emplacement exact de la source scellable.",
                "shop.church.requests.source.sketch",
                "exorcist_note",
                "Note d'exorciste",
                "sealable_source_sketch",
                "Croquis de source scellable"
            }
        };
    }

    Quest makeChurchNpcCurseQuest(Player& player, int type)
    {
        Quest quest;
        quest.origin = "Église et exorcisme";
        quest.location = "Église et salle des prières";
        quest.accepted = true;
        quest.availableFromDay = player.getWorldDaysElapsed();
        quest.expiresAtDay = player.getWorldDaysElapsed() + 6;
        quest.target = 1;
        quest.progress = 0;
        quest.guildQuest = false;
        quest.objectiveType = "material";
        quest.targetFamily = "Cas d'église";
        quest.requiredMaterialQuantity = 1;
        quest.rewardMaterialQuantity = 1;

        if (type == 1)
        {
            quest.id = "church_sleep_watch_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 25 ? "C" : "D";
            quest.title = "Lysa parle en dormant — veillée";
            quest.client = "Sœur Maëlys l'exorciste";
            quest.objective = "Apporter un encens d'exorcisme et accompagner Mira pendant la veillée. Il faut noter les mots de Lysa, pas décider déjà ce qu'elle a.";
            quest.requiredMaterialId = "exorcism_incense";
            quest.requiredMaterialName = "Encens d'exorcisme";
            quest.rewardExperience = 18 + player.getLevel() * 2;
            quest.rewardGold = 48 + player.getLevel() * 3;
            quest.rewardMaterialId = "church_night_testimony";
            quest.rewardMaterialName = "Témoignage nocturne signé";
            quest.rewardNote = "Mira a enfin raconté ce qu'elle entend. Le cas est crédible, mais la cause reste inconnue.";
            return quest;
        }

        if (type == 2)
        {
            quest.id = "church_sleep_room_reading_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 28 ? "C" : "D";
            quest.title = "Lysa parle en dormant — chambre";
            quest.client = "Sœur Maëlys l'exorciste";
            quest.objective = "Ramener les mots entendus pendant la veillée pour lire la chambre, les objets et les traces de passage avant de viser l'enfant.";
            quest.requiredMaterialId = "church_night_testimony";
            quest.requiredMaterialName = "Témoignage nocturne signé";
            quest.rewardExperience = 22 + player.getLevel() * 2;
            quest.rewardGold = 58 + player.getLevel() * 3;
            quest.rewardMaterialId = "church_sleep_diagnosis";
            quest.rewardMaterialName = "Diagnostic de chambre endormie";
            quest.rewardNote = "La lecture vise le lieu : Maëlys comprend mieux où chercher sans accuser Lysa.";
            return quest;
        }

        if (type == 3)
        {
            quest.id = "church_sleep_source_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 32 ? "C" : "D";
            quest.title = "Lysa parle en dormant — objet accroché";
            quest.client = "Sœur Maëlys l'exorciste";
            quest.objective = "Utiliser le diagnostic de chambre pour isoler l'objet banal qui accroche le sommeil de Lysa. La famille doit rester protégée de la panique.";
            quest.requiredMaterialId = "church_sleep_diagnosis";
            quest.requiredMaterialName = "Diagnostic de chambre endormie";
            quest.requiredMaterialQuantity = 1;
            quest.rewardExperience = 28 + player.getLevel() * 2;
            quest.rewardGold = 70 + player.getLevel() * 3;
            quest.rewardMaterialId = "identified_source_object";
            quest.rewardMaterialName = "Objet source identifié";
            quest.rewardNote = "Le patient n'est pas le problème : l'objet source peut maintenant être traité par une solution spéciale.";
            return quest;
        }

        if (type == 4)
        {
            quest.id = "church_oath_named_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 35 ? "B" : "C";
            quest.title = "Ronan s'étouffe quand il ment — serment nommé";
            quest.client = "Père Orwan";
            quest.objective = "Fournir un sceau sanctuaire pour écouter Ronan sans le forcer. Le but est de retrouver le nom de la promesse qui serre sa gorge.";
            quest.requiredMaterialId = "sanctuary_wax_seal";
            quest.requiredMaterialName = "Sceau de cire sanctuaire";
            quest.rewardExperience = 30 + player.getLevel() * 2;
            quest.rewardGold = 78 + player.getLevel() * 4;
            quest.rewardMaterialId = "named_oath_testimony";
            quest.rewardMaterialName = "Témoignage de serment nommé";
            quest.rewardNote = "Le serment a un nom. Une malédiction de parole totalement diagnostiquée pourra être brisée avec cette preuve.";
            return quest;
        }

        if (type == 5)
        {
            quest.id = "church_dream_pattern_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = "C";
            quest.title = "Le dortoir fait le même rêve — motif";
            quest.client = "Frère Calixte";
            quest.objective = "Apporter une note de bénédiction et comparer les récits d'Ivo, Nelle et Sali sans décider trop vite qu'un seul patient est fautif.";
            quest.requiredMaterialId = "blessing_note";
            quest.requiredMaterialName = "Note de bénédiction";
            quest.rewardExperience = 24 + player.getLevel() * 2;
            quest.rewardGold = 64 + player.getLevel() * 3;
            quest.rewardMaterialId = "shared_dream_pattern";
            quest.rewardMaterialName = "Motif de rêve partagé";
            quest.rewardNote = "Les trois récits partagent le même motif. Il peut maintenant guider une recherche de contre-légende.";
            return quest;
        }

        if (type == 6)
        {
            quest.id = "church_dream_counter_legend_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 30 ? "C" : "D";
            quest.title = "Le dortoir fait le même rêve — contre-légende";
            quest.client = "Frère Calixte";
            quest.objective = "Ramener le motif de rêve partagé aux archives pour recopier la version de l'histoire qui sait comment se terminer.";
            quest.requiredMaterialId = "shared_dream_pattern";
            quest.requiredMaterialName = "Motif de rêve partagé";
            quest.rewardExperience = 26 + player.getLevel() * 2;
            quest.rewardGold = 58 + player.getLevel() * 3;
            quest.rewardMaterialId = "copied_counter_legend";
            quest.rewardMaterialName = "Contre-légende copiée";
            quest.rewardNote = "La contre-légende n'est pas un sort gratuit : elle donne une condition concrète pour refermer une histoire maudite.";
            return quest;
        }

        if (type == 8)
        {
            quest.id = "church_pact_witness_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
            quest.rank = player.getLevel() >= 34 ? "B" : "C";
            quest.title = "Elian regrette un pacte — contrepartie";
            quest.client = "Sœur Maëlys l'exorciste";
            quest.objective = "Apporter de la craie d'os blanc et écouter Elian jusqu'à ce que la contrepartie du pacte soit nommée. Sans le prix exact, l'église refuse de rompre au hasard.";
            quest.requiredMaterialId = "white_bone_chalk";
            quest.requiredMaterialName = "Craie d'os blanc";
            quest.rewardExperience = 30 + player.getLevel() * 2;
            quest.rewardGold = 76 + player.getLevel() * 4;
            quest.rewardMaterialId = "pact_break_witness";
            quest.rewardMaterialName = "Témoin de pacte rompu";
            quest.rewardNote = "La contrepartie a été nommée. Une marque de pacte totalement diagnostiquée pourra être rompue sans viser la mauvaise faute.";
            return quest;
        }

        quest.id = "church_sealable_source_" + std::to_string(player.getWorldDaysElapsed()) + "_" + std::to_string(player.getLevel());
        quest.rank = player.getLevel() >= 40 ? "B" : "C";
        quest.title = "Le vieux seuil répond — croquis scellable";
        quest.client = "Père Orwan";
        quest.objective = "Apporter une note d'exorciste pour préparer un croquis précis du seuil. Il servira seulement si la source est déjà totalement comprise.";
        quest.requiredMaterialId = "exorcist_note";
        quest.requiredMaterialName = "Note d'exorciste";
        quest.rewardExperience = 32 + player.getLevel() * 2;
        quest.rewardGold = 88 + player.getLevel() * 4;
        quest.rewardMaterialId = "sealable_source_sketch";
        quest.rewardMaterialName = "Croquis de source scellable";
        quest.rewardNote = "Le croquis ne bat pas un boss. Il donne une condition concrète pour sceller une trace de seuil diagnostiquée à fond.";
        return quest;
    }

    void openChurchTroubledPrayerRequests(Player& player)
    {
        const std::vector<ChurchCaseOption> options = getChurchCaseOptions(player);
        MenuScreen screen("PRIÈRES ET DEMANDES INQUIÉTANTES", "shop.church.troubled_requests");
        screen.addLine("Frère Calixte lit les billets déposés près des cierges : ici, on avance par preuves, pas par fiches froides.");
        screen.addLine("Chaque demande est une petite scène : écouter une personne, chercher un indice, puis revenir avec quelque chose de concret.");
        screen.addLine("Les objets affichés servent juste de trace de suivi dans l'inventaire ; en jeu, ce sont des indices ou des notes de terrain.");
        screen.addOption(0, "Retour", "Revenir aux services d'église.", true, "shop.church.requests.back");
        for (const ChurchCaseOption& option : options)
        {
            const bool available = playerHasMaterial(player, option.requiredProofId);
            std::string detail = option.detail;
            detail += " | Besoin actuel : " + churchMaterialStatusLine(player, option.requiredProofId, option.requiredProofName) + ".";
            detail += " | Indice à obtenir : " + option.expectedClueLine;
            screen.addOption(option.menuChoice, option.label, detail, available, option.screenId);
        }

        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une demande à prendre au sérieux.");
        if (choice <= 0)
        {
            return;
        }

        auto it = std::find_if(options.begin(), options.end(), [choice](const ChurchCaseOption& option) {
            return option.menuChoice == choice;
        });
        if (it == options.end())
        {
            return;
        }

        Quest quest = makeChurchNpcCurseQuest(player, it->type);
        std::vector<std::string> lines;
        if (!playerHasMaterial(player, it->requiredProofId))
        {
            lines.push_back("Pré-requis manquant : " + it->requiredProofName + ".");
            lines.push_back("L'église refuse de sauter une étape : sinon on invente un diagnostic au lieu d'aider quelqu'un.");
            showShopResult("DEMANDE REFUSÉE", "shop.church.requests.missing_proof", lines);
            return;
        }

        if (!player.getQuestLog().canAcceptPersonalQuestForClient(quest.client))
        {
            lines.push_back(quest.client + " t'a déjà confié assez d'urgences pour l'instant.");
            lines.push_back("Termine ou abandonne une autre demande avant de prendre un nouveau cas d'église.");
            showShopResult("DEMANDE REFUSÉE", "shop.church.requests.blocked", lines);
            return;
        }

        if (player.getQuestLog().addQuest(quest))
        {
            player.getQuestLog().refreshMaterialDeliveryQuests(player.getInventory());
            lines.push_back("Demande acceptée : " + quest.title + ".");
            lines.push_back("Référent : " + quest.client + ".");
            lines.push_back("Scène : " + it->sceneIntro);
            lines.push_back("Ce que tu vas faire : " + it->investigationLine);
            lines.push_back(it->expectedClueLine);
            lines.push_back("Objectif journal : " + quest.objective);
            lines.push_back("Indice conservé ensuite : " + quest.rewardMaterialName + ".");
            lines.push_back("Important : tu aides une personne, pas une case à cocher. L'objet de suivi existe pour ne pas perdre le fil de l'enquête.");
            showShopResult("DEMANDE ACCEPTÉE", "shop.church.requests.accepted", lines);
        }
        else
        {
            lines.push_back("Le journal refuse cette demande : elle est peut-être déjà active ou incompatible avec tes demandes actuelles.");
            showShopResult("DEMANDE NON AJOUTÉE", "shop.church.requests.failed", lines);
        }
    }

    void showCurseLegendArchive()
    {
        MenuScreen screen("LÉGENDES DE MALÉDICTION", "shop.church.curse_legends");
        screen.addLine("La bibliothèque ne donne pas de faiblesse gratuite, mais elle peut expliquer les familles de solutions spéciales.");
        screen.addOption(0, "Retour", "Revenir aux services d'église.", true, "shop.church.legends.back");
        screen.addOption(1, "Lire : Les trois manières de rompre une trace", "Objet source, serment, contre-légende.", true, "shop.church.legends.counter_rites");
        screen.addOption(2, "Lire : Le patient n'est pas la malédiction", "Cas humains, symptômes flous et prudence d'exorciste.", true, "shop.church.legends.cursed_patients");

        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une lecture.");
        if (choice == 1)
        {
            LegendTriggerSystem::displayArchiveEntry("curse_counter_rites");
        }
        else if (choice == 2)
        {
            LegendTriggerSystem::displayArchiveEntry("curse_cursed_patients");
        }
    }

    struct SpecialCurseProof
    {
        std::string solutionId;
        std::string materialId;
        std::string materialName;
    };

    SpecialCurseProof getSpecialCurseProof(const std::string& solutionId)
    {
        if (solutionId == "destroy_source_object") return {solutionId, "identified_source_object", "Objet source identifié"};
        if (solutionId == "break_oath") return {solutionId, "named_oath_testimony", "Témoignage de serment nommé"};
        if (solutionId == "read_counter_legend") return {solutionId, "copied_counter_legend", "Contre-légende copiée"};
        if (solutionId == "seal_source") return {solutionId, "sealable_source_sketch", "Croquis de source scellable"};
        if (solutionId == "break_pact") return {solutionId, "pact_break_witness", "Témoin de pacte rompu"};
        if (solutionId == "confirm_source_defeated") return {solutionId, "source_defeat_notice", "Note de source vaincue"};
        return {solutionId, "", ""};
    }

    SpecialCurseProof getSpecialCurseOutcome(const std::string& solutionId)
    {
        if (solutionId == "destroy_source_object") return {solutionId, "purified_source_ashes", "Cendres de source purifiée"};
        if (solutionId == "break_oath") return {solutionId, "broken_oath_record", "Acte de serment brisé"};
        if (solutionId == "read_counter_legend") return {solutionId, "closed_counter_legend", "Contre-légende refermée"};
        if (solutionId == "seal_source") return {solutionId, "sealed_source_mark", "Marque de source scellée"};
        if (solutionId == "break_pact") return {solutionId, "released_pact_record", "Acte de pacte libéré"};
        if (solutionId == "confirm_source_defeated") return {solutionId, "confirmed_source_silence", "Silence de source confirmé"};
        return {solutionId, "", ""};
    }

    bool hasSpecialCurseProof(const Player& player, const std::string& solutionId)
    {
        const SpecialCurseProof proof = getSpecialCurseProof(solutionId);
        return proof.materialId.empty() || player.getInventory().countMaterialById(proof.materialId) > 0;
    }

    std::string specialCurseProofLine(const Player& player, const std::string& solutionId)
    {
        const SpecialCurseProof proof = getSpecialCurseProof(solutionId);
        if (proof.materialId.empty()) return "Preuve : aucune.";
        return "Preuve suivie : " + proof.materialName + " " + std::to_string(player.getInventory().countMaterialById(proof.materialId)) + "/1.";
    }

    void runSpecialCurseSolution(Player& player)
    {
        MenuScreen screen("SOLUTIONS SPÉCIALES", "shop.church.special_solution");
        screen.addLine("Ces actions demandent deux choses : comprendre la malédiction au niveau 3/3, puis posséder l'indice concret obtenu en enquête.");
        screen.addLine("Ici, Maëlys ne soigne pas au hasard : elle vérifie la source, le serment, la légende ou le seuil avant d'agir.");
        screen.addOption(0, "Retour", "Revenir aux services d'église.", true, "shop.church.special.back");
        screen.addOption(1, "Détruire l'objet source", "Scène : isoler l'objet responsable, puis le briser dans un cercle sûr. " + specialCurseProofLine(player, "destroy_source_object"), player.hasCurseEligibleForSpecialSolution("destroy_source_object") && hasSpecialCurseProof(player, "destroy_source_object"), "shop.church.special.destroy_object");
        screen.addOption(2, "Briser un serment", "Scène : faire perdre son droit à une promesse qui serre encore. " + specialCurseProofLine(player, "break_oath"), player.hasCurseEligibleForSpecialSolution("break_oath") && hasSpecialCurseProof(player, "break_oath"), "shop.church.special.break_oath");
        screen.addOption(3, "Lire une contre-légende", "Scène : lire la version de l'histoire où la trace accepte de finir. " + specialCurseProofLine(player, "read_counter_legend"), player.hasCurseEligibleForSpecialSolution("read_counter_legend") && hasSpecialCurseProof(player, "read_counter_legend"), "shop.church.special.counter_legend");
        screen.addOption(4, "Sceller une source", "Scène : refermer un passage déjà compris, sans prétendre vaincre ce qui vit derrière. " + specialCurseProofLine(player, "seal_source"), player.hasCurseEligibleForSpecialSolution("seal_source") && hasSpecialCurseProof(player, "seal_source"), "shop.church.special.seal_source");
        screen.addOption(5, "Rompre un pacte volontaire", "Scène : faire tomber la contrepartie nommée, pas annuler gratuitement un choix. " + specialCurseProofLine(player, "break_pact"), player.hasCurseEligibleForSpecialSolution("break_pact") && hasSpecialCurseProof(player, "break_pact"), "shop.church.special.break_pact");
        screen.addOption(6, "Confirmer une source vaincue", "Scène : retirer la trace d'un seuil après une vraie victoire contre sa source. " + specialCurseProofLine(player, "confirm_source_defeated"), player.hasCurseEligibleForSpecialSolution("confirm_source_defeated") && hasSpecialCurseProof(player, "confirm_source_defeated"), "shop.church.special.source_defeated");

        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une solution spéciale.");
        if (choice <= 0 || choice > 6)
        {
            return;
        }

        std::string solutionId;
        std::string title;
        std::string voucherId;
        std::string voucherName;
        int cost = 0;
        std::vector<std::string> lines;

        if (choice == 1)
        {
            solutionId = "destroy_source_object";
            title = "OBJET SOURCE DÉTRUIT";
            voucherId = "sanctuary_wax_seal";
            voucherName = "Sceau de cire sanctuaire";
            cost = 120;
            lines.push_back("Sœur Maëlys ne frappe pas la trace : elle isole ce à quoi elle était accrochée.");
        }
        else if (choice == 2)
        {
            solutionId = "break_oath";
            title = "SERMENT BRISÉ";
            voucherId = "exorcist_note";
            voucherName = "Note d'exorciste";
            cost = 140;
            lines.push_back("Père Orwan demande une vérité, pas une incantation : le serment doit perdre son droit de serrer.");
        }
        else if (choice == 3)
        {
            solutionId = "read_counter_legend";
            title = "CONTRE-LÉGENDE LUE";
            voucherId = "exorcist_note";
            voucherName = "Note d'exorciste";
            cost = 95;
            lines.push_back("Frère Calixte ouvre la version contraire de l'histoire : celle qui explique comment elle se termine.");
        }
        else if (choice == 4)
        {
            solutionId = "seal_source";
            title = "SOURCE SCELLÉE";
            voucherId = "sanctuary_wax_seal";
            voucherName = "Sceau de cire sanctuaire";
            cost = 160;
            lines.push_back("L'église ne prétend pas vaincre le boss à ta place : elle ferme seulement la trace de seuil déjà comprise.");
        }
        else if (choice == 5)
        {
            solutionId = "break_pact";
            title = "PACTE ROMPU";
            voucherId = "white_bone_chalk";
            voucherName = "Craie d'os blanc";
            cost = 145;
            lines.push_back("Sœur Maëlys ne juge pas le choix : elle force seulement le pacte à nommer son vrai prix.");
        }
        else
        {
            solutionId = "confirm_source_defeated";
            title = "SOURCE VAINCUE CONFIRMÉE";
            voucherId = "exorcist_note";
            voucherName = "Note d'exorciste";
            cost = 110;
            lines.push_back("Père Orwan vérifie la victoire : la source a été affrontée, la trace n'a plus le droit de prétendre qu'elle attend encore.");
        }

        if (!player.hasCurseEligibleForSpecialSolution(solutionId))
        {
            lines.push_back("Aucune trace totalement diagnostiquée ne correspond encore à cette solution.");
            lines.push_back("Il faut d'abord atteindre le diagnostic total 3/3 sur la malédiction concernée.");
            showShopResult("SOLUTION SANS CIBLE", "shop.church.special.no_eligible_curse", lines);
            return;
        }

        const SpecialCurseProof proof = getSpecialCurseProof(solutionId);
        if (!proof.materialId.empty() && player.getInventory().countMaterialById(proof.materialId) <= 0)
        {
            lines.push_back("Preuve manquante : " + proof.materialName + ".");
            lines.push_back("Père Orwan refuse : sans preuve concrète, on ferait juste semblant d'avoir compris la condition spéciale.");
            showShopResult("SOLUTION REFUSÉE", "shop.church.special.missing_proof", lines);
            return;
        }

        if (!payServiceWithVoucherOrGold(player, voucherId, voucherName, cost, lines))
        {
            showShopResult("SOLUTION REFUSÉE", "shop.church.special.failed_cost", lines);
            return;
        }

        if (!proof.materialId.empty())
        {
            player.getInventory().removeMaterialQuantityById(proof.materialId, 1);
            lines.push_back("Preuve consommée : " + proof.materialName + ".");
        }

        const int removed = player.resolveSpecialCurseSolution(solutionId);
        if (removed <= 0)
        {
            lines.push_back("Aucune trace totalement diagnostiquée ne correspond à cette solution.");
            lines.push_back("Il faut atteindre le niveau 3/3 sur la malédiction concernée avant de tenter ce type d'acte.");
            showShopResult("SOLUTION SANS EFFET", "shop.church.special.no_target", lines);
            return;
        }

        lines.push_back("Trace(s) rompue(s) : " + std::to_string(removed) + ".");
        lines.push_back("La condition spéciale était la vraie clé : l'église n'a pas soigné au hasard, elle a retiré le point d'accroche.");
        const SpecialCurseProof outcome = getSpecialCurseOutcome(solutionId);
        if (!outcome.materialId.empty())
        {
            player.getInventory().addMaterial(MaterialCatalog::createById(outcome.materialId, 1));
            lines.push_back("Suite visible obtenue : " + outcome.materialName + " x1.");
        }
        lines.push_back("Note : les objets d'enquête consommés ne sont pas des papiers administratifs, mais les preuves qui permettaient d'agir sans inventer la réponse.");
        showLocalServiceResult(title, "shop.church.special.success", player, lines, 1);
    }


    void openChurchCaseDialogueScenes(Player& player)
    {
        const auto hasProof = [&player](const std::string& id) {
            return player.getInventory().countMaterialById(id) > 0;
        };

        const bool lysaSolved = hasProof("purified_source_ashes");
        const bool lysaObjectKnown = hasProof("identified_source_object") || lysaSolved;
        const bool lysaRoomKnown = hasProof("church_sleep_diagnosis") || lysaObjectKnown;
        const bool ronanSolved = hasProof("broken_oath_record");
        const bool ronanNamed = hasProof("named_oath_testimony") || ronanSolved;
        const bool dormitorySolved = hasProof("closed_counter_legend");
        const bool dormitoryPatternKnown = hasProof("shared_dream_pattern") || hasProof("copied_counter_legend") || dormitorySolved;
        const bool elianSolved = hasProof("released_pact_record");
        const bool elianNamed = hasProof("pact_break_witness") || elianSolved;
        const bool thresholdSolved = hasProof("sealed_source_mark") || hasProof("confirmed_source_silence");
        const bool thresholdSketched = hasProof("sealable_source_sketch") || thresholdSolved;

        MenuScreen screen("PARLER DES CAS D'ÉGLISE", "shop.church.case_dialogues");
        screen.addLine("Ces scènes ne sont pas un registre : ce sont des moments courts pour comprendre les personnes derrière les demandes.");
        screen.addLine("Les dialogues changent si tu as déjà trouvé une preuve ou résolu une trace liée au cas.");
        screen.addOption(0, "Retour", "Revenir aux services d'église.", true, "shop.church.case_dialogues.back");
        screen.addOption(1, "Mira et Lysa", lysaSolved ? "Suite : la chambre est apaisée." : (lysaRoomKnown ? "Suite : l'enquête avance dans la chambre." : "Une mère inquiète, une enfant qui parle avec des mots trop vieux."), true, "shop.church.case_dialogues.lysa");
        screen.addOption(2, "Ronan", ronanSolved ? "Suite : le serment ne serre plus sa gorge." : (ronanNamed ? "Suite : le serment a enfin un nom." : "Un charretier qui s'étouffe dès qu'un ancien serment remonte."), true, "shop.church.case_dialogues.ronan");
        screen.addOption(3, "Ivo, Nelle et Sali", dormitorySolved ? "Suite : la porte du rêve se referme." : (dormitoryPatternKnown ? "Suite : le motif du rêve a été compris." : "Trois apprentis, un même rêve, une porte dessinée sans l'avoir vue."), true, "shop.church.case_dialogues.dormitory");
        screen.addOption(4, "Elian", elianSolved ? "Suite : il peut appeler son choix par son nom." : (elianNamed ? "Suite : la contrepartie a été dite." : "Un aventurier qui appelle dette ce qu'il appelait chance."), true, "shop.church.case_dialogues.elian");
        screen.addOption(5, "Le vieux seuil", thresholdSolved ? "Suite : le lieu ne répond plus de la même façon." : (thresholdSketched ? "Suite : le bon point d'accroche est dessiné." : "Père Orwan explique pourquoi un lieu doit être compris avant d'être scellé."), true, "shop.church.case_dialogues.threshold");

        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "De quel cas veux-tu parler ?");
        if (choice <= 0) return;

        if (choice == 1)
        {
            std::vector<std::string> lines;
            if (lysaSolved)
            {
                lines = {
                    "Mira ne serre plus les mains comme avant. Elle les garde ouvertes, posées sur la couverture de Lysa.",
                    "Lysa dort vraiment cette fois. Pas profondément, pas miraculeusement, mais avec sa propre respiration.",
                    "Sœur Maëlys murmure : « Voilà pourquoi on a cherché l'objet avant de toucher l'enfant. »",
                    "Suite visible : le cas n'est plus une fiche d'enquête, c'est une famille qui respire mieux."
                };
            }
            else if (lysaObjectKnown)
            {
                lines = {
                    "Mira montre la petite boîte retrouvée sous le lit. Elle a l'air banale, et c'est justement ce qui la rend dangereuse.",
                    "Maëlys refuse de la briser dans la panique : « Une source identifiée doit être détruite dans un cercle sûr. »",
                    "Suite possible : utiliser la solution spéciale 'Détruire l'objet source'."
                };
            }
            else if (lysaRoomKnown)
            {
                lines = {
                    "La chambre de Lysa est devenue plus importante que Lysa elle-même dans l'enquête.",
                    "Mira hésite : « Donc... ce n'est pas ma fille qui est maudite ? »",
                    "Sœur Maëlys répond : « Pas forcément. Un patient peut seulement être l'endroit où la trace parle. »"
                };
            }
            else
            {
                lines = {
                    "Mira garde les mains jointes pour ne pas trembler : « Elle dort, mais ce n'est pas sa voix. Je connais ma fille. »",
                    "Sœur Maëlys répond doucement : « Alors on commencera par écouter la chambre. Pas par accuser l'enfant. »",
                    "Lysa, à moitié réveillée, demande seulement si quelqu'un a déplacé la petite boîte sous son lit.",
                    "Indice clair : le problème peut être accroché à un objet banal, pas forcément au patient."
                };
            }
            showShopResult("MIRA ET LYSA", "shop.church.case_dialogues.lysa.result", lines);
            return;
        }

        if (choice == 2)
        {
            std::vector<std::string> lines;
            if (ronanSolved)
            {
                lines = {
                    "Ronan parle plus bas qu'avant, mais il finit ses phrases.",
                    "Père Orwan ne sourit pas trop vite : « Un serment brisé laisse parfois de la honte, mais plus de corde. »",
                    "Ronan souffle enfin le nom qu'il évitait, sans s'étouffer.",
                    "Suite visible : le serment n'est plus accroché à sa gorge."
                };
            }
            else if (ronanNamed)
            {
                lines = {
                    "Ronan n'arrive plus à prétendre que rien n'existe. Le nom du serment est sur la table.",
                    "Père Orwan pose le témoignage devant lui : « Maintenant, on sait quoi briser. Pas avant. »",
                    "Suite possible : utiliser la solution spéciale 'Briser un serment'."
                };
            }
            else
            {
                lines = {
                    "Ronan rit trop fort : « Je n'ai rien promis à personne. »",
                    "Son rire se casse aussitôt, comme si une corde invisible serrait sa gorge.",
                    "Père Orwan pose un sceau sur la table : « Ne mens pas pour être courageux. Donne un nom à ce qui t'étrangle. »",
                    "Indice clair : un serment maudit demande une vérité précise, pas un exorcisme lancé au hasard."
                };
            }
            showShopResult("RONAN", "shop.church.case_dialogues.ronan.result", lines);
            return;
        }

        if (choice == 3)
        {
            std::vector<std::string> lines;
            if (dormitorySolved)
            {
                lines = {
                    "Ivo dessine encore la porte, mais cette fois il ajoute une serrure fermée.",
                    "Nelle rit nerveusement : « Dans mon rêve, la cloche sonnait plus loin. »",
                    "Frère Calixte range la contre-légende : « Une histoire refermée peut rester dans les archives. Pas dans les enfants. »",
                    "Suite visible : le dortoir n'est pas guéri par force, il a reçu la bonne fin de l'histoire."
                };
            }
            else if (dormitoryPatternKnown)
            {
                lines = {
                    "Les trois enfants décrivent maintenant la même porte sans se couper la parole.",
                    "Calixte tient le motif comme une clé de recherche : « On ne soigne pas un rêve partagé comme une fièvre. On cherche l'histoire qui l'a commencé. »",
                    "Suite possible : trouver ou lire une contre-légende."
                };
            }
            else
            {
                lines = {
                    "Ivo dessine une porte noire. Nelle ajoute une poignée en forme de lune. Sali complète sans réfléchir : « Et derrière, il y a une cloche. »",
                    "Frère Calixte pâlit : « Trois enfants n'inventent pas la même erreur avec les mêmes détails. »",
                    "Il demande de comparer les récits avant d'ouvrir les archives : une légende se referme avec sa contre-légende.",
                    "Indice clair : les cauchemars partagés doivent être compris comme une histoire, pas comme une maladie isolée."
                };
            }
            showShopResult("LE DORTOIR", "shop.church.case_dialogues.dormitory.result", lines);
            return;
        }

        if (choice == 4)
        {
            std::vector<std::string> lines;
            if (elianSolved)
            {
                lines = {
                    "Elian ne dit plus que c'était seulement de la chance.",
                    "Sœur Maëlys lui laisse le temps de répondre : « Tu as choisi. Tu as payé. Maintenant, tu peux repartir sans mentir à ton propre choix. »",
                    "Il garde la tête basse, mais ses pas ne résonnent plus comme une dette.",
                    "Suite visible : un pacte volontaire peut être rompu, mais pas effacé de l'histoire du personnage."
                };
            }
            else if (elianNamed)
            {
                lines = {
                    "Elian a enfin nommé la contrepartie. Ce n'était pas un mot héroïque, seulement un prix.",
                    "Maëlys trace une ligne de craie : « Maintenant que le pacte a un nom, il peut perdre sa prise. »",
                    "Suite possible : utiliser la solution spéciale 'Rompre un pacte volontaire'."
                };
            }
            else
            {
                lines = {
                    "Elian regarde ses bottes : « J'avais besoin d'un coup de chance. J'ai dit oui. C'est tout. »",
                    "Sœur Maëlys ne le juge pas : « Un pacte ne se rompt pas en niant qu'il a aidé. Il faut nommer ce qu'il réclame. »",
                    "La craie d'os blanc sert à garder la parole ouverte pendant que la dette tente de se refermer.",
                    "Indice clair : un pacte volontaire vient d'un choix du joueur ou d'un PNJ, donc la rupture demande d'assumer la contrepartie."
                };
            }
            showShopResult("ELIAN", "shop.church.case_dialogues.elian.result", lines);
            return;
        }

        if (choice == 5)
        {
            std::vector<std::string> lines;
            if (thresholdSolved)
            {
                lines = {
                    "Père Orwan ne déplie plus le plan : il le pose fermé sur la table.",
                    "« Le seuil existe encore. Un lieu ne disparaît pas parce qu'on le bénit. Mais il ne répond plus à ton passage. »",
                    "Frère Calixte ajoute une note discrète dans les archives, sans appeler ça une victoire.",
                    "Suite visible : le lieu est suivi comme un endroit scellé ou confirmé, pas comme une menace oubliée."
                };
            }
            else if (thresholdSketched)
            {
                lines = {
                    "Le croquis montre le bon point d'accroche : une fissure, pas toute la porte.",
                    "Père Orwan insiste : « On ne ferme pas un lieu entier quand une seule couture saigne. »",
                    "Suite possible : sceller la source ou confirmer qu'elle a été vaincue."
                };
            }
            else
            {
                lines = {
                    "Père Orwan déplie un vieux plan : « Sceller un seuil sans le dessiner, c'est fermer une porte en laissant la clé au monstre. »",
                    "Frère Calixte ajoute que certains lieux ne sont pas maléfiques : ils répètent seulement un passage resté ouvert trop longtemps.",
                    "Le croquis sert donc à viser le bon point d'accroche, pas à inventer une faiblesse gratuite.",
                    "Indice clair : les solutions de source demandent exploration, preuve et diagnostic total."
                };
            }
            showShopResult("LE VIEUX SEUIL", "shop.church.case_dialogues.threshold.result", lines);
        }
    }


    void runFullChurchExorcism(Player& player)
    {
        std::vector<std::string> lines;
        if (!player.getInventory().spendGold(180))
        {
            showShopResult("EXORCISME COMPLET REFUSÉ", "shop.church.full_exorcism.failed_cost", {"Paiement refusé : il faut " + Money::formatGoldWithRaw(180) + "."});
            return;
        }
        lines.push_back("Coût payé : " + Money::formatGoldWithRaw(180) + ".");
        int acted = 0;
        int removed = 0;
        const std::vector<PlayerCurse> curses = player.getActiveCurses();
        for (const PlayerCurse& curse : curses)
        {
            if (!curse.removableByChurch || curse.bossIdRequiredToBreak > 0 || curse.diagnosisLevel < 1)
            {
                continue;
            }
            ++acted;
            if (player.advanceChurchExorcism(curse.id) && !player.hasActiveCurse(curse.id))
            {
                ++removed;
            }
        }
        if (acted <= 0)
        {
            lines.push_back("Aucune malédiction ne peut être retirée : il faut au moins un niveau 1, et les verrous de boss ne cèdent pas ici.");
        }
        else
        {
            lines.push_back("Exorcisme complet : " + std::to_string(acted) + " trace(s) retirable(s) traitée(s).");
            lines.push_back("Retirées totalement pendant ce passage : " + std::to_string(removed) + ". Les rites longs peuvent demander de revenir.");
        }
        showLocalServiceResult("EXORCISME COMPLET", "shop.church.full_exorcism.result", player, lines, 2);
    }


    struct EnchantmentOffer
    {
        std::string id;
        std::string label;
        std::string effectLabel;
        int price = 0;
        std::string materialId;
        std::string materialName;
        int materialQuantity = 0;
        std::string description;
        int riskModifier = 0;
    };


}

void ChurchServiceMenu::open(Player& player)
{
    bool stay = true;

    while (stay)
    {
        MenuScreen screen("ÉGLISE ET EXORCISME", "shop.church.services");
        screen.addLine("Sœur Maëlys l'exorciste diagnostique les traces. Père Orwan bénit les routes. Frère Calixte écoute les prières laissées aux cierges.");
        screen.addLine("Horaires : matin, midi, après-midi, soir. Fermée la nuit.");
        screen.addLine("Règle : une malédiction inconnue reste affichée ????? et ne peut pas être exorcisée avant le niveau 1.");
        screen.addLine("Chaque diagnostic peut échouer : 10% de chance de lecture inutilisable.");
        screen.addLine("Temps : " + worldTimeLineForPlayer(player));
        screen.addLine("Argent : " + Money::formatCurrencyOverviewFromCopper(player.getInventory().getTotalCopper()));
        screen.addLine("Traces actives : " + std::to_string(player.getActiveCurseCount()) + ".");
        screen.addLine(serviceCostLine(player, "sanctuary_candle", "Cierge de veille", 32));
        screen.addLine(serviceCostLine(player, "exorcism_incense", "Encens d'exorcisme", 86));
        screen.addLine(serviceCostLine(player, "holy_water_vial", "Fiole d'eau bénite", 58));
        screen.addOption(0, "Retour", "Revenir au comptoir de l'église.", true, "shop.church.back");
        screen.addOption(1, "Voir le statut des malédictions", "Affiche ????? si la trace n'est pas encore diagnostiquée.", true, "shop.church.status");
        screen.addOption(2, "Diagnostic ciblé par symptôme", "Choisir une catégorie vague : santé, attaque, mana, précision, etc.", player.getActiveCurseCount() > 0, "shop.church.diagnosis.targeted");
        screen.addOption(3, "Diagnostic général", "Plus cher, mais révèle plus facilement une première piste globale.", player.getActiveCurseCount() > 0, "shop.church.diagnosis.general");
        screen.addOption(4, "Diagnostic approfondi", "Niveau 2/3 : demande une trace déjà connue niveau 1.", player.getActiveCurseCount() > 0, "shop.church.diagnosis.deep");
        screen.addOption(5, "Diagnostic total", "Niveau 3/3 : révèle la lecture complète si le niveau 2 est déjà atteint.", player.getActiveCurseCount() > 0, "shop.church.diagnosis.total");
        screen.addOption(6, "Exorcisme complet", "Tente un passage sur toutes les traces connues niveau 1 et retirable par l'église.", player.getActiveCurseCount() > 0, "shop.church.full_exorcism");
        screen.addOption(7, "Bénédiction de route", "Père Orwan prépare une note de bénédiction. Ne retire pas une vraie malédiction.", true, "shop.church.route_blessing");
        screen.addOption(8, "Rite d'apaisement court", "Rite léger : donne une preuve sacrée locale, consomme 1 segment.", true, "shop.church.blessing");
        screen.addOption(9, "Prières et demandes inquiétantes", "Prendre une demande d'église liée à un habitant, un serment ou un cauchemar partagé.", true, "shop.church.troubled_requests");
        screen.addOption(10, "Solutions spéciales", "Objet à détruire, serment à briser, contre-légende, source à sceller.", player.getActiveCurseCount() > 0, "shop.church.special_solution");
        screen.addOption(11, "Lire les légendes de malédiction", "Bibliothèque/archives : donne du contexte sans révéler de chiffres.", true, "shop.church.curse_legends");
        screen.addOption(12, "Parler des cas d'église", "Scènes courtes avec Mira/Lysa, Ronan, le dortoir, Elian ou le vieux seuil.", true, "shop.church.case_dialogues");
        screen.addOption(13, "Prêter serment", "Fondation : serments acceptés sous conditions, futurs bonus forts avec contraintes et rupture à l'église.", true, "shop.church.oath");

        const std::vector<PlayerCurse>& curses = player.getActiveCurses();
        for (std::size_t i = 0; i < curses.size(); ++i)
        {
            const PlayerCurse& curse = curses[i];
            const bool canExorciseHere = curse.removableByChurch && curse.bossIdRequiredToBreak <= 0 && curse.diagnosisLevel >= 1;
            screen.addOption(
                static_cast<int>(20 + i),
                "Exorciser : " + curseKnownName(curse),
                churchCurseDetailText(curse),
                canExorciseHere,
                "shop.church.exorcise." + std::to_string(i)
            );
        }

        Console::clear();
        const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un service d'église, ou 0 pour revenir.");

        if (choice == 0)
        {
            stay = false;
            continue;
        }

        if (choice == 1)
        {
            std::vector<std::string> lines;
            addChurchDiagnosisLines(player, lines);
            showShopResult("STATUT DES MALÉDICTIONS", "shop.church.status", lines);
            continue;
        }
        if (choice == 2) { runTargetedChurchDiagnosis(player); continue; }
        if (choice == 3) { runGeneralChurchDiagnosis(player); continue; }
        if (choice == 4) { runDeepChurchDiagnosis(player); continue; }
        if (choice == 5) { runTotalChurchDiagnosis(player); continue; }
        if (choice == 6) { runFullChurchExorcism(player); continue; }
        if (choice == 9) { openChurchTroubledPrayerRequests(player); continue; }
        if (choice == 10) { runSpecialCurseSolution(player); continue; }
        if (choice == 11) { showCurseLegendArchive(); continue; }
        if (choice == 12) { openChurchCaseDialogueScenes(player); continue; }
        if (choice == 13)
        {
            bool oathStay = true;
            while (oathStay)
            {
                const bool shieldKnown = player.isPassiveSkillUnlocked("church_oath_shield");
                const bool bloodKnown = player.isPassiveSkillUnlocked("church_oath_blood");
                const bool hunterKnown = player.isPassiveSkillUnlocked("church_oath_hunter");
                const bool kingKnown = player.isPassiveSkillUnlocked("church_oath_king");
                const bool flameKnown = player.isPassiveSkillUnlocked("church_oath_guarded_flame");
                const bool shadowKnown = player.isPassiveSkillUnlocked("church_oath_shadow");
                const bool pilgrimKnown = player.isPassiveSkillUnlocked("church_oath_pilgrim");
                const bool memoryKnown = player.isPassiveSkillUnlocked("church_oath_memory");
                const bool silenceKnown = player.isPassiveSkillUnlocked("church_oath_silence");
                const bool skyKnown = player.isPassiveSkillUnlocked("church_oath_open_sky");
                const bool rootsKnown = player.isPassiveSkillUnlocked("church_oath_roots");
                const bool mirrorKnown = player.isPassiveSkillUnlocked("church_oath_broken_mirror");
                const bool witnessKnown = player.isPassiveSkillUnlocked("church_oath_witness");
                const bool scarsKnown = player.isPassiveSkillUnlocked("church_oath_scars");
                const bool legacyKnown = player.isPassiveSkillUnlocked("church_oath_legacy");
                const bool forgeKnown = player.isPassiveSkillUnlocked("church_oath_bound_forge");
                const bool bondsKnown = player.isPassiveSkillUnlocked("church_oath_bonds");
                const bool rivalsKnown = player.isPassiveSkillUnlocked("church_oath_rivals");
                const bool fateKnown = player.isPassiveSkillUnlocked("church_oath_unstable_fate");
                const int enemyKills = player.getCanonicalJournalCategoryTotal("ennemis_tues");
                const int pnjServed = player.getCanonicalJournalCategoryTotal("pnj_servis");
                const int questsDone = player.getCanonicalJournalCategoryTotal("quetes_terminees");
                const int observedThreats = player.getCanonicalJournalCategoryTotal("observations_combat");
                const int oathCount = (shieldKnown ? 1 : 0) + (bloodKnown ? 1 : 0) + (hunterKnown ? 1 : 0) + (kingKnown ? 1 : 0)
                    + (flameKnown ? 1 : 0) + (shadowKnown ? 1 : 0) + (pilgrimKnown ? 1 : 0) + (memoryKnown ? 1 : 0) + (silenceKnown ? 1 : 0)
                    + (skyKnown ? 1 : 0) + (rootsKnown ? 1 : 0) + (mirrorKnown ? 1 : 0)
                    + (witnessKnown ? 1 : 0) + (scarsKnown ? 1 : 0) + (legacyKnown ? 1 : 0)
                    + (forgeKnown ? 1 : 0) + (bondsKnown ? 1 : 0) + (rivalsKnown ? 1 : 0) + (fateKnown ? 1 : 0);

                MenuScreen oathScreen("SERMENTS D'ÉGLISE", "shop.church.oath.menu");
                oathScreen.addLine("Père Orwan refuse les serments gratuits : une promesse doit être entendue, méritée, puis portée comme un vrai statut.");
                oathScreen.addLine("Les serments deviennent des passifs/statuts forts mais contraignants. La rupture reste prévue à l'église, avec prix et trace.");
                oathScreen.addLine("Attention : plusieurs serments pourront se contredire plus tard. L'église note déjà les promesses cumulées.");
                oathScreen.addBackOption();
                oathScreen.addOption(1, "Serment du Bouclier" + std::string(shieldKnown ? " [déjà prêté]" : ""), "Condition : niveau 3+ ou vraie habitude d'armure. Bonus futur : protection/alliés, prix moral si tu abandonnes la ligne.", player.getLevel() >= 3 || player.hasPassiveSkill("armor_habit") || player.hasPassiveSkill("steady_guard"), "shop.church.oath.shield");
                oathScreen.addOption(2, "Serment du Sang" + std::string(bloodKnown ? " [déjà prêté]" : ""), "Condition : niveau 5+ ou maîtrise/pacte sanguin déjà approché. Bonus futur : dégâts/élan, prix sur soins ou sécurité.", player.getLevel() >= 5 || player.hasPassiveSkill("blood_pact_mastery") || player.hasPassiveSkill("scar_tissue"), "shop.church.oath.blood");
                oathScreen.addOption(3, "Serment du Chasseur" + std::string(hunterKnown ? " [déjà prêté]" : ""), "Condition : au moins 5 ennemis tués ou vraie lecture des familles. Bonus futur : piste/familles ennemies, prix si tu frappes sans comprendre.", enemyKills >= 5 || player.hasPassiveSkill("bestiary_family_reader") || player.hasPassiveSkill("ranger_eye"), "shop.church.oath.hunter");
                oathScreen.addOption(4, "Serment du Roi" + std::string(kingKnown ? " [déjà prêté]" : ""), "Condition : avoir aidé des PNJ ou prouvé une présence de meneur. Bonus futur : alliés/ordres, prix si tu fuis tes responsabilités.", pnjServed >= 2 || player.hasPassiveSkill("battle_order_mastery") || player.hasPassiveSkill("war_cry_caller"), "shop.church.oath.king");
                oathScreen.addOption(5, "Serment de la Flamme gardée" + std::string(flameKnown ? " [déjà prêté]" : ""), "Condition : niveau 7+ ou vraie maîtrise du feu/élémentaire. Bonus futur : chaleur, courage, protection contre brûlure ; prix si tu consumes sans protéger.", player.getLevel() >= 7 || player.hasPassiveSkill("elemental_blade_mastery") || player.hasPassiveSkill("minor_fire_resistance") || player.hasPassiveSkill("infernal_fire_resistance"), "shop.church.oath.guarded_flame");
                oathScreen.addOption(6, "Serment des Ombres franches" + std::string(shadowKnown ? " [déjà prêté]" : ""), "Condition : niveau 8+ ou vraie habitude de ruse/déplacement. Bonus futur : discrétion, esquive, angle ; prix si tu trahis la parole donnée.", player.getLevel() >= 8 || player.hasPassiveSkill("shadow_stepper") || player.hasPassiveSkill("rogue_feinter") || player.hasPassiveSkill("trick_image_mastery"), "shop.church.oath.shadow");
                oathScreen.addOption(7, "Serment du Pèlerin" + std::string(pilgrimKnown ? " [déjà prêté]" : ""), "Condition : avoir voyagé, être inscrit localement ou niveau 4+. Bonus futur : route, fatigue, villages ; prix si tu refuses toute aide de passage.", player.getWorldDaysElapsed() >= 2 || player.isRegisteredAtCurrentCityGuild() || player.getLevel() >= 4, "shop.church.oath.pilgrim");
                oathScreen.addOption(8, "Serment de Mémoire" + std::string(memoryKnown ? " [déjà prêté]" : ""), "Condition : niveau 6+ ou quêtes/observations suffisantes. Bonus futur : traces, rumeurs, héritage moral ; prix si tu mens sur ce qui a été vu.", player.getLevel() >= 6 || questsDone >= 2 || observedThreats >= 3, "shop.church.oath.memory");
                oathScreen.addOption(9, "Serment du Silence" + std::string(silenceKnown ? " [déjà prêté]" : ""), "Condition : niveau 10+ ou maîtrise de lecture/ruse. Bonus futur : anti-panique, anti-illusion, concentration ; prix si tu brises le calme pour provoquer inutilement.", player.getLevel() >= 10 || player.hasPassiveSkill("threat_reader") || player.hasPassiveSkill("body_reader") || player.hasPassiveSkill("trick_image_mastery"), "shop.church.oath.silence");
                oathScreen.addOption(10, "Serment du Ciel ouvert" + std::string(skyKnown ? " [déjà prêté]" : ""), "Condition : niveau 6+, vraie habitude de tir/allonge ou sang des hauteurs. Bonus : mieux gérer Vol ; prix si tu ignores le sol et les alliés.", player.getLevel() >= 6 || player.hasPassiveSkill("ranger_eye") || player.hasPassiveSkill("semi_bird_open_sky") || player.getBowKillProgress() >= 4 || player.getSpearKillProgress() >= 4, "shop.church.oath.open_sky");
                oathScreen.addOption(11, "Serment des Racines" + std::string(rootsKnown ? " [déjà prêté]" : ""), "Condition : niveau 6+, lecture de terrain ou route prudente. Bonus : contre-entrave ; prix si tu piétines les lieux traversés.", player.getLevel() >= 6 || player.hasPassiveSkill("terrain_reader") || player.hasPassiveSkill("cautious_pathing") || player.hasPassiveSkill("threat_route_planner"), "shop.church.oath.roots");
                oathScreen.addOption(12, "Serment du Miroir brisé" + std::string(mirrorKnown ? " [déjà prêté]" : ""), "Condition : niveau 9+ ou vraie expérience des illusions. Bonus : lire les faux reflets ; prix si tu refuses la vérité vue.", player.getLevel() >= 9 || player.hasPassiveSkill("trick_image_mastery") || player.hasPassiveSkill("body_reader") || player.hasPassiveSkill("semi_fox_cunning"), "shop.church.oath.broken_mirror");
                oathScreen.addOption(13, "Serment du Témoin" + std::string(witnessKnown ? " [déjà prêté]" : ""), "Condition : quêtes, observations ou PNJ servis. Bonus : mémoire logique, rumeurs vues, contre-lecture ; prix si tu affirmes sans témoin.", questsDone >= 1 || observedThreats >= 2 || pnjServed >= 1 || player.hasPassiveSkill("church_oath_memory"), "shop.church.oath.witness");
                oathScreen.addOption(14, "Serment des Cicatrices" + std::string(scarsKnown ? " [déjà prêté]" : ""), "Condition : niveau 8+, vraie survie ou trace déjà portée. Bonus : douleur utile, tenue sous pression ; prix si tu cherches la blessure gratuitement.", player.getLevel() >= 8 || player.hasPassiveSkill("scar_tissue") || player.hasPassiveSkill("church_oath_blood") || player.hasPassiveSkill("church_oath_broken_trace"), "shop.church.oath.scars");
                oathScreen.addOption(15, "Serment de l'Héritage" + std::string(legacyKnown ? " [déjà prêté]" : ""), "Condition : niveau 12+, mémoire ou trace de rupture. Bonus futur : Mortel/Léthal, objets avec mémoire, tombes ; prix si tu profanes l'héritage.", player.getLevel() >= 12 || player.hasPassiveSkill("church_oath_memory") || player.hasPassiveSkill("church_oath_broken_trace"), "shop.church.oath.legacy");
                oathScreen.addOption(16, "Serment de la Forge liée" + std::string(forgeKnown ? " [déjà prêté]" : ""), "Condition : niveau 6+, arme entretenue ou build mémorisé. Bonus : objets avec mémoire, arme cohérente, forge ; prix si tu traites l'équipement comme jetable.", player.getLevel() >= 6 || player.hasPassiveSkill("weapon_care_habit") || player.hasPassiveSkill("loadout_memory") || player.hasPassiveSkill("field_maintenance"), "shop.church.oath.bound_forge");
                oathScreen.addOption(17, "Serment des Liens" + std::string(bondsKnown ? " [déjà prêté]" : ""), "Condition : niveau 7+, ordres/recrues ou présence de groupe. Bonus : techniques combinées alliées, loyauté, combat psychologique de groupe ; prix si tu brises les liens.", player.getLevel() >= 7 || player.hasPassiveSkill("battle_order_mastery") || player.hasPassiveSkill("war_cry_caller") || player.getCanonicalJournalCategoryTotal("participation_recrues") >= 3, "shop.church.oath.bonds");
                oathScreen.addOption(18, "Serment des Rivaux" + std::string(rivalsKnown ? " [déjà prêté]" : ""), "Condition : ennemi déjà fui/paniqué, niveau 9+ ou témoin/mémoire. Bonus : traces de rivaux et futurs mini-boss ; prix si tu humilies sans assumer.", player.getLevel() >= 9 || player.getCanonicalJournalCategoryTotal("rivaux_potentiels") >= 1 || player.hasPassiveSkill("church_oath_witness") || player.hasPassiveSkill("church_oath_memory"), "shop.church.oath.rivals");
                oathScreen.addOption(19, "Serment du Destin instable" + std::string(fateKnown ? " [déjà prêté]" : ""), "Condition : niveau 10+, trace de rupture, mémoire ou cicatrice. Bonus : destin réactif aux actes réels ; prix si tu cherches à forcer l'anomalie.", player.getLevel() >= 10 || player.hasPassiveSkill("church_oath_broken_trace") || player.hasPassiveSkill("church_oath_memory") || player.hasPassiveSkill("church_oath_scars"), "shop.church.oath.unstable_fate");
                oathScreen.addOption(20, "Rompre un contrat", "Rupture réelle : coûte un rite, désactive le serment choisi, ajoute une trace de registre et garde l'événement en mémoire.", oathCount > 0, "shop.church.oath.break");

                const int oathChoice = TerminalInterface::askMenuChoiceFromOptions(oathScreen, "Choisis un serment, ou 0 pour revenir.");
                if (oathChoice == 0)
                {
                    oathStay = false;
                    continue;
                }

                auto acceptOath = [&](const std::string& id, const std::string& name, const std::vector<std::string>& extraLines) {
                    const bool alreadyKnown = player.isPassiveSkillUnlocked(id);
                    const bool alreadyActive = player.hasPassiveSkill(id);
                    if (alreadyKnown)
                    {
                        if (alreadyActive)
                        {
                            showShopResult("SERMENT DÉJÀ ACTIF", "shop.church.oath.already_active", {
                                name + " est déjà inscrit comme contrat actif.",
                                "Un serment ne se cumule pas avec lui-même et ne consomme pas d'emplacement passif."
                            });
                        }
                        else
                        {
                            showShopResult("SERMENT DÉJÀ ROMPU", "shop.church.oath.broken", {
                                name + " porte déjà une rupture dans le registre.",
                                "Il ne peut pas être réactivé gratuitement depuis le menu des compétences ni reprêté comme si rien ne s'était passé.",
                                "Une future voie de réparation/restauration devra demander un vrai prix et des conséquences."
                            });
                        }
                        return;
                    }

                    const bool hadBlood = player.hasPassiveSkill("church_oath_blood");
                    const bool hadShield = player.hasPassiveSkill("church_oath_shield");
                    const bool hadSilence = player.hasPassiveSkill("church_oath_silence");
                    const bool hadShadow = player.hasPassiveSkill("church_oath_shadow");
                    const bool hadMemory = player.hasPassiveSkill("church_oath_memory");
                    const bool hadLegacy = player.hasPassiveSkill("church_oath_legacy");
                    const bool hadSilenceForFate = player.hasPassiveSkill("church_oath_silence");
                    const bool hadRivals = player.hasPassiveSkill("church_oath_rivals");
                    const bool unlocked = player.unlockPassiveSkill(id, name);
                    std::vector<std::string> lines;
                    lines.push_back("Père Orwan ne grave pas une promesse parce qu'elle sonne bien : il cherche une preuve que quelqu'un, quelque part, peut croire à ce serment.");
                    lines.push_back(unlocked ? ("Statut ajouté : " + name + ".") : ("Statut déjà présent : " + name + "."));
                    if ((id == "church_oath_blood" && hadShield) || (id == "church_oath_shield" && hadBlood))
                    {
                        lines.push_back("Contradiction surveillée : Sang et Bouclier peuvent cohabiter, mais l'un réclame le prix du corps quand l'autre réclame la tenue de la ligne.");
                    }
                    if ((id == "church_oath_shadow" && hadSilence) || (id == "church_oath_silence" && hadShadow))
                    {
                        lines.push_back("Contradiction surveillée : Ombres franches et Silence se supportent seulement si la ruse ne devient pas provocation gratuite.");
                    }
                    if ((id == "church_oath_legacy" && !hadMemory) || (id == "church_oath_memory" && hadLegacy))
                    {
                        lines.push_back("Contrat lié : l'héritage sans mémoire est fragile. L'église notera plus tard si l'histoire est portée ou seulement utilisée.");
                    }
                    if ((id == "church_oath_rivals" && !hadMemory && !hadRivals) || (id == "church_oath_memory" && hadRivals))
                    {
                        lines.push_back("Contrat lié : un rival sans témoin devient seulement une vengeance privée. L'église demandera des traces, pas des noms inventés.");
                    }
                    if ((id == "church_oath_unstable_fate" && hadSilenceForFate) || (id == "church_oath_silence" && player.hasPassiveSkill("church_oath_unstable_fate")))
                    {
                        lines.push_back("Contradiction surveillée : Silence veut tenir le calme, Destin instable accepte les oscillations. Les deux pourront cohabiter, mais pas sans tension.");
                    }
                    lines.insert(lines.end(), extraLines.begin(), extraLines.end());
                    lines.push_back("Rupture prévue : revenir à l'église pour rompre proprement le contrat, avec prix, témoin et trace, au lieu d'effacer ça comme une option gratuite.");
                    player.recordCanonicalEvent("serments_eglise", id, name, 1);
                    player.recordHistoricalEvent("oath_sworn", id, "Serment prêté : " + name, false);
                    showShopResult("SERMENT ACCEPTÉ", "shop.church.oath.accepted", lines);
                };

                if (oathChoice == 1)
                {
                    acceptOath("church_oath_shield", "Serment du Bouclier", {
                        "Sens : protéger avant de briller. Les futurs effets devront valoriser garde, alliés et refus d'abandon.",
                        "Prix prévu : la promesse supportera mal les alliés laissés sans couverture."
                    });
                    continue;
                }
                if (oathChoice == 2)
                {
                    acceptOath("church_oath_blood", "Serment du Sang", {
                        "Sens : payer quelque chose de réel pour obtenir un élan réel.",
                        "Prix prévu : soins, sécurité ou stabilité devront compter ; ce ne sera pas juste un bonus de dégâts gratuit."
                    });
                    continue;
                }
                if (oathChoice == 3)
                {
                    acceptOath("church_oath_hunter", "Serment du Chasseur", {
                        "Sens : comprendre la proie, sa famille, ses traces et le terrain avant de réclamer l'avantage.",
                        "Prix prévu : frapper sans lecture ou contre une mauvaise cible pourra rendre le serment instable."
                    });
                    continue;
                }
                if (oathChoice == 4)
                {
                    acceptOath("church_oath_king", "Serment du Roi", {
                        "Sens : tenir une responsabilité visible. Ce serment doit valoriser ordres, alliés, présence et réputation.",
                        "Prix prévu : fuir trop facilement ou sacrifier les autres devra abîmer la promesse."
                    });
                    continue;
                }
                if (oathChoice == 5)
                {
                    acceptOath("church_oath_guarded_flame", "Serment de la Flamme gardée", {
                        "Sens : garder une chaleur qui protège avant de chercher à brûler plus fort.",
                        "Prix prévu : les futurs abus de feu sans protection pourront fragiliser le serment."
                    });
                    continue;
                }
                if (oathChoice == 6)
                {
                    acceptOath("church_oath_shadow", "Serment des Ombres franches", {
                        "Sens : avancer dans l'ombre sans transformer la discrétion en trahison gratuite.",
                        "Prix prévu : mensonge, vol ou abandon d'allié pourront salir la promesse."
                    });
                    continue;
                }
                if (oathChoice == 7)
                {
                    acceptOath("church_oath_pilgrim", "Serment du Pèlerin", {
                        "Sens : respecter les routes, les relais, les villages et les témoins qui rendent un voyage possible.",
                        "Prix prévu : ignorer systématiquement les lieux traversés pourra rendre la promesse creuse."
                    });
                    continue;
                }
                if (oathChoice == 8)
                {
                    acceptOath("church_oath_memory", "Serment de Mémoire", {
                        "Sens : ne pas laisser les morts, les témoins, les objets et les erreurs disparaître du récit.",
                        "Prix prévu : mentir sur une trace ou effacer une responsabilité devra laisser une marque."
                    });
                    continue;
                }
                if (oathChoice == 9)
                {
                    acceptOath("church_oath_silence", "Serment du Silence", {
                        "Sens : garder assez de calme pour lire peur, illusions, panique et provocations.",
                        "Prix prévu : rompre le calme par orgueil pourra affaiblir la concentration promise."
                    });
                    continue;
                }
                if (oathChoice == 10)
                {
                    acceptOath("church_oath_open_sky", "Serment du Ciel ouvert", {
                        "Sens : ne pas paniquer quand l'ennemi quitte le sol. Le ciel s'affronte avec lecture, allonge, tir ou patience.",
                        "Effet actuel : une arme courte peut parfois trouver un angle contre Vol, mais jamais gratuitement."
                    });
                    continue;
                }
                if (oathChoice == 11)
                {
                    acceptOath("church_oath_roots", "Serment des Racines", {
                        "Sens : sentir les appuis, les fils et les racines avant qu'ils ne volent tout le tour.",
                        "Effet actuel : une entrave peut parfois être arrachée en début de tour au prix d'un effort visible."
                    });
                    continue;
                }
                if (oathChoice == 12)
                {
                    acceptOath("church_oath_broken_mirror", "Serment du Miroir brisé", {
                        "Sens : casser le faux reflet sans prétendre recevoir une vérité divine.",
                        "Effet actuel : réduit le risque de frapper le mauvais reflet si des indices existent."
                    });
                    continue;
                }
                if (oathChoice == 13)
                {
                    acceptOath("church_oath_witness", "Serment du Témoin", {
                        "Sens : ne croire qu'une trace parce qu'elle a une source : témoin, rumeur, registre, bestiaire ou observation réelle.",
                        "Effet actuel : aide légèrement les contre-lectures et les coups portés sur une faille réellement observée."
                    });
                    continue;
                }
                if (oathChoice == 14)
                {
                    acceptOath("church_oath_scars", "Serment des Cicatrices", {
                        "Sens : transformer une blessure vécue en tenue, pas chercher la douleur pour faire joli.",
                        "Effet actuel : sous pression, la cicatrice peut soutenir un impact ou une garde courte."
                    });
                    continue;
                }
                if (oathChoice == 15)
                {
                    acceptOath("church_oath_legacy", "Serment de l'Héritage", {
                        "Sens : préparer les systèmes Mortel/Léthal, les tombes, les objets avec mémoire et ce qui reste après une vraie perte.",
                        "Effet actuel : petite aide rare quand une action prolonge une trace déjà inscrite."
                    });
                    continue;
                }
                if (oathChoice == 16)
                {
                    acceptOath("church_oath_bound_forge", "Serment de la Forge liée", {
                        "Sens : lier l'objet à ce qu'il a vraiment vécu : coups portés, réparations, boss affrontés et mains qui l'ont porté.",
                        "Effet actuel : une arme cohérente avec la classe peut laisser une trace de mémoire d'objet, sans devenir légendaire gratuitement."
                    });
                    continue;
                }
                if (oathChoice == 17)
                {
                    acceptOath("church_oath_bonds", "Serment des Liens", {
                        "Sens : valoriser le groupe, les recrues, les ordres et les techniques combinées sans transformer les alliés faibles en vétérans instantanés.",
                        "Effet actuel : la présence de groupe peut soutenir une pression courte et laisser une trace pour les futurs combos alliés."
                    });
                    continue;
                }
                if (oathChoice == 18)
                {
                    acceptOath("church_oath_rivals", "Serment des Rivaux", {
                        "Sens : si un ennemi survit à une fuite, une humiliation ou une défaite interrompue, il peut porter une histoire au lieu de disparaître dans une statistique.",
                        "Effet actuel : les fuites, paniques et compétences ennemies marquantes laissent plus facilement une trace de rival potentiel."
                    });
                    continue;
                }
                if (oathChoice == 19)
                {
                    acceptOath("church_oath_unstable_fate", "Serment du Destin instable", {
                        "Sens : accepter que certains chemins se déplacent selon les actes réels : serments rompus, cicatrices, rumeurs, objets marqués ou classes en mutation.",
                        "Effet actuel : de rares oscillations peuvent soutenir ou durcir une action quand une trace existe, sans garantir le résultat."
                    });
                    continue;
                }
                if (oathChoice == 20)
                {
                    struct OathBreakOption { int id; std::string skill; std::string name; };
                    std::vector<OathBreakOption> breakOptions;
                    auto addBreak = [&](const std::string& skill, const std::string& name) {
                        if (player.hasPassiveSkill(skill))
                        {
                            breakOptions.push_back({static_cast<int>(breakOptions.size()) + 1, skill, name});
                        }
                    };
                    addBreak("church_oath_shield", "Serment du Bouclier");
                    addBreak("church_oath_blood", "Serment du Sang");
                    addBreak("church_oath_hunter", "Serment du Chasseur");
                    addBreak("church_oath_king", "Serment du Roi");
                    addBreak("church_oath_guarded_flame", "Serment de la Flamme gardée");
                    addBreak("church_oath_shadow", "Serment des Ombres franches");
                    addBreak("church_oath_pilgrim", "Serment du Pèlerin");
                    addBreak("church_oath_memory", "Serment de Mémoire");
                    addBreak("church_oath_silence", "Serment du Silence");
                    addBreak("church_oath_open_sky", "Serment du Ciel ouvert");
                    addBreak("church_oath_roots", "Serment des Racines");
                    addBreak("church_oath_broken_mirror", "Serment du Miroir brisé");
                    addBreak("church_oath_witness", "Serment du Témoin");
                    addBreak("church_oath_scars", "Serment des Cicatrices");
                    addBreak("church_oath_legacy", "Serment de l'Héritage");
                    addBreak("church_oath_bound_forge", "Serment de la Forge liée");
                    addBreak("church_oath_bonds", "Serment des Liens");
                    addBreak("church_oath_rivals", "Serment des Rivaux");
                    addBreak("church_oath_unstable_fate", "Serment du Destin instable");

                    MenuScreen breakScreen("ROMPRE UN CONTRAT", "shop.church.oath.break.menu");
                    breakScreen.addLine("Frère Calixte sort un registre noir : rompre ne supprime pas l'histoire, ça la déplace dans les traces.");
                    breakScreen.addLine("Effet actuel : le serment choisi est désactivé, une rupture est enregistrée et les futurs PNJ pourront s'en souvenir.");
                    breakScreen.addBackOption();
                    for (const OathBreakOption& option : breakOptions)
                    {
                        const bool active = player.hasPassiveSkill(option.skill);
                        breakScreen.addOption(option.id, option.name + std::string(active ? " [actif]" : " [déjà inactif]"), "Rompre ce contrat au registre de l'église.", true, "shop.church.oath.break.option");
                    }
                    const int breakChoice = TerminalInterface::askMenuChoiceFromOptions(breakScreen, "Choisis le serment à rompre, ou 0 pour revenir.");
                    if (breakChoice == 0)
                    {
                        continue;
                    }
                    if (breakChoice >= 1 && breakChoice <= static_cast<int>(breakOptions.size()))
                    {
                        const OathBreakOption selected = breakOptions[static_cast<std::size_t>(breakChoice - 1)];
                        std::vector<std::string> breakLines;
                        if (!payServiceWithVoucherOrGold(player, "sanctuary_wax_seal", "Sceau de cire sanctuaire", 42, breakLines))
                        {
                            breakLines.push_back("Frère Calixte referme le registre : une rupture propre demande au moins un rite, un témoin ou de quoi payer l'acte.");
                            showShopResult("RUPTURE REFUSÉE", "shop.church.oath.break.refused", breakLines);
                            continue;
                        }
                        player.unlockPassiveSkill("church_oath_broken_trace", "Trace de serment rompu");
                        player.recordCanonicalEvent("serments_rompus", selected.skill, selected.name, 1);
                        player.recordHistoricalEvent("oath_broken", selected.skill, "Rupture de " + selected.name, false);
                        player.recordCanonicalEvent("eglise", "rupture_serment", "Le joueur a rompu un serment d'église", 1);
                        breakLines.push_back("Contrat rompu : " + selected.name + ".");
                        breakLines.push_back("Effet : le contrat cesse d'agir immédiatement. Il ne peut pas être réactivé depuis le loadout passif.");
                        breakLines.push_back("Trace : le registre garde le nom du serment, la date, le prix payé et le fait qu'il n'a pas disparu gratuitement.");
                        breakLines.push_back("Conséquence : la Trace de serment rompu pourra servir aux prêtres, villes, boss, compagnons ou héritages futurs.");
                        showShopResult("SERMENT ROMPU", "shop.church.oath.break.done", breakLines);
                    }
                    continue;
                }
            }
            player.recordCanonicalEvent("eglise", "serment_consulte", "Le joueur a consulté les serments d'église", 1);
            continue;
        }

        if (choice == 7)
        {
            std::vector<std::string> lines;
            if (!payServiceWithVoucherOrGold(player, "sanctuary_candle", "Cierge de veille", 36, lines))
            {
                showShopResult("BÉNÉDICTION REFUSÉE", "shop.church.route_blessing.failed", lines);
                continue;
            }
            player.getInventory().addMaterial(MaterialCatalog::createById("blessing_note", 1));
            lines.push_back("Père Orwan trace un signe court au bas d'un papier et refuse d'en faire une promesse absolue.");
            lines.push_back("Preuve obtenue : Note de bénédiction x1.");
            lines.push_back("Limite : utile pour des routes, quêtes et futurs dialogues, mais cela ne retire pas une vraie malédiction.");
            showLocalServiceResult("BÉNÉDICTION DE ROUTE", "shop.church.route_blessing.success", player, lines, 1);
            continue;
        }

        if (choice == 8)
        {
            std::vector<std::string> lines;
            if (!payServiceWithVoucherOrGold(player, "sanctuary_candle", "Cierge de veille", 28, lines))
            {
                showShopResult("RITE REFUSÉ", "shop.church.blessing.failed", lines);
                continue;
            }

            player.getInventory().addMaterial(MaterialCatalog::createById("sanctuary_wax_seal", 1));
            lines.push_back("Rite d'apaisement : Frère Calixte grave ton nom sur une cire froide, puis la casse avant qu'elle ne colle à ton ombre.");
            lines.push_back("Preuve obtenue : Sceau de cire sanctuaire x1.");
            lines.push_back("Limite : cela ne retire pas les grandes malédictions, mais peut aider des quêtes, PNJ ou rites futurs.");
            showLocalServiceResult("RITE D'APAISEMENT", "shop.church.blessing.success", player, lines, 1);
            continue;
        }

        const int curseIndex = choice - 20;
        if (curseIndex >= 0 && curseIndex < static_cast<int>(curses.size()))
        {
            const PlayerCurse target = curses[static_cast<std::size_t>(curseIndex)];
            std::vector<std::string> lines;
            if (target.diagnosisLevel < 1)
            {
                lines.push_back("Sœur Maëlys refuse : elle ne retire pas une trace affichée ?????.");
                lines.push_back("Il faut au moins un diagnostic niveau 1 avant de tenter un exorcisme.");
                showShopResult("EXORCISME REFUSÉ", "shop.church.exorcise.unknown", lines);
                continue;
            }
            if (!target.removableByChurch || target.bossIdRequiredToBreak > 0)
            {
                lines.push_back("Père Orwan refuse de mentir : cette malédiction ne partira pas par un simple rite d'église.");
                lines.push_back(target.diagnosisLevel >= 3 && !target.removalHint.empty() ? "Condition : " + target.removalHint : "Condition : recherche totale conseillée.");
                showShopResult("EXORCISME IMPOSSIBLE", "shop.church.exorcise.locked", lines);
                continue;
            }

            if (!payServiceWithVoucherOrGold(player, "holy_water_vial", "Fiole d'eau bénite", 58, lines))
            {
                showShopResult("EXORCISME REFUSÉ", "shop.church.exorcise.failed", lines);
                continue;
            }

            const int beforeProgress = target.exorcismProgress;
            if (!player.advanceChurchExorcism(target.id))
            {
                lines.push_back("Le rite n'accroche pas : la trace ne réagit pas comme une malédiction retirable ici.");
                showShopResult("EXORCISME ÉCHOUÉ", "shop.church.exorcise.invalid", lines);
                continue;
            }

            if (!player.hasActiveCurse(target.id))
            {
                lines.push_back("Exorcisme terminé : " + target.name + " ne pèse plus dans les traces actives.");
                lines.push_back("Maëlys rappelle que certaines malédictions rares peuvent revenir si leur source n'est pas traitée.");
                showLocalServiceResult("MALÉDICTION RETIRÉE", "shop.church.exorcise.removed", player, lines, 1);
                continue;
            }

            const PlayerCurse* updated = findActiveCurseById(player, target.id);
            if (updated != nullptr)
            {
                lines.push_back("Exorcisme progressif : " + std::to_string(beforeProgress) + " -> " + std::to_string(updated->exorcismProgress) + "/" + std::to_string(updated->exorcismRequiredVisits) + ".");
                lines.push_back("Important : il faudra revenir un autre jour/passage pour continuer. Le jeu ne bloque pas toute la journée, donc c'est au joueur d'y penser.");
            }
            showLocalServiceResult("EXORCISME EN COURS", "shop.church.exorcise.progress", player, lines, 1);
            continue;
        }
    }
}
