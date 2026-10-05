// EN: City services, local subscriptions, lodging and transport extracted from ShopMenu.
// FR: Services de ville, abonnements locaux, hébergement et transport extraits de ShopMenu.

#include "interface/menu/shop/ShopCityServiceMenu.hpp"
#include "interface/menu/shop/ShopServiceSupport.hpp"

#include "core/Console.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "economy/Money.hpp"
#include "economy/EconomyBalance.hpp"
#include "economy/shop/ShopCatalog.hpp"
#include "economy/shop/ShopTransactionSystem.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/menu/common/PagedMenu.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "item/material/MaterialCatalog.hpp"
#include "item/material/Material.hpp"
#include "quest/Quest.hpp"
#include "world/City.hpp"
#include "world/LocalReputationSystem.hpp"
#include "world/npc/NpcKnowledgeSystem.hpp"
#include "world/npc/NpcInformationPropagationSystem.hpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <string>
#include <utility>
#include <vector>

namespace ShopCityServiceMenu
{
    using ShopServiceSupport::localReputationLineForPlayer;
    using ShopServiceSupport::payServiceWithVoucherOrGold;
    using ShopServiceSupport::showLocalServiceResult;
    using ShopServiceSupport::showShopResult;

    int cityRepairDaysRemaining(const Player& player)
    {
        return player.getInventory().countMaterialById("city_repair_days_marker");
    }

    std::string worldTimeLineForPlayer(const Player& player)
    {
        return player.formatWorldDateTimeLine();
    }

    struct LocalSubscriptionOffer
    {
        std::string id;
        std::string name;
        int price;
        std::string description;
    };

    std::string subscriptionStatusLine(const Player& player, const LocalSubscriptionOffer& offer)
    {
        const int expiresAt = player.getLocalSubscriptionExpiresAtDay(offer.id);
        if (expiresAt < 0)
        {
            return "Abonnement : inactif.";
        }

        std::string line = "Abonnement : actif jusqu'à la fin du jour " + std::to_string(expiresAt + 1) + ".";
        if (player.isLocalSubscriptionCancellationRequested(offer.id))
        {
            line += " Annulation demandée : il reste actif jusqu'à cette date, puis disparaîtra.";
        }
        else
        {
            line += " Renouvellement possible par période de 7 jours.";
        }

        return line;
    }

    bool subscriptionCoversService(const Player& player, const std::string& serviceKind)
    {
        if (serviceKind == "lodging")
        {
            return player.hasActiveLocalSubscription("lodging_modest_weekly")
                || player.hasActiveLocalSubscription("guild_adventurer_standard_weekly")
                || player.hasActiveLocalSubscription("guild_adventurer_silver_weekly");
        }

        if (serviceKind == "stable")
        {
            return player.hasActiveLocalSubscription("stable_relay_weekly")
                || player.hasActiveLocalSubscription("trade_route_weekly");
        }

        if (serviceKind == "transport")
        {
            return player.hasActiveLocalSubscription("trade_route_weekly");
        }

        if (serviceKind == "city")
        {
            return player.hasActiveLocalSubscription("guild_adventurer_standard_weekly")
                || player.hasActiveLocalSubscription("merchant_cotisation_weekly");
        }

        return false;
    }

    std::string serviceCostLine(const Player& player, const std::string& voucherId, const std::string& voucherName, int fallbackPrice)
    {
        const int voucherCount = player.getInventory().countMaterialById(voucherId);
        if (voucherCount > 0)
        {
            return "Coût prévu : " + voucherName + " x1 déjà présent dans l'inventaire.";
        }

        return "Coût prévu : " + Money::formatEconomyUnits(fallbackPrice) + " si aucun bon/ticket n'est présenté.";
    }

    bool payServiceWithSubscriptionVoucherOrGold(
        Player& player,
        const std::string& serviceKind,
        const std::string& voucherId,
        const std::string& voucherName,
        int fallbackPrice,
        std::vector<std::string>& resultLines
    )
    {
        if (subscriptionCoversService(player, serviceKind))
        {
            resultLines.push_back("Abonnement utilisé : aucun bon ni paiement direct n'est consommé pour ce service.");
            return true;
        }

        return payServiceWithVoucherOrGold(player, voucherId, voucherName, fallbackPrice, resultLines);
    }

    void openSingleSubscriptionMenu(Player& player, const LocalSubscriptionOffer& offer)
    {
        bool stay = true;

        while (stay)
        {
            MenuScreen screen("ABONNEMENT", "shop.subscription.single");
            screen.addLine(offer.name);
            screen.addLine(offer.description);
            screen.addLine("Durée : 7 jours. Si tu annules, l'effet reste actif jusqu'à la fin de la période déjà payée.");
            screen.addLine("Prix de période : " + Money::formatEconomyUnits(offer.price) + ".");
            screen.addLine("Temps actuel : " + player.formatWorldDateTimeLine());
            screen.addLine(subscriptionStatusLine(player, offer));
            screen.addOption(0, "Retour", "Revenir aux abonnements.", true, "shop.subscription.back");
            screen.addOption(1, player.hasActiveLocalSubscription(offer.id) ? "Renouveler 7 jours" : "Prendre l'abonnement 7 jours", "Paye ou renouvelle une période de 7 jours à partir d'aujourd'hui.", true, "shop.subscription.activate");
            screen.addOption(2, "Demander l'annulation", "L'abonnement reste actif jusqu'à la date de fin déjà payée.", player.hasActiveLocalSubscription(offer.id), "shop.subscription.cancel");

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une action d'abonnement.");

            if (choice == 0)
            {
                stay = false;
                continue;
            }

            if (choice == 1)
            {
                std::vector<std::string> lines;
                if (!player.getInventory().spendEconomyUnits(offer.price))
                {
                    lines.push_back("Paiement refusé : il manque " + Money::formatEconomyUnits(offer.price) + ".");
                    lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
                    showShopResult("ABONNEMENT REFUSÉ", "shop.subscription.failed", lines);
                    continue;
                }

                player.activateLocalSubscription(offer.id, offer.name, 7, offer.price);
                lines.push_back("Abonnement actif : " + offer.name + ".");
                lines.push_back("Fin de période : fin du jour " + std::to_string(player.getLocalSubscriptionExpiresAtDay(offer.id) + 1) + ".");
                lines.push_back("Annulation possible : l'effet restera jusqu'à la fin de la période déjà payée.");
                showShopResult("ABONNEMENT VALIDÉ", "shop.subscription.success", lines);
            }
            else if (choice == 2)
            {
                std::vector<std::string> lines;
                if (player.requestLocalSubscriptionCancellation(offer.id))
                {
                    lines.push_back("Annulation enregistrée pour : " + offer.name + ".");
                    lines.push_back("L'abonnement reste actif jusqu'à la fin du jour " + std::to_string(player.getLocalSubscriptionExpiresAtDay(offer.id) + 1) + ".");
                    lines.push_back("Aucun remboursement : le comptoir appelle ça une cotisation, évidemment.");
                    showShopResult("ABONNEMENT ANNULÉ", "shop.subscription.cancelled", lines);
                }
                else
                {
                    lines.push_back("Aucun abonnement actif à annuler.");
                    showShopResult("ANNULATION IMPOSSIBLE", "shop.subscription.cancel.failed", lines);
                }
            }
        }
    }

    void openSubscriptionMenu(Player& player, const std::string& title, const std::vector<LocalSubscriptionOffer>& offers)
    {
        bool stay = true;
        while (stay)
        {
            MenuScreen screen(title, "shop.subscription.list");
            screen.addLine("Temps actuel : " + player.formatWorldDateTimeLine());
            screen.addLine("Les abonnements durent 7 jours. Une annulation ne coupe jamais l'effet immédiatement.");
            screen.addOption(0, "Retour", "Revenir au service précédent.", true, "shop.subscription.list.back");

            for (std::size_t i = 0; i < offers.size(); ++i)
            {
                std::string label = offers[i].name + " | " + Money::formatEconomyUnits(offers[i].price);
                if (player.hasActiveLocalSubscription(offers[i].id))
                {
                    label += " | actif jusqu'au jour " + std::to_string(player.getLocalSubscriptionExpiresAtDay(offers[i].id) + 1);
                    if (player.isLocalSubscriptionCancellationRequested(offers[i].id))
                    {
                        label += " | annulation demandée";
                    }
                }
                screen.addOption(static_cast<int>(i + 1), label, offers[i].description, true, "shop.subscription.open." + std::to_string(i + 1));
            }

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis un abonnement, ou 0 pour revenir.");
            if (choice == 0)
            {
                stay = false;
                continue;
            }
            if (choice > 0 && static_cast<std::size_t>(choice) <= offers.size())
            {
                openSingleSubscriptionMenu(player, offers[static_cast<std::size_t>(choice - 1)]);
            }
        }
    }

    void reduceCityRepairDays(Player& player, int days)
    {
        const int owned = player.getInventory().countMaterialById("city_repair_days_marker");
        if (owned <= 0 || days <= 0)
        {
            return;
        }
        player.getInventory().removeMaterialQuantityById("city_repair_days_marker", std::min(owned, days));
        if (player.getInventory().countMaterialById("city_repair_days_marker") <= 0)
        {
            const int notices = player.getInventory().countMaterialById("city_damage_notice");
            if (notices > 0)
            {
                player.getInventory().removeMaterialQuantityById("city_damage_notice", notices);
            }
        }
    }


    constexpr int kRegularCityEventIntervalDays = 7;
    constexpr int kRoyalBonusCityEventChancePercent = 32;

    int safeCityDay(const Player& player)
    {
        return std::max(0, player.getWorldDaysElapsed());
    }

    CityEventOfDay regularCityEventForWeekIndex(int weekIndex)
    {
        const int eventIndex = std::max(0, weekIndex) % 8;
        switch (eventIndex)
        {
            case 0:
                return {"guild_tournament", "Tournoi de guilde", "La place est balisée, les soigneurs restent proches et les parieurs parlent trop fort.", "S'inscrire comme participant encadré"};
            case 1:
                return {"monster_hunt", "Chasse aux monstres locale", "Les pisteurs comparent des traces au lieu de vendre des promesses héroïques.", "Aider à identifier les pistes"};
            case 2:
                return {"merchant_fair", "Foire marchande", "Des étals temporaires prennent toute la rue et les prix changent avec le sourire du vendeur.", "Tenir un comptoir ou négocier"};
            case 3:
                return {"knowledge_day", "Journée du savoir", "L'archiviste sort des cartes, des notes et des vieux avertissements que personne ne lit assez.", "Assister aux explications"};
            case 4:
                return {"mission_exchange", "Bourse aux missions", "La guilde trie les demandes trop petites pour un grand contrat, mais trop urgentes pour attendre.", "Aider au tri des demandes"};
            case 5:
                return {"honor_ceremony", "Cérémonie d'honneur", "Les gardes lisent les noms de ceux qui ont tenu une rue, une porte ou juste une promesse.", "Présenter ses services récents"};
            case 6:
                return {"village_games", "Jeux du village", "Ce n'est pas un entraînement militaire, mais tout le monde regarde quand quelqu'un tombe dans la boue.", "Participer sans casser le mobilier"};
            default:
                return {"harvest_festival", "Fête des moissons / solstice", "Les cuisines fument, les greniers s'ouvrent et les anciens surveillent qui aide vraiment.", "Porter, compter et distribuer"};
        }
    }

    int projectedCityMarkerDays(const Player* player, const std::string& materialId, int targetDay)
    {
        if (player == nullptr)
        {
            return 0;
        }
        const int daysAhead = std::max(0, std::max(0, targetDay) - safeCityDay(*player));
        return std::max(0, player->getInventory().countMaterialById(materialId) - daysAhead);
    }

    int cityScheduledWeekPressurePercent(int targetDay)
    {
        const int safeTargetDay = std::max(0, targetDay);
        const int dayInWeek = safeTargetDay % kRegularCityEventIntervalDays;
        int pressure = 0;

        // Même si le joueur ne participe pas, la ville doit quand même gérer l'affiche régulière
        // de la semaine : gardes, place publique, stocks, annonces, familles qui viennent voir.
        // Cela compte dans la chance du second événement royal : on ne rajoute pas une fête au-dessus
        // d'une ville déjà occupée par sa propre affiche.
        if (dayInWeek >= 1 && dayInWeek <= 4)
        {
            pressure += 12;
        }
        else if (dayInWeek >= 5)
        {
            pressure += 6;
        }

        // Petite variation fixe de semaine : rumeurs de passage, inventaire municipal, pluie, route fermée.
        // Ça évite que chaque semaine soit mathématiquement identique sans créer un event de plus.
        const int weekIndex = safeTargetDay / kRegularCityEventIntervalDays;
        pressure += (weekIndex * 11 + 3) % 7;
        return std::min(18, pressure);
    }

    int cityOtherEventPressurePercent(const Player* player, int targetDay)
    {
        if (player == nullptr)
        {
            return 0;
        }

        int pressure = cityScheduledWeekPressurePercent(targetDay);
        if (projectedCityMarkerDays(player, "city_repair_days_marker", targetDay) > 0
            || player->getInventory().countMaterialById("city_damage_notice") > 0)
        {
            // Une ville déjà occupée à réparer ne rajoute pas une seconde fête royale par-dessus.
            pressure += 100;
        }
        if (projectedCityMarkerDays(player, "city_defense_gratitude_days_marker", targetDay) > 0)
        {
            // Une défense récente compte déjà comme gros événement local : le bonus royal devient moins probable.
            pressure += 22;
        }
        const int recentEventDays = projectedCityMarkerDays(player, "city_event_recent_days_marker", targetDay);
        if (recentEventDays > 0)
        {
            // Une foire, un tournoi ou une affiche royale déjà gérée dans la semaine fatigue les organisateurs.
            pressure += std::min(30, 12 + recentEventDays * 6);
        }
        return std::min(100, pressure);
    }

    int royalBonusCityEventChanceForPlayer(const Player* player, int targetDay)
    {
        return std::max(0, kRoyalBonusCityEventChancePercent - cityOtherEventPressurePercent(player, targetDay));
    }

    std::string royalBonusChanceContextLine(const Player& player, int targetDay)
    {
        const int pressure = cityOtherEventPressurePercent(&player, targetDay);
        const int chance = royalBonusCityEventChanceForPlayer(&player, targetDay);
        if (pressure >= 100)
        {
            return "Second événement royal : impossible pour cette date, la ville gère déjà une crise, des dégâts ou des réparations.";
        }
        return "Second événement royal : chance actuelle " + std::to_string(chance)
            + "% après prise en compte des affiches régulières, événements récents, défenses, réparations et tensions de semaine.";
    }

    bool hasRoyalBonusCityEventThisWeekIndex(int weekIndex, const Player* player = nullptr, int targetDay = 0)
    {
        const int seed = (std::max(0, weekIndex) * 73 + 29) % 100;
        return seed < royalBonusCityEventChanceForPlayer(player, targetDay);
    }

    int royalBonusCityEventOffsetForWeekIndex(int weekIndex)
    {
        return 2 + ((std::max(0, weekIndex) * 17 + 5) % 3); // entre 2 et 4 jours après l'affiche régulière
    }

    CityEventOfDay royalBonusCityEventForWeekIndex(int weekIndex)
    {
        switch ((std::max(0, weekIndex) * 5 + 1) % 4)
        {
            case 0:
                return {"royal_merit_reward", "Récompense royale des mérites du peuple", "Un héraut affirme que le roi veut remercier les artisans, gardes et petites mains restées debout.", "Aider à distribuer sans te servir au passage"};
            case 1:
                return {"royal_whim_games", "Jeux ordonnés par caprice royal", "Personne ne sait si le roi était inspiré ou juste de bonne humeur, mais les fanions sont déjà posés.", "Participer à une épreuve courte et encadrée"};
            case 2:
                return {"royal_supply_day", "Distribution royale de provisions", "Les greniers ouvrent un peu : pas assez pour devenir riche, assez pour calmer la rue.", "Aider à compter et porter les caisses"};
            default:
                return {"royal_patrol_gratitude", "Patrouille honorifique du roi", "Un officier veut montrer que la couronne n'oublie pas les portes qui tiennent.", "Escorter une ronde symbolique"};
        }
    }

    bool hasRegularCityEventOnDay(int day)
    {
        return std::max(0, day) % kRegularCityEventIntervalDays == 0;
    }

    bool hasRoyalBonusCityEventOnDay(int day, const Player* player = nullptr)
    {
        const int safeDay = std::max(0, day);
        if (hasRegularCityEventOnDay(safeDay))
        {
            return false;
        }
        const int weekIndex = safeDay / kRegularCityEventIntervalDays;
        return hasRoyalBonusCityEventThisWeekIndex(weekIndex, player, safeDay)
            && safeDay % kRegularCityEventIntervalDays == royalBonusCityEventOffsetForWeekIndex(weekIndex);
    }

    bool hasRegularCityEventToday(const Player& player)
    {
        return hasRegularCityEventOnDay(safeCityDay(player));
    }

    bool hasRoyalBonusCityEventToday(const Player& player)
    {
        return hasRoyalBonusCityEventOnDay(safeCityDay(player), &player);
    }

    bool hasAnyCityEventToday(const Player& player)
    {
        return hasRegularCityEventToday(player) || hasRoyalBonusCityEventToday(player);
    }


    CityEventOfDay cityEventForAbsoluteDay(int day, const Player* player = nullptr)
    {
        const int safeDay = std::max(0, day);
        const int weekIndex = safeDay / kRegularCityEventIntervalDays;
        if (hasRoyalBonusCityEventOnDay(safeDay, player))
        {
            return royalBonusCityEventForWeekIndex(weekIndex);
        }
        return regularCityEventForWeekIndex(weekIndex);
    }

    CityEventOfDay cityEventForTodayOrNext(const Player& player)
    {
        const int today = safeCityDay(player);
        if (hasAnyCityEventToday(player))
        {
            return cityEventForAbsoluteDay(today, &player);
        }
        for (int offset = 1; offset <= kRegularCityEventIntervalDays; ++offset)
        {
            if (hasRegularCityEventOnDay(today + offset) || hasRoyalBonusCityEventOnDay(today + offset, &player))
            {
                return cityEventForAbsoluteDay(today + offset, &player);
            }
        }
        return regularCityEventForWeekIndex((today + kRegularCityEventIntervalDays) / kRegularCityEventIntervalDays);
    }

    int daysUntilNextRegularCityEvent(const Player& player)
    {
        const int remainder = safeCityDay(player) % kRegularCityEventIntervalDays;
        return remainder == 0 ? 0 : kRegularCityEventIntervalDays - remainder;
    }

    int daysUntilNextAnyCityEvent(const Player& player)
    {
        const int today = safeCityDay(player);
        for (int offset = 0; offset <= kRegularCityEventIntervalDays; ++offset)
        {
            if (hasRegularCityEventOnDay(today + offset) || hasRoyalBonusCityEventOnDay(today + offset, &player))
            {
                return offset;
            }
        }
        return daysUntilNextRegularCityEvent(player);
    }

    bool isRoyalBonusCityEvent(const CityEventOfDay& event)
    {
        return event.id.rfind("royal_", 0) == 0;
    }

    void markCityEventAsRecentlyHandled(Player& player, bool royalEvent)
    {
        const int previousDays = player.getInventory().countMaterialById("city_event_recent_days_marker");
        if (previousDays > 0)
        {
            player.getInventory().removeMaterialQuantityById("city_event_recent_days_marker", previousDays);
        }
        const int newDays = std::min(4, std::max(previousDays, royalEvent ? 3 : 2));
        player.getInventory().addMaterial(MaterialCatalog::createById("city_event_recent_days_marker", newDays));
    }

    std::vector<std::string> cityRaceReactionLines(const Player& player)
    {
        std::vector<std::string> lines;
        switch (player.getRace())
        {
            case CharacterRace::SemiFox:
            case CharacterRace::Kitsune:
                lines.push_back("Réaction locale : certains marchands gardent un œil sur les balances, mais écoutent volontiers une bonne négociation.");
                break;
            case CharacterRace::SemiDog:
            case CharacterRace::SemiWolf:
                lines.push_back("Réaction locale : les gardes et pisteurs parlent plus vite de patrouille que de paperasse.");
                break;
            case CharacterRace::SemiCat:
                lines.push_back("Réaction locale : on te confie facilement les toits, les caves et les passages trop étroits pour les armures.");
                break;
            case CharacterRace::SemiLizard:
                lines.push_back("Réaction locale : les ouvriers pensent aux canaux, aux mares et aux pierres humides quand ils te voient arriver.");
                break;
            case CharacterRace::SemiBird:
                lines.push_back("Réaction locale : les messagers demandent surtout ce que tu as vu depuis les hauteurs.");
                break;
            case CharacterRace::Gnome:
            case CharacterRace::Dwarf:
                lines.push_back("Réaction locale : les artisans te parlent outils, mesures et réparations avant même de parler prime.");
                break;
            case CharacterRace::Vampire:
            case CharacterRace::Tiefling:
            case CharacterRace::DarkElf:
                lines.push_back("Réaction locale : l'accueil reste poli, mais les temples et guichets vérifient deux fois les sceaux.");
                break;
            default:
                break;
        }
        return lines;
    }

    void appendCityEventRaceAid(Player& player, const CityEventOfDay& event, Random& random, std::vector<std::string>& lines)
    {
        bool helped = false;
        switch (player.getRace())
        {
            case CharacterRace::SemiFox:
            case CharacterRace::Kitsune:
                if (event.id == "merchant_fair" || event.id == "mission_exchange" || event.id == "royal_merit_reward")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                    lines.push_back("Atout racial : ta ruse aide à repérer une clause bizarre ou un client trop sûr de lui. Note de réputation locale x1.");
                    helped = true;
                }
                break;
            case CharacterRace::SemiDog:
            case CharacterRace::SemiWolf:
                if (event.id == "monster_hunt" || event.id == "royal_patrol_gratitude")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("advanced_monster_notes", 1));
                    lines.push_back("Atout racial : flair et rondes propres donnent une piste plus fiable. Notes avancées de monstres x1.");
                    helped = true;
                }
                break;
            case CharacterRace::SemiCat:
                if (event.id == "village_games" || event.id == "honor_ceremony")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                    lines.push_back("Atout racial : les passages étroits et les toits deviennent une vraie aide logistique. Tampon de service municipal x1.");
                    helped = true;
                }
                break;
            case CharacterRace::SemiLizard:
                if (event.id == "harvest_festival" || event.id == "royal_supply_day")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("bitter_healing_leaf", 1));
                    lines.push_back("Atout racial : tu aides près des canaux et réserves humides sans glisser toutes les deux secondes. Feuille amère de soin x1.");
                    helped = true;
                }
                break;
            case CharacterRace::SemiBird:
                if (event.id == "monster_hunt" || event.id == "mission_exchange" || event.id == "royal_patrol_gratitude")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("route_scout_note", 1));
                    lines.push_back("Atout racial : un regard depuis les hauteurs évite une fausse piste. Note d'éclaireur de route x1.");
                    helped = true;
                }
                break;
            case CharacterRace::Gnome:
            case CharacterRace::Dwarf:
                if (event.id == "merchant_fair" || event.id == "knowledge_day")
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("municipal_proof_letter", 1));
                    lines.push_back("Atout racial : mesures, outils et détails propres convainquent le bureau. Attestation municipale x1.");
                    helped = true;
                }
                break;
            default:
                break;
        }

        if (!helped && random.between(1, 100) <= 12)
        {
            lines.push_back("Petit détail local : rien de spécial à exploiter aujourd'hui, mais l'aide reste notée sans bonus abusif.");
        }
    }

    void appendCityEventReward(Player& player, const CityEventOfDay& event, std::vector<std::string>& lines)
    {
        Random random;
        if (event.id == "royal_merit_reward")
        {
            const int reward = 10 + player.getLevel() / 2;
            player.getInventory().earnEconomyUnits(reward);
            player.refreshCurrencyTitles();
            player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
            lines.push_back("Le héraut insiste : ce n'est pas une foire permanente, juste une grâce rare pour services rendus au peuple.");
            lines.push_back("Récompense : " + Money::formatEconomyUnits(reward) + " + Note de réputation locale x1.");
        }
        else if (event.id == "royal_whim_games")
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
            player.getInventory().addMaterial(MaterialCatalog::createById("survival_ration", 1));
            lines.push_back("L'épreuve est courte et encadrée : assez sérieuse pour compter, pas assez rentable pour être farmée.");
            lines.push_back("Récompense : Tampon de service municipal x1 + ration de survie x1.");
        }
        else if (event.id == "royal_supply_day")
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("survival_ration", random.between(1, 2)));
            player.getInventory().addMaterial(MaterialCatalog::createById("bitter_healing_leaf", 1));
            lines.push_back("Tu aides surtout à éviter les resquilleurs et les doubles comptes. Le roi paie l'affiche, pas ton futur château.");
            lines.push_back("Ressources : ration(s) de survie + feuille médicinale amère x1.");
        }
        else if (event.id == "royal_patrol_gratitude")
        {
            const int reward = 12 + player.getLevel();
            player.getInventory().earnEconomyUnits(reward);
            player.refreshCurrencyTitles();
            player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
            lines.push_back("La patrouille n'a rien d'une guerre, mais la présence d'un aventurier rassure les rues." );
            lines.push_back("Récompense : " + Money::formatEconomyUnits(reward) + " + Tampon de service municipal x1.");
        }
        else if (event.id == "guild_tournament")
        {
            const int reward = 14 + player.getLevel();
            player.getInventory().earnEconomyUnits(reward);
            player.refreshCurrencyTitles();
            player.getInventory().addMaterial(MaterialCatalog::createById("guild_favor_token", 1));
            lines.push_back("Tu combats ou arbitres en cadre sécurisé : assez réel pour apprendre, pas assez libre pour finir en massacre.");
            lines.push_back("Récompense : " + Money::formatEconomyUnits(reward) + " + Jeton de faveur de guilde x1.");
        }
        else if (event.id == "monster_hunt")
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("advanced_monster_notes", 1));
            if (random.between(1, 100) <= 45)
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("wolf_fang", 1));
                lines.push_back("Un pisteur te laisse un croc marqué pour comparer les morsures plus tard.");
            }
            lines.push_back("Note obtenue : notes avancées de monstres x1.");
        }
        else if (event.id == "merchant_fair")
        {
            const int reward = 10 + player.getLevel() / 2;
            player.getInventory().earnEconomyUnits(reward);
            player.refreshCurrencyTitles();
            player.getInventory().addMaterial(MaterialCatalog::createById("local_service_letter", 1));
            lines.push_back("Tu aides à tenir un étal, contrôler une facture ou calmer une dispute de prix sans reprendre les mauvais prix des fiches.");
            lines.push_back("Récompense : " + Money::formatEconomyUnits(reward) + " + Lettre de service local x1.");
        }
        else if (event.id == "knowledge_day")
        {
            if (player.getInventory().spendEconomyUnits(12))
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("magic_learning_notes", 1));
                lines.push_back("Tu paies une place modeste pour copier des notes propres au lieu d'écouter depuis la fenêtre.");
                lines.push_back("Notes obtenues : notes d'apprentissage magique x1.");
            }
            else
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("client_recommendation", 1));
                lines.push_back("Tu n'as pas payé la copie complète, mais l'archiviste te donne une recommandation pour revenir mieux préparé.");
            }
        }
        else if (event.id == "mission_exchange")
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("client_recommendation", 1));
            player.getInventory().addMaterial(MaterialCatalog::createById("local_service_letter", 1));
            lines.push_back("Tu tries les demandes qui sentent la vraie urgence et celles qui sentent juste le client qui crie fort.");
            lines.push_back("Documents obtenus : recommandation de client x1, lettre de service local x1.");
        }
        else if (event.id == "honor_ceremony")
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
            if (player.getInventory().countMaterialById("city_defense_medal") > 0)
            {
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                lines.push_back("Ton attestation de défense donne du poids à ton nom pendant la cérémonie.");
            }
            lines.push_back("Tampon de service municipal x1 obtenu.");
        }
        else if (event.id == "village_games")
        {
            const int reward = random.between(6, 18 + player.getLevel());
            player.getInventory().earnEconomyUnits(reward);
            player.refreshCurrencyTitles();
            player.getInventory().addMaterial(MaterialCatalog::createById("survival_ration", 1));
            lines.push_back("Tu gagnes surtout de la boue, quelques rires, et assez de nourriture pour ne pas appeler ça une perte de temps.");
            lines.push_back("Récompense : " + Money::formatEconomyUnits(reward) + " + ration de survie x1.");
        }
        else
        {
            player.getInventory().addMaterial(MaterialCatalog::createById("bitter_healing_leaf", random.between(1, 3)));
            player.getInventory().addMaterial(MaterialCatalog::createById("survival_ration", 1));
            lines.push_back("Tu aides à porter, compter, distribuer. Pas glorieux, mais les greniers ne se remplissent pas en criant 'quête'.");
            lines.push_back("Ressources obtenues : plantes communes et ration de survie.");
        }

        appendCityEventRaceAid(player, event, random, lines);
    }

    void participateInCityEventOfDay(Player& player)
    {
        if (!hasAnyCityEventToday(player))
        {
            const CityEventOfDay nextEvent = cityEventForTodayOrNext(player);
            showShopResult(
                "AUCUN ÉVÉNEMENT ACTIF",
                "shop.city.events.none_today",
                {
                    "Il n'y a pas d'événement de guilde, de village ou d'ordre royal aujourd'hui. La ville respire aussi entre deux affiches.",
                    "Rythme actuel : une affiche régulière environ par semaine. Un second événement royal reste rare : sa chance baisse aussi avec l'affiche régulière, les autres événements, les défenses, crises et réparations.",
                    "Prochaine affiche dans " + std::to_string(daysUntilNextAnyCityEvent(player)) + " jour(s) : " + nextEvent.name + "."
                }
            );
            return;
        }

        const CityEventOfDay event = cityEventForTodayOrNext(player);
        std::vector<std::string> lines = {
            std::string(isRoyalBonusCityEvent(event) ? "Événement exceptionnel actif : " : "Événement régulier actif : ") + event.name + ".",
            event.mood,
            "Action choisie : " + event.action + "."
        };
        if (isRoyalBonusCityEvent(event))
        {
            lines.push_back("Origine : ordre du roi, récompense des mérites du peuple ou caprice officiel. Ça peut arriver en plus de l'affiche hebdomadaire, mais la ville évite d'empiler ça sur une crise ou une défense récente.");
        }
        std::vector<std::string> raceLines = cityRaceReactionLines(player);
        lines.insert(lines.end(), raceLines.begin(), raceLines.end());
        appendCityEventReward(player, event, lines);
        markCityEventAsRecentlyHandled(player, isRoyalBonusCityEvent(event));
        lines.push_back(isRoyalBonusCityEvent(event)
            ? "Organisation : l'affiche royale occupe le bureau pendant quelques jours, ce qui rend un autre événement encore moins probable."
            : "Organisation : l'événement compte comme animation récente de ville et réduit un peu la chance d'un second événement royal cette semaine.");
        showLocalServiceResult("ÉVÉNEMENT DE VILLE", "shop.city.events.participate", player, lines, 1);
    }

    void showLocalEconomyReport(const Player& player)
    {
        std::vector<std::string> lines;
        const int repairDays = cityRepairDaysRemaining(player);
        if (repairDays > 0)
        {
            lines.push_back("Économie locale : crise active, réparations restantes " + std::to_string(repairDays) + " jour(s).");
            lines.push_back("Effet concret : commerces fermés ou ouverts au compte-gouttes ; les demandes utiles valent souvent mieux que négocier trois pièces.");
            lines.push_back("Ressources recherchées : métal rouillé, cuir abîmé, feuilles de soin, rations, poussière arcanique selon le quartier touché.");
            lines.push_back("Prix de crise : les comptoirs ouverts hors services prioritaires montent un peu leurs prix, mais ton aide locale peut réduire cette tension jusqu'à un minimum raisonnable.");
        }
        else
        {
            const CityEventOfDay event = cityEventForTodayOrNext(player);
            lines.push_back("Économie locale : stable pour l'instant.");
            const int gratitudeDays = player.getInventory().countMaterialById("city_defense_gratitude_days_marker");
            const int recentEventDays = player.getInventory().countMaterialById("city_event_recent_days_marker");
            if (gratitudeDays > 0)
            {
                lines.push_back("Reconnaissance de défense : petites remises d'achat encore actives pendant " + std::to_string(gratitudeDays) + " jour(s), sauf marché noir et exceptions louches.");
            }
            if (recentEventDays > 0)
            {
                lines.push_back("Organisation locale : un événement récent occupe encore le bureau pendant " + std::to_string(recentEventDays) + " jour(s), donc un second événement royal devient moins probable.");
            }
            if (hasAnyCityEventToday(player))
            {
                lines.push_back(std::string(isRoyalBonusCityEvent(event) ? "Événement exceptionnel actif aujourd'hui : " : "Événement régulier actif aujourd'hui : ") + event.name + ".");
                if (isRoyalBonusCityEvent(event))
                {
                    lines.push_back("Raison annoncée : ordre royal, mérite du peuple ou caprice de cour. C'est rare, donc les récompenses restent modestes.");
                }
                lines.push_back("Marché : certaines familles de boutiques peuvent bouger de 2 ou 3%, mais rien qui permette de casser l'économie.");
            }
            else
            {
                lines.push_back("Pas d'événement actif aujourd'hui. Prochaine affiche dans " + std::to_string(daysUntilNextAnyCityEvent(player)) + " jour(s) : " + event.name + ".");
            }
            lines.push_back("Équilibre : les événements donnent surtout documents, ressources et petites primes ; les gros gains doivent rester liés aux vrais risques.");
            lines.push_back("Prix : les petites variations de ville restent limitées à quelques pourcents. Acheter/revendre en boucle ne doit pas devenir une stratégie.");
            lines.push_back("Plafond : réputation, défense et affiche favorable ne peuvent pas empiler plus de " + std::to_string(kCityEconomyDiscountCapPercent) + "% de remise locale utile.");
            lines.push_back(royalBonusChanceContextLine(player, safeCityDay(player)));
            lines.push_back("Troc : les ressources locales gardent une valeur utile, surtout quand la ville répare ou manque de bras.");
        }
        const std::vector<std::string> raceLines = cityRaceReactionLines(player);
        lines.insert(lines.end(), raceLines.begin(), raceLines.end());
        showShopResult("ÉCONOMIE LOCALE", "shop.city.economy.report", lines);
    }

    void showCityAndGuildNoticeBoard(const Player& player)
    {
        std::vector<std::string> lines;
        const int repairDays = cityRepairDaysRemaining(player);
        lines.push_back("Panneau commun : les annonces de ville et de guilde servent surtout à lire le contexte, pas à donner une récompense gratuite.");
        lines.push_back("Date locale : " + player.formatWorldDateTimeLine() + ".");

        if (repairDays > 0)
        {
            lines.push_back("Priorité affichée : réparations encore " + std::to_string(repairDays) + " jour(s). Les demandes utiles restent réparation, garde, récolte et remise en route.");
            lines.push_back("La guilde suspend les annonces festives : elle préfère des bras fiables à une nouvelle cérémonie.");
        }
        else
        {
            const CityEventOfDay event = cityEventForTodayOrNext(player);
            if (hasAnyCityEventToday(player))
            {
                lines.push_back(std::string(isRoyalBonusCityEvent(event) ? "Affiche active exceptionnelle : " : "Affiche active régulière : ") + event.name + ".");
                lines.push_back("Rappel : participer consomme du temps et donne surtout documents, ressources ou petite réputation, pas une pluie d'or.");
            }
            else
            {
                lines.push_back("Aucune affiche active aujourd'hui. Prochaine affiche dans " + std::to_string(daysUntilNextAnyCityEvent(player)) + " jour(s) : " + event.name + ".");
                lines.push_back(royalBonusChanceContextLine(player, safeCityDay(player)));
            }
            const int weekIndex = safeCityDay(player) / kRegularCityEventIntervalDays;
            switch ((weekIndex * 13 + 4) % 5)
            {
                case 0:
                    lines.push_back("Rumeur de guilde : une patrouille cherche des volontaires, mais rien n'est encore assez grave pour devenir une quête officielle.");
                    break;
                case 1:
                    lines.push_back("Rumeur marchande : les artisans surveillent le stock de métal et de cuir avant la prochaine affiche.");
                    break;
                case 2:
                    lines.push_back("Rumeur de quartier : quelques familles demandent surtout des bras fiables, pas un héros qui casse la porte.");
                    break;
                case 3:
                    lines.push_back("Rumeur de garde : les rondes ont été renforcées autour des comptoirs sensibles.");
                    break;
                default:
                    lines.push_back("Rumeur calme : la ville respire un peu. C'est aussi important qu'une quête.");
                    break;
            }
        }

        const int recentEventDays = player.getInventory().countMaterialById("city_event_recent_days_marker");
        if (recentEventDays > 0)
        {
            lines.push_back("Organisation : le bureau reste occupé encore " + std::to_string(recentEventDays) + " jour(s) par une affiche récente.");
        }

        if (player.getInventory().countMaterialById("city_defense_gratitude_days_marker") > 0)
        {
            lines.push_back("Défense récente : les commerçants reconnaissent encore ton nom, mais la remise reste courte et plafonnée.");
        }

        switch (player.getRace())
        {
            case CharacterRace::SemiFox:
            case CharacterRace::Kitsune:
                lines.push_back("Piste raciale : contrats, foires et bourses aux missions sont les meilleurs moments pour repérer une clause louche.");
                break;
            case CharacterRace::SemiDog:
            case CharacterRace::SemiWolf:
                lines.push_back("Piste raciale : les rondes, chasses locales et alertes de route t'emploient mieux que les comptoirs fermés.");
                break;
            case CharacterRace::SemiCat:
                lines.push_back("Piste raciale : les toits, greniers et passages étroits reviennent souvent pendant les réparations ou fêtes bondées.");
                break;
            case CharacterRace::SemiLizard:
                lines.push_back("Piste raciale : canaux, mares et pierres humides sont souvent oubliés par les ouvriers pressés.");
                break;
            case CharacterRace::SemiBird:
                lines.push_back("Piste raciale : observation, messagers et hauteurs donnent parfois une meilleure aide qu'un combat.");
                break;
            case CharacterRace::Gnome:
            case CharacterRace::Dwarf:
                lines.push_back("Piste raciale : mesures, outils et réparations précises comptent beaucoup quand l'économie locale se tend.");
                break;
            default:
                lines.push_back("Piste générale : les petites preuves locales servent surtout à ouvrir des services, pas à remplacer les vraies quêtes.");
                break;
        }

        showShopResult("PANNEAU VILLE / GUILDE", "shop.city.notice_board", lines);
    }

    void openCityRepairWorkOrderMenu(Player& player)
    {
        bool stay = true;
        while (stay)
        {
            MenuScreen screen("DEMANDES DE RÉPARATION", "shop.city.repair.orders");
            screen.addLine("Ces demandes sont courtes : elles servent à rendre la crise visible sans bloquer le jeu pendant trois heures.");
            screen.addLine("Réparations restantes : " + std::to_string(cityRepairDaysRemaining(player)) + " jour(s).");
            screen.addBackOption("Retour", "shop.city.repair.orders.back");
            screen.addOption(1, "Réparer l'infirmerie", "Demande : Feuille amère de soin x2 + Morceau de cuir abîmé x1.", true, "shop.city.repair.order.infirmary");
            screen.addOption(2, "Renforcer les étals du marché", "Demande : Fragment de métal rouillé x4 + Morceau de cuir abîmé x1.", true, "shop.city.repair.order.market");
            screen.addOption(3, "Rationner les ouvriers", "Demande : Ration de survie x2. Réduit surtout la panique et la fatigue.", true, "shop.city.repair.order.rations");
            screen.addOption(4, "Stabiliser une faille mineure", "Demande : Poussière arcanique x2. Utile si la crise a une trace magique.", true, "shop.city.repair.order.arcane");
            screen.addOption(5, "Aider selon tes atouts", "Petite intervention selon race/classe : toit, garde, canal, mesure, négociation. Gain modeste, pas farm gratuit.", true, "shop.city.repair.order.personal_skill");
            screen.addOption(6, "Consolider des murs fissurés", "Demande : Argile rouge séchée x2 + Fragment de métal rouillé x2.", true, "shop.city.repair.order.walls");
            screen.addOption(7, "Organiser une collecte de quartier", "Pas d'argent direct : transforme un peu de temps en ressources simples pour les réparations.", true, "shop.city.repair.order.collection");
            screen.addOption(8, "Remettre une route de relais en état", "Demande : Reçu de péage de route x1 + Fragment de métal rouillé x2.", true, "shop.city.repair.order.relay_road");
            screen.addOption(9, "Préparer une réserve de secours", "Demande : Ration de survie x3 + Feuille amère de soin x1.", true, "shop.city.repair.order.reserve");
            screen.addOption(10, "Rouvrir le panneau des quêtes", "Demande : Lettre de service local x1 + Tampon municipal x1. Aide la guilde à relancer les petites missions.", true, "shop.city.repair.order.quest_board");
            screen.addOption(11, "Dégager une rue encombrée", "Sortie courte : peu d'or, quelques matériaux, parfois une journée de réparation gagnée.", true, "shop.city.repair.order.clear_street");
            screen.addOption(12, "Sécuriser un entrepôt fragile", "Demande : Kit de réparation faible x1 + Fragment de métal rouillé x1. Protège les stocks sans grosse prime.", true, "shop.city.repair.order.storage");
            screen.addOption(13, "Contrôler les prix abusifs", "Action courte : aide le guichet à repérer les profiteurs de crise. Gain surtout réputation/ordre local.", true, "shop.city.repair.order.price_check");
            screen.addOption(14, "Protéger un dépôt de rations", "Demande : Ration de survie x2 + Kit de réparation faible x1. Protège les stocks utiles.", true, "shop.city.repair.order.ration_depot");
            screen.addOption(15, "Faire une ronde avec la guilde", "Petite ronde utile : surtout sécurité, notes et chance limitée de réduire la crise.", true, "shop.city.repair.order.guild_round");

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une demande de réparation.");
            Console::clear();
            if (choice == 0)
            {
                stay = false;
                continue;
            }

            std::vector<std::string> lines;
            auto missing = [&](const std::string& id, int qty) {
                return player.getInventory().countMaterialById(id) < qty;
            };
            auto consume = [&](const std::string& id, int qty) {
                player.getInventory().removeMaterialQuantityById(id, qty);
            };

            if (choice == 1)
            {
                if (missing("bitter_healing_leaf", 2) || missing("worn_leather_piece", 1))
                {
                    showShopResult("MATÉRIAUX MANQUANTS", "shop.city.repair.order.infirmary.failed", {"Il faut Feuille amère de soin x2 et Morceau de cuir abîmé x1.", "L'infirmerie ne demande pas une belle histoire : elle demande de quoi panser les gens."});
                    continue;
                }
                consume("bitter_healing_leaf", 2);
                consume("worn_leather_piece", 1);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                lines = {"Tu fournis bandages, sangles et plantes. L'infirmerie respire un peu mieux.", "Réparations réduites de 1 jour. Reçu d'aide aux réparations x1."};
                showLocalServiceResult("INFIRMERIE AIDÉE", "shop.city.repair.order.infirmary.success", player, lines, 1);
            }
            else if (choice == 2)
            {
                if (missing("rusted_metal_fragment", 4) || missing("worn_leather_piece", 1))
                {
                    showShopResult("MATÉRIAUX MANQUANTS", "shop.city.repair.order.market.failed", {"Il faut Fragment de métal rouillé x4 et Morceau de cuir abîmé x1.", "Les étals peuvent être moches, mais ils doivent tenir debout."});
                    continue;
                }
                consume("rusted_metal_fragment", 4);
                consume("worn_leather_piece", 1);
                reduceCityRepairDays(player, 2);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                lines = {"Tu aides à renforcer plusieurs comptoirs. Les marchands arrêtent de tenir leurs caisses à deux mains.", "Réparations réduites de 2 jours. Tampon de service municipal x1."};
                showLocalServiceResult("MARCHÉ RENFORCÉ", "shop.city.repair.order.market.success", player, lines, 1);
            }
            else if (choice == 3)
            {
                if (missing("survival_ration", 2))
                {
                    showShopResult("RATIONS MANQUANTES", "shop.city.repair.order.rations.failed", {"Il faut Ration de survie x2.", "Un chantier sans nourriture finit par réparer les disputes avant les murs."});
                    continue;
                }
                consume("survival_ration", 2);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("local_service_letter", 1));
                lines = {"Les ouvriers mangent quelque chose qui ressemble presque à un repas. Le chantier évite une journée de retard.", "Réparations réduites de 1 jour. Lettre de service local x1."};
                showLocalServiceResult("OUVRIERS RAVITAILLÉS", "shop.city.repair.order.rations.success", player, lines, 1);
            }
            else if (choice == 4)
            {
                if (missing("arcane_dust", 2))
                {
                    showShopResult("COMPOSANTS MANQUANTS", "shop.city.repair.order.arcane.failed", {"Il faut Poussière arcanique x2.", "Le scribe refuse d'appeler ça de la magie noire. Il dit seulement que le mur grésille."});
                    continue;
                }
                consume("arcane_dust", 2);
                reduceCityRepairDays(player, 2);
                player.getInventory().addMaterial(MaterialCatalog::createById("municipal_proof_letter", 1));
                lines = {"La faille mineure se calme assez pour que les ouvriers approchent sans perdre leurs outils dans une lumière bizarre.", "Réparations réduites de 2 jours. Preuve municipale x1."};
                showLocalServiceResult("TRACE STABILISÉE", "shop.city.repair.order.arcane.success", player, lines, 1);
            }

            else if (choice == 5)
            {
                Random random;
                const int roll = random.between(1, 100);
                lines.push_back("Le contremaître ne te donne pas un grand discours : il cherche surtout où tes atouts peuvent éviter une erreur de plus.");
                switch (player.getRace())
                {
                    case CharacterRace::SemiCat:
                        lines.push_back("On t'envoie vérifier des toits, poutres basses et passages où une armure coincerait tout le monde.");
                        break;
                    case CharacterRace::SemiLizard:
                        lines.push_back("On te confie les canaux, pierres humides et écoulements qui ruinent souvent les réparations propres.");
                        break;
                    case CharacterRace::SemiBird:
                        lines.push_back("On te demande d'observer les hauteurs avant qu'un ouvrier découvre trop tard qu'un pan de mur bouge.");
                        break;
                    case CharacterRace::SemiDog:
                    case CharacterRace::SemiWolf:
                        lines.push_back("Les gardes te placent sur les odeurs, traces et rondes autour des entrepôts encore ouverts.");
                        break;
                    case CharacterRace::Gnome:
                    case CharacterRace::Dwarf:
                        lines.push_back("Les artisans te collent près des mesures, cales et outils : moins glorieux, plus utile.");
                        break;
                    default:
                        lines.push_back("Tu aides là où il manque des bras : porter, compter, surveiller, calmer quelqu'un qui veut déjà rouvrir trop tôt.");
                        break;
                }
                if (roll <= 45)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("Résultat : une erreur de chantier évitée. Réparations réduites de 1 jour.");
                }
                else
                {
                    lines.push_back("Résultat : la journée avance sans miracle, mais le quartier reste organisé.");
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                showLocalServiceResult("AIDE CIBLÉE", "shop.city.repair.order.personal_skill.success", player, lines, 1);
            }
            else if (choice == 6)
            {
                if (missing("sun_dried_clay", 2) || missing("rusted_metal_fragment", 2))
                {
                    showShopResult("MATÉRIAUX MANQUANTS", "shop.city.repair.order.walls.failed", {"Il faut Argile rouge séchée x2 et Fragment de métal rouillé x2.", "Les murs fissurés n'ont pas besoin d'un héros légendaire : ils ont besoin de cales, d'argile et de métal."});
                    continue;
                }
                consume("sun_dried_clay", 2);
                consume("rusted_metal_fragment", 2);
                reduceCityRepairDays(player, 2);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("municipal_proof_letter", 1));
                lines = {"Tu aides à bloquer les fissures les plus dangereuses avant qu'elles ne deviennent une nouvelle crise.", "Réparations réduites de 2 jours. Reçu d'aide x1 et attestation municipale x1."};
                showLocalServiceResult("MURS CONSOLIDÉS", "shop.city.repair.order.walls.success", player, lines, 1);
            }
            else if (choice == 7)
            {
                Random random;
                lines = {
                    "Tu ne fais pas tomber une bourse d'or du ciel : tu organises des voisins, des paniers et des bras disponibles.",
                    "La collecte rapporte surtout des ressources utiles, pas une prime gratuite."
                };
                player.getInventory().addMaterial(MaterialCatalog::createById("rusted_metal_fragment", random.between(1, 2)));
                player.getInventory().addMaterial(MaterialCatalog::createById("worn_leather_piece", 1));
                if (random.between(1, 100) <= 40)
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("bitter_healing_leaf", 1));
                    lines.push_back("Une vieille voisine ajoute une feuille de soin en disant que les ouvriers font n'importe quoi avec leurs mains.");
                }
                if (random.between(1, 100) <= 25)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("La bonne organisation évite une demi-journée perdue : réparations réduites de 1 jour.");
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                showLocalServiceResult("COLLECTE ORGANISÉE", "shop.city.repair.order.collection.success", player, lines, 1);
            }
            else if (choice == 8)
            {
                if (missing("route_toll_receipt", 1) || missing("rusted_metal_fragment", 2))
                {
                    showShopResult("MATÉRIAUX MANQUANTS", "shop.city.repair.order.relay_road.failed", {"Il faut Reçu de péage de route x1 et Fragment de métal rouillé x2.", "Le relais ne peut pas rouvrir proprement si la route devant lui ressemble à une mâchoire cassée."});
                    continue;
                }
                consume("route_toll_receipt", 1);
                consume("rusted_metal_fragment", 2);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("relay_route_badge", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                lines = {"Tu aides le relais à remettre un passage praticable sans transformer ça en grande expédition.", "Réparations réduites de 1 jour. Badge de route du relais x1, note de réputation locale x1."};
                showLocalServiceResult("ROUTE DU RELAIS RÉPARÉE", "shop.city.repair.order.relay_road.success", player, lines, 1);
            }
            else if (choice == 9)
            {
                if (missing("survival_ration", 3) || missing("bitter_healing_leaf", 1))
                {
                    showShopResult("RÉSERVE INCOMPLÈTE", "shop.city.repair.order.reserve.failed", {"Il faut Ration de survie x3 et Feuille amère de soin x1.", "Une réserve de secours vide rassure seulement les affiches, pas les gens."});
                    continue;
                }
                consume("survival_ration", 3);
                consume("bitter_healing_leaf", 1);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("municipal_proof_letter", 1));
                lines = {"Tu montes une petite réserve pour éviter que le chantier s'arrête à la première toux ou au premier repas raté.", "Réparations réduites de 1 jour. Reçu d'aide x1, attestation municipale x1."};
                showLocalServiceResult("RÉSERVE DE SECOURS PRÊTE", "shop.city.repair.order.reserve.success", player, lines, 1);
            }
            else if (choice == 10)
            {
                if (missing("local_service_letter", 1) || missing("city_service_stamp", 1))
                {
                    showShopResult("DOSSIER INCOMPLET", "shop.city.repair.order.quest_board.failed", {"Il faut Lettre de service local x1 et Tampon de service municipal x1.", "Le panneau des quêtes ne repart pas avec juste un clou : il faut aussi des demandes vérifiées."});
                    continue;
                }
                consume("local_service_letter", 1);
                consume("city_service_stamp", 1);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("guild_favor_token", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("client_recommendation", 1));
                lines = {"Tu aides à trier les demandes urgentes, retirer les fausses primes et remettre une affiche propre devant la guilde.", "Réparations réduites de 1 jour. Jeton de faveur de guilde x1, recommandation de client x1."};
                showLocalServiceResult("PANNEAU DES QUÊTES ROUVERT", "shop.city.repair.order.quest_board.success", player, lines, 1);
            }
            else if (choice == 11)
            {
                Random random;
                lines = {
                    "Tu dégages une rue où tout le monde disait 'on verra demain' depuis trop longtemps.",
                    "Le gain reste modeste : la ville récupère surtout un passage utilisable."
                };
                player.getInventory().addMaterial(MaterialCatalog::createById("rusted_metal_fragment", random.between(1, 2)));
                if (random.between(1, 100) <= 55)
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("worn_leather_piece", 1));
                    lines.push_back("Tu récupères aussi quelques sangles abîmées sous les débris.");
                }
                if (random.between(1, 100) <= 30)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("La rue rouverte évite un détour de chantier : réparations réduites de 1 jour.");
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                showLocalServiceResult("RUE DÉGAGÉE", "shop.city.repair.order.clear_street.success", player, lines, 1);
            }
            else if (choice == 12)
            {
                if (missing("weak_repair_kit", 1) || missing("rusted_metal_fragment", 1))
                {
                    showShopResult("MATÉRIEL MANQUANT", "shop.city.repair.order.storage.failed", {"Il faut Kit de réparation faible x1 et Fragment de métal rouillé x1.", "L'entrepôt n'a pas besoin d'un miracle : il faut juste éviter que les stocks tombent sous la pluie ou les voleurs."});
                    continue;
                }
                consume("weak_repair_kit", 1);
                consume("rusted_metal_fragment", 1);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("client_recommendation", 1));
                lines = {"Tu renforces une porte, poses deux cales, et le responsable arrête de surveiller ses sacs comme s'ils allaient fuir.", "Réparations réduites de 1 jour. Reçu d'aide x1, recommandation de client x1."};
                showLocalServiceResult("ENTREPÔT SÉCURISÉ", "shop.city.repair.order.storage.success", player, lines, 1);
            }
            else if (choice == 13)
            {
                Random random;
                lines = {
                    "Le guichet ne demande pas de casser des dents : seulement de comparer les affiches, les stocks et les excuses des vendeurs pressés.",
                    "Objectif : limiter les abus de crise sans transformer chaque achat en débat politique."
                };
                switch (player.getRace())
                {
                    case CharacterRace::SemiFox:
                    case CharacterRace::Kitsune:
                        lines.push_back("Ton flair pour les clauses tordues aide à repérer deux faux frais cachés.");
                        break;
                    case CharacterRace::Gnome:
                    case CharacterRace::Dwarf:
                        lines.push_back("Tes mesures et tes comptes rapides rendent les prix gonflés plus difficiles à défendre.");
                        break;
                    default:
                        lines.push_back("Tu aides surtout à vérifier que les prix affichés correspondent encore aux stocks réels.");
                        break;
                }
                if (random.between(1, 100) <= 35)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("Quelques abus stoppés évitent une file de colère devant le bureau : réparations réduites de 1 jour.");
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("municipal_proof_letter", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                showLocalServiceResult("PRIX CONTRÔLÉS", "shop.city.repair.order.price_check.success", player, lines, 1);
            }
            else if (choice == 14)
            {
                if (missing("survival_ration", 2) || missing("weak_repair_kit", 1))
                {
                    showShopResult("DÉPÔT NON PROTÉGÉ", "shop.city.repair.order.ration_depot.failed", {"Il faut Ration de survie x2 et Kit de réparation faible x1.", "Le dépôt n'a pas besoin d'un champion, juste de caisses qui ferment et de réserves qui ne disparaissent pas."});
                    continue;
                }
                consume("survival_ration", 2);
                consume("weak_repair_kit", 1);
                reduceCityRepairDays(player, 1);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("local_service_letter", 1));
                lines = {"Tu sécurises les réserves sans transformer ça en trésor caché : moins de gaspillage, moins de disputes, plus de travail possible.", "Réparations réduites de 1 jour. Reçu d'aide x1, lettre de service local x1."};
                showLocalServiceResult("DÉPÔT PROTÉGÉ", "shop.city.repair.order.ration_depot.success", player, lines, 1);
            }
            else if (choice == 15)
            {
                Random random;
                lines = {"La guilde ne promet pas une aventure héroïque : juste une ronde où il faut rester éveillé, regarder les bonnes portes et ne pas paniquer au moindre bruit."};
                switch (player.getRace())
                {
                    case CharacterRace::SemiDog:
                    case CharacterRace::SemiWolf:
                        lines.push_back("Ton flair rend la ronde plus propre : une odeur de passage récent évite une fausse alerte.");
                        player.getInventory().addMaterial(MaterialCatalog::createById("advanced_monster_notes", 1));
                        break;
                    case CharacterRace::SemiBird:
                        lines.push_back("Depuis les hauteurs, tu signales un détour dangereux avant qu'un chariot s'y coince.");
                        player.getInventory().addMaterial(MaterialCatalog::createById("route_scout_note", 1));
                        break;
                    case CharacterRace::SemiCat:
                        lines.push_back("Tu passes par deux toits et un grenier que personne n'avait pensé à vérifier.");
                        player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                        break;
                    default:
                        lines.push_back("Tu aides surtout à tenir la présence visible dont les quartiers ont besoin pendant la crise.");
                        player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                        break;
                }
                if (random.between(1, 100) <= 30)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("La ronde évite un nouveau retard au chantier : réparations réduites de 1 jour.");
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                showLocalServiceResult("RONDE DE GUILDE", "shop.city.repair.order.guild_round.success", player, lines, 1);
            }
        }
    }

    void openCityServiceSpecialMenu(Player& player)
    {
        bool stay = true;
        while (stay)
        {
            const int repairDays = cityRepairDaysRemaining(player);
            MenuScreen screen(repairDays > 0 ? "BUREAU DES RÉPARATIONS" : "BUREAU DE VILLE", "shop.city.special");
            screen.addLine("Temps actuel : " + player.formatWorldDateTimeLine());
            if (repairDays > 0)
            {
                screen.addLine("État : réparations en cours pendant encore " + std::to_string(repairDays) + " jour(s).");
                screen.addLine("La ville priorise les gardes, les prêtres, l'auberge et quelques comptoirs tirés au sort selon les besoins du jour.");
                screen.addLine("Demandes du moment : réparer, garder les rues, récolter du cuir/bois/métal et remettre les échoppes debout.");
            }
            else
            {
                const CityEventOfDay event = cityEventForTodayOrNext(player);
                const int recentEventDays = player.getInventory().countMaterialById("city_event_recent_days_marker");
                screen.addLine("Le bureau affiche les événements de guilde et de village : tournoi, foire, journée du savoir, bourse aux missions, cérémonie, jeux, moissons.");
                screen.addLine("Rythme : environ un événement régulier par semaine. Certaines semaines peuvent avoir un second événement rare, mais sa chance tient compte de l'affiche régulière, des autres événements et des tensions de la ville.");
                if (recentEventDays > 0)
                {
                    screen.addLine("Organisation : la ville vient déjà de gérer une affiche. Un second événement royal est moins probable pendant encore " + std::to_string(recentEventDays) + " jour(s).");
                }
                if (hasAnyCityEventToday(player))
                {
                    screen.addLine(std::string(isRoyalBonusCityEvent(event) ? "Affiche exceptionnelle aujourd'hui : " : "Affiche régulière aujourd'hui : ") + event.name + " — " + event.action + ".");
                }
                else
                {
                    screen.addLine("Aucun événement actif aujourd'hui. Prochaine affiche dans " + std::to_string(daysUntilNextAnyCityEvent(player)) + " jour(s) : " + event.name + ".");
                }
                screen.addLine("Les vrais malheurs de ville restent très rares, mais ils peuvent bloquer l'économie locale si personne ne tient la ligne.");
            }
            screen.addOption(0, "Retour", "Revenir au comptoir municipal.", true, "shop.city.special.back");
            if (repairDays > 0)
            {
                screen.addOption(1, "Apporter des matériaux de réparation", "Demande : Fragment de métal rouillé x3 + Morceau de cuir abîmé x2. Réduit les réparations.", true, "shop.city.repair.materials");
                screen.addOption(2, "Aider à récolter des ressources", "Sortie courte encadrée : récupère surtout des matériaux simples pour les réparations.", true, "shop.city.repair.gather");
                screen.addOption(3, "Aider la garde autour des boutiques ouvertes", "Patrouille courte : protège les rares comptoirs actifs et améliore un peu la confiance locale.", true, "shop.city.repair.guard");
                screen.addOption(5, "Choisir une demande précise de réparation", "Infirmerie, marché, rations, faille mineure : petites tâches utiles et lisibles.", true, "shop.city.repair.orders");
            }
            else
            {
                screen.addOption(1, "Lire le calendrier des événements", "Affiche les événements réguliers inspirés des fiches : guilde, foire, savoir, missions, moissons.", true, "shop.city.events.calendar");
                screen.addOption(2, "Demander les risques du moment", "Rumeurs sur invasion, incendie, épidémie, faille magique, disparition ou crise économique.", true, "shop.city.events.risks");
                screen.addOption(3,
                    hasAnyCityEventToday(player) ? "Participer à l'événement actif" : "Aucun événement actif aujourd'hui",
                    hasAnyCityEventToday(player) ? "Activité courte : gain modeste, document, ressource ou réputation selon l'événement." : "Les affiches ne sont pas disponibles tous les jours.",
                    hasAnyCityEventToday(player),
                    "shop.city.events.participate");
            }
            screen.addOption(4, "Cotisations de ville", "Abonnements locaux : guilde, commerce, route. Conservé ici pour ne pas perdre l'ancien service.", true, "shop.city.subscriptions");
            screen.addOption(6, "Lire l'économie locale", "Résumé des besoins actuels, crise éventuelle et réaction légère selon la race.", true, "shop.city.economy.report");
            screen.addOption(7, "Lire le panneau ville / guilde", "Contexte local : prochaine affiche, tensions, pistes selon race. Aucun gain direct.", true, "shop.city.notice_board");

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(screen, "Choisis une action de ville.");
            Console::clear();

            if (choice == 0)
            {
                stay = false;
                continue;
            }

            if (choice == 4)
            {
                openSubscriptionMenu(
                    player,
                    "COTISATIONS DE VILLE",
                    {
                        {"guild_adventurer_standard_weekly", "Cotisation aventurier standard", 160, "Petits services de guilde, salle commune et paperasse locale couverts pendant 7 jours."},
                        {"merchant_cotisation_weekly", "Cotisation de la confrérie du commerce", 210, "Meilleurs papiers marchands, contrôles plus propres et aide de guichet pendant 7 jours."},
                        {"trade_route_weekly", "Pass de commerce hebdomadaire", 240, "Préparatifs de routes, caravanes et contrôles marchands simples pendant 7 jours."}
                    }
                );
                continue;
            }

            if (choice == 6)
            {
                showLocalEconomyReport(player);
                continue;
            }

            if (choice == 7)
            {
                showCityAndGuildNoticeBoard(player);
                continue;
            }

            if (repairDays > 0 && choice == 5)
            {
                openCityRepairWorkOrderMenu(player);
                continue;
            }

            if (repairDays > 0 && choice == 1)
            {
                std::vector<std::string> lines;
                if (player.getInventory().countMaterialById("rusted_metal_fragment") < 3
                    || player.getInventory().countMaterialById("worn_leather_piece") < 2)
                {
                    lines.push_back("Matériaux insuffisants : il faut Fragment de métal rouillé x3 et Morceau de cuir abîmé x2.");
                    lines.push_back("La ville ne demande pas de prix en or ici : elle veut des ressources utiles aux réparations.");
                    showShopResult("AIDE IMPOSSIBLE", "shop.city.repair.materials.failed", lines);
                    continue;
                }
                player.getInventory().removeMaterialQuantityById("rusted_metal_fragment", 3);
                player.getInventory().removeMaterialQuantityById("worn_leather_piece", 2);
                reduceCityRepairDays(player, 2);
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                lines.push_back("Tu remets de quoi clouer, sangler, rafistoler et sécuriser plusieurs devantures.");
                lines.push_back("Réparations réduites de 2 jour(s), sans transformer ça en quête infinie.");
                lines.push_back("Reçu obtenu : aide aux réparations x1. Note de réputation locale x1.");
                showLocalServiceResult("RÉPARATIONS AIDÉES", "shop.city.repair.materials.success", player, lines, 1);
            }
            else if (repairDays > 0 && choice == 2)
            {
                Random random;
                std::vector<std::string> lines = {
                    "Un contremaître te donne une liste simple : récupérer ce qui peut servir sans mourir pour une planche.",
                    "Ce n'est pas une grande aventure, mais une ville se relève avec ce genre de petites corvées."
                };
                player.getInventory().addMaterial(MaterialCatalog::createById("rusted_metal_fragment", random.between(1, 3)));
                player.getInventory().addMaterial(MaterialCatalog::createById("worn_leather_piece", random.between(1, 2)));
                if (random.between(1, 100) <= 35)
                {
                    player.getInventory().addMaterial(MaterialCatalog::createById("weak_repair_kit", 1));
                    lines.push_back("Un artisan te laisse un kit de réparation faible, cabossé mais utilisable.");
                }
                lines.push_back("Matériaux simples obtenus pour aider la reconstruction ou tes propres crafts.");
                showLocalServiceResult("RÉCOLTE DE CHANTIER", "shop.city.repair.gather.success", player, lines, 1);
            }
            else if (repairDays > 0 && choice == 3)
            {
                std::vector<std::string> lines = {
                    "Tu ne sauves pas la ville à toi seul, mais ta présence évite quelques vols et panique autour des rares comptoirs ouverts.",
                    "Les marchands notent ton nom pour une bonne raison cette fois."
                };
                player.getInventory().addMaterial(MaterialCatalog::createById("city_repair_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("city_service_stamp", 1));
                player.getInventory().earnEconomyUnits(18 + player.getLevel());
                player.refreshCurrencyTitles();
                if (cityRepairDaysRemaining(player) <= 2)
                {
                    reduceCityRepairDays(player, 1);
                    lines.push_back("La surveillance accélère la réouverture d'un secteur : réparations réduites de 1 jour.");
                }
                lines.push_back("Récompense de patrouille : " + Money::formatEconomyUnits(18 + player.getLevel()) + ".");
                showLocalServiceResult("PATROUILLE DE RÉPARATION", "shop.city.repair.guard.success", player, lines, 1);
            }
            else if (repairDays <= 0 && choice == 1)
            {
                const CityEventOfDay event = cityEventForTodayOrNext(player);
                std::vector<std::string> calendarLines = {
                    "Événements réguliers possibles : Tournoi de guilde, Chasse aux monstres, Grande foire marchande, Journée du savoir.",
                    "Autres affiches : Bourse aux missions, Cérémonie d'honneur, Jeux du village, Fête des moissons / solstice.",
                    "Rythme corrigé : environ un événement régulier par semaine. Un second événement exceptionnel peut apparaître, mais sa chance baisse aussi si d'autres événements viennent déjà d'occuper la ville.",
                    "Sécurité anti-farm : la présence d'un événement est déterminée par le calendrier, pas en rouvrant le menu en boucle.",
                    royalBonusChanceContextLine(player, safeCityDay(player))
                };
                if (hasAnyCityEventToday(player))
                {
                    calendarLines.push_back(std::string("Aujourd'hui : ") + (isRoyalBonusCityEvent(event) ? "affiche exceptionnelle — " : "affiche régulière — ") + event.name + " — " + event.mood);
                }
                else
                {
                    calendarLines.push_back("Aujourd'hui : aucune affiche active. Prochaine affiche dans " + std::to_string(daysUntilNextAnyCityEvent(player)) + " jour(s) : " + event.name + ".");
                }
                calendarLines.push_back("Villes douteuses : un Festival de l'Ombre peut exister, mais plutôt comme événement rare et dangereux.");
                showShopResult(
                    "CALENDRIER DE VILLE",
                    "shop.city.events.calendar",
                    calendarLines
                );
            }
            else if (repairDays <= 0 && choice == 3)
            {
                participateInCityEventOfDay(player);
            }
            else if (repairDays <= 0 && choice == 2)
            {
                std::vector<std::string> riskLines = {
                    "Malheurs rares possibles : invasion de monstres, épidémie ou malédiction, incendie, faille magique, disparition d'un notable, secte, guerre de guildes, crise économique.",
                    "Si une défense de ville échoue, la conséquence n'est pas juste du texte : plusieurs boutiques ferment pendant les réparations.",
                    "Pendant ce temps, les demandes locales se concentrent sur réparation, garde, récolte et remise en route des commerces.",
                    royalBonusChanceContextLine(player, safeCityDay(player)),
                    "Remise de défense : si la ville est sauvée, les commerçants remercient un peu, mais jamais plus de 10 jours."
                };
                const int recentEventDays = player.getInventory().countMaterialById("city_event_recent_days_marker");
                if (recentEventDays > 0)
                {
                    riskLines.push_back("Organisation : événement récent encore présent pendant " + std::to_string(recentEventDays) + " jour(s). Le bureau évite d'empiler les animations.");
                }
                showShopResult("RISQUES DU MOMENT", "shop.city.events.risks", riskLines);
            }
        }
    }

    void openLodgingServiceMenu(Player& player)
    {
        bool stayInServices = true;

        while (stayInServices)
        {
            MenuScreen screen("AUBERGE DU REPOS BRUYANT", "shop.lodging.services");
            screen.addLine("Tavia propose des services utiles, mais jamais gratuits en temps : manger ou dormir peut faire avancer les délais.");
            screen.addLine("PV : " + std::to_string(player.getHp()) + "/" + std::to_string(player.getMaxHp()));
            screen.addLine("Temps : " + worldTimeLineForPlayer(player));
            if (subscriptionCoversService(player, "lodging"))
            {
                screen.addLine("Abonnement : repas/chambre simple couverts par la période active.");
            }
            if (subscriptionCoversService(player, "stable"))
            {
                screen.addLine("Abonnement : écurie/relais couvert par la période active.");
            }
            screen.addLine(serviceCostLine(player, "warm_meal_voucher", "Bon de repas chaud", 12));
            screen.addLine(serviceCostLine(player, "lodging_bed_token", "Bon de lit d'auberge", 24));
            screen.addLine(serviceCostLine(player, "stable_stall_ticket", "Ticket d'écurie", 30));
            screen.addLine(serviceCostLine(player, "rental_mount_voucher", "Bon de monture", 65));
            screen.addLine(serviceCostLine(player, "stable_box_reservation", "Réservation de box", 44));
            if (player.getInventory().countMaterialById("owned_mount_registration") > 0)
            {
                const bool hasReinforcedSaddle = player.getInventory().countMaterialById("stable_saddle_upgrade") > 0;
                const bool hasMinorInjury = player.getInventory().countMaterialById("mount_minor_injury_marker") > 0;
                const int fatigueLimit = hasReinforcedSaddle ? 4 : 3;
                const int bond = std::min(3, player.getInventory().countMaterialById("mount_bond_marker"));
                const bool hasAnyAdvancedMountCare = player.getInventory().countMaterialById("mount_comfort_bridle") > 0
                    || player.getInventory().countMaterialById("mount_weather_blanket") > 0
                    || player.getInventory().countMaterialById("mount_pack_harness") > 0
                    || player.getInventory().countMaterialById("mount_road_shoes") > 0
                    || player.getInventory().countMaterialById("mount_route_memory_marker") > 0
                    || player.getInventory().countMaterialById("mount_surefoot_training_marker") > 0;
                screen.addLine("Monture personnelle : enregistrée | fatigue " + std::to_string(player.getInventory().countMaterialById("mount_fatigue_marker")) + "/" + std::to_string(fatigueLimit)
                    + " | lien " + std::to_string(bond) + "/3"
                    + (hasReinforcedSaddle ? " | selle renforcée" : "")
                    + (hasAnyAdvancedMountCare ? " | entretien/équipement avancé passif" : "")
                    + (hasMinorInjury ? " | blessure légère" : ""));
            }
            else
            {
                screen.addLine("Monture personnelle : aucune. Une monture durable coûte cher, mais évite de relouer à chaque long trajet.");
            }
            screen.addOption(0, "Retour", "Revenir au comptoir de l'auberge.", true, "shop.lodging.back");
            screen.addOption(1, "Manger un repas chaud — 12 cuivre", "Soin léger, consomme 1 segment de journée. Prix affiché avant validation.", true, "shop.lodging.meal");
            screen.addOption(2, "Dormir dans une chambre simple — 24 cuivre", "Une nuit simple aide, mais ne dépasse pas 50% PV. Prix affiché avant validation.", true, "shop.lodging.sleep");
            screen.addOption(3, "Écouter les rumeurs de comptoir", "Indice de ville sans récompense directe, consomme 1 segment.", true, "shop.lodging.rumors");
            screen.addOption(4, "Préparer une place d'écurie — 30 cuivre", "Stabilise monture, sacoches ou stockage court pour les quêtes de relais.", true, "shop.lodging.stable");
            screen.addOption(5, "Préparer sacoches et charge — 32 cuivre", "Préparation utile pour réduire un déplacement de biome plus tard.", true, "shop.lodging.saddlebags");
            screen.addOption(6, "Déposer une charge à l'écurie — 18 cuivre", "Dépôt temporaire utile pour certains services de ville/relais.", true, "shop.lodging.storage");
            screen.addOption(7, "Louer une monture de route — 65 cuivre", "Préparation plus forte pour les distances longues, consomme 1 segment.", true, "shop.lodging.mount");
            screen.addOption(8, "Réserver un box sécurisé — 44 cuivre", "Stockage/box plus sérieux, utile pour quêtes d'écurie et relais.", true, "shop.lodging.box");
            screen.addOption(9, "Abonnements de l'auberge", "Forfaits de 7 jours : repas, lit simple, écurie ou cotisation de guilde.", true, "shop.lodging.subscriptions");
            screen.addOption(10, "Enregistrer une monture personnelle — 420 cuivre", "Achat/acte durable : utile sur plusieurs trajets, mais la monture peut fatiguer.", true, "shop.lodging.owned_mount");
            screen.addOption(11, "Soin et repos de monture — 62 cuivre", "Retire la fatigue accumulée par une monture personnelle.", true, "shop.lodging.mount_rest");
            screen.addOption(12, "Examiner la monture personnelle", "Affiche son état, sa limite de fatigue et les conseils d'écurie sans consommer de temps.", player.getInventory().countMaterialById("owned_mount_registration") > 0, "shop.lodging.mount_check");
            if (player.getInventory().countMaterialById("owned_mount_registration") > 0)
            {
                screen.addLine("Gestion simplifiée : les réglages avancés de monture restent passifs dans l'inventaire, mais ne prennent plus autant de place dans le menu.");
                screen.addLine("Actions conservées : enregistrer, examiner, reposer/soigner. Pas besoin de jouer à Équitation Simulator pour profiter du bonus de route.");
            }
            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Choisis un service d'auberge, ou 0 pour revenir."
            );

            if (choice == 0)
            {
                stayInServices = false;
                continue;
            }

            if (choice == 1)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "lodging", "warm_meal_voucher", "Bon de repas chaud", 12, lines))
                {
                    showShopResult("REPAS REFUSÉ", "shop.lodging.meal.failed", lines);
                    continue;
                }

                const int beforeHp = player.getHp();
                const int healAmount = std::max(1, player.getMaxHp() / 5);
                player.heal(healAmount);
                lines.push_back("Repas pris : soupe chaude, pain correct et aucune assiette maudite repérée.");
                lines.push_back("PV récupérés : " + std::to_string(player.getHp() - beforeHp)
                    + " (" + std::to_string(beforeHp) + " -> " + std::to_string(player.getHp()) + ").");
                showLocalServiceResult("REPAS TERMINÉ", "shop.lodging.meal.success", player, lines, 1);
            }
            else if (choice == 2)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "lodging", "lodging_bed_token", "Bon de lit d'auberge", 24, lines))
                {
                    showShopResult("NUIT REFUSÉE", "shop.lodging.sleep.failed", lines);
                    continue;
                }

                const int beforeHp = player.getHp();
                const int targetHp = std::max(1, player.getMaxHp() * 50 / 100);
                if (player.getHp() < targetHp)
                {
                    player.heal(targetHp - player.getHp());
                }
                lines.push_back("Repos : chambre simple, couverture honnête et porte qui ferme presque bien.");
                lines.push_back("Plafond : une nuit simple ne soigne pas au-delà de 50% des PV.");
                lines.push_back("PV récupérés : " + std::to_string(player.getHp() - beforeHp)
                    + " (" + std::to_string(beforeHp) + " -> " + std::to_string(player.getHp()) + ").");
                lines.push_back("Rappel : dormir peut faire échouer les quêtes urgentes si la date limite passe pendant la nuit.");
                showLocalServiceResult("NUIT TERMINÉE", "shop.lodging.sleep.success", player, lines, 2);
            }
            else if (choice == 3)
            {
                std::vector<std::string> lines;
                lines.push_back("Tavia parle bas : les bons d'auberge, tickets d'écurie et reçus de route peuvent compter plus qu'une petite prime.");
                lines.push_back("Elle conseille de garder au moins une preuve de service quand une quête mentionne relais, groupe, chambre ou départ à l'aube.");
                lines.push_back("Aucune récompense directe : tu gagnes surtout une piste propre, pas un objet gratuit.");
                showLocalServiceResult("RUMEURS DE COMPTOIR", "shop.lodging.rumors", player, lines, 1);
            }
            else if (choice == 4)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_stall_ticket", "Ticket d'écurie", 30, lines))
                {
                    showShopResult("ÉCURIE REFUSÉE", "shop.lodging.stable.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("travel_pass_note", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("travel_distance_mark", 1));
                lines.push_back("Écurie préparée : box réservé, sacoches notées, départ un peu moins chaotique.");
                lines.push_back("Preuves obtenues : Note de pass de voyage x1, Marque de distance de trajet x1.");
                lines.push_back("Le registre d'écurie garde cette préparation comme preuve de départ organisé.");
                showLocalServiceResult("ÉCURIE PRÉPARÉE", "shop.lodging.stable.success", player, lines, 1);
            }
            else if (choice == 5)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_stall_ticket", "Ticket d'écurie", 32, lines))
                {
                    showShopResult("SACOCHES REFUSÉES", "shop.lodging.saddlebags.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("prepared_saddlebags", 1));
                lines.push_back("Sacoches préparées : charge répartie, sangles vérifiées, rien ne pendouille au mauvais endroit.");
                lines.push_back("Preuve obtenue : Sacoches préparées x1.");
                lines.push_back("Les sacoches répartissent la charge pour les voyages où la distance compte vraiment.");
                showLocalServiceResult("SACOCHES PRÉPARÉES", "shop.lodging.saddlebags.success", player, lines, 1);
            }
            else if (choice == 6)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_stall_ticket", "Ticket d'écurie", 18, lines))
                {
                    showShopResult("DÉPÔT REFUSÉ", "shop.lodging.storage.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("temporary_stable_storage", 1));
                lines.push_back("Dépôt enregistré : la charge est gardée au sec, loin des bols de soupe et des bardes.");
                lines.push_back("Preuve obtenue : Dépôt temporaire d'écurie x1.");
                lines.push_back("Le reçu prouve qu'une charge a été confiée à l'écurie.");
                showLocalServiceResult("DÉPÔT D'ÉCURIE", "shop.lodging.storage.success", player, lines, 1);
            }
            else if (choice == 7)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "rental_mount_voucher", "Bon de monture", 65, lines))
                {
                    showShopResult("MONTURE REFUSÉE", "shop.lodging.mount.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("rental_mount_voucher", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("loaded_pack_saddle", 1));
                lines.push_back("Monture louée : animal nourri, bride vérifiée, selle chargée sans angle idiot.");
                lines.push_back("Preuves obtenues : Bon de monture de location x1, Selle de bât chargée x1.");
                lines.push_back("La monture est destinée aux biomes vastes et aux longues explorations.");
                showLocalServiceResult("MONTURE PRÊTE", "shop.lodging.mount.success", player, lines, 1);
            }
            else if (choice == 8)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_box_reservation", "Réservation de box", 44, lines))
                {
                    showShopResult("BOX REFUSÉ", "shop.lodging.box.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("stable_box_reservation", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("temporary_stable_storage", 1));
                lines.push_back("Box réservé : une place propre, une serrure honnête et une note claire dans le registre.");
                lines.push_back("Preuves obtenues : Réservation de box sécurisé x1, Dépôt temporaire d'écurie x1.");
                lines.push_back("La réservation protège une cargaison légère ou un départ reporté.");
                showLocalServiceResult("BOX RÉSERVÉ", "shop.lodging.box.success", player, lines, 1);
            }
            else if (choice == 10)
            {
                std::vector<std::string> lines;
                if (player.getInventory().countMaterialById("owned_mount_registration") > 0)
                {
                    lines.push_back("Tu possèdes déjà une monture personnelle enregistrée.");
                    lines.push_back("Elle peut aider plusieurs longs trajets, mais elle accumule de la fatigue et doit parfois se reposer.");
                    showShopResult("MONTURE DÉJÀ ENREGISTRÉE", "shop.lodging.owned_mount.already", lines);
                    continue;
                }

                const int price = 240;
                if (!player.getInventory().spendEconomyUnits(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(price) + ".");
                    lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
                    showShopResult("MONTURE REFUSÉE", "shop.lodging.owned_mount.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("owned_mount_registration", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("stable_saddle_upgrade", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_temperament_calm", 1));
                lines.push_back("Monture personnelle enregistrée : l'écurie connaît maintenant ton animal, ses habitudes et son box de base.");
                lines.push_back("Preuves obtenues : Acte de monture personnelle x1, Selle renforcée de route x1, Tempérament de monture calme x1.");
                lines.push_back("Règle : la monture aide durablement les longs trajets, mais accumule de la fatigue après usage.");
                showLocalServiceResult("MONTURE ENREGISTRÉE", "shop.lodging.owned_mount.success", player, lines, 1);
            }
            else if (choice == 11)
            {
                std::vector<std::string> lines;
                const int fatigue = player.getInventory().countMaterialById("mount_fatigue_marker");
                const bool hasReinforcedSaddle = player.getInventory().countMaterialById("stable_saddle_upgrade") > 0;
                const int fatigueLimit = hasReinforcedSaddle ? 4 : 3;
                if (fatigue <= 0)
                {
                    lines.push_back("Aucune fatigue de monture à retirer pour le moment.");
                    lines.push_back("Tavia conseille quand même de garder de quoi payer un repos avant les grandes routes.");
                    showShopResult("MONTURE REPOSÉE", "shop.lodging.mount_rest.none", lines);
                    continue;
                }

                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "mount_rest_care", "Soin et repos de monture", 45, lines))
                {
                    showShopResult("REPOS REFUSÉ", "shop.lodging.mount_rest.failed", lines);
                    continue;
                }

                player.getInventory().removeMaterialQuantityById("mount_fatigue_marker", fatigue);
                lines.push_back("Repos de monture : nourriture, eau, pansage, vérification des fers et vrai silence loin des caravanes.");
                lines.push_back("Fatigue retirée : " + std::to_string(fatigue) + "/" + std::to_string(fatigueLimit) + ".");
                showLocalServiceResult("MONTURE REPOSÉE", "shop.lodging.mount_rest.success", player, lines, 1);
            }
            else if (choice == 12)
            {
                std::vector<std::string> lines;
                const int fatigue = player.getInventory().countMaterialById("mount_fatigue_marker");
                const int bond = std::min(3, player.getInventory().countMaterialById("mount_bond_marker"));
                const bool hasName = player.getInventory().countMaterialById("mount_name_tag") > 0;
                const bool hasTemperament = player.getInventory().countMaterialById("mount_temperament_calm") > 0;
                const bool hasReinforcedSaddle = player.getInventory().countMaterialById("stable_saddle_upgrade") > 0;
                const bool hasComfortBridle = player.getInventory().countMaterialById("mount_comfort_bridle") > 0;
                const bool hasWeatherBlanket = player.getInventory().countMaterialById("mount_weather_blanket") > 0;
                const bool hasPackHarness = player.getInventory().countMaterialById("mount_pack_harness") > 0;
                const bool hasRoadShoes = player.getInventory().countMaterialById("mount_road_shoes") > 0;
                const int surefoot = std::min(2, player.getInventory().countMaterialById("mount_surefoot_training_marker"));
                const int routeMemory = std::min(2, player.getInventory().countMaterialById("mount_route_memory_marker"));
                const bool hasMinorInjury = player.getInventory().countMaterialById("mount_minor_injury_marker") > 0;
                const int fatigueLimit = hasReinforcedSaddle ? 4 : 3;
                lines.push_back("Monture personnelle : enregistrée à l'écurie.");
                lines.push_back(std::string("Nom de terrain : ") + (hasName ? "noté par Tavia." : "pas encore noté."));
                lines.push_back(std::string("Tempérament : ") + (hasTemperament ? "calme, fiable sur route." : "encore mal observé."));
                lines.push_back("Fatigue actuelle : " + std::to_string(fatigue) + "/" + std::to_string(fatigueLimit) + ".");
                lines.push_back("Lien actuel : " + std::to_string(bond) + "/3.");
                lines.push_back(hasReinforcedSaddle ? "Selle renforcée : limite de fatigue augmentée et meilleure réduction sur longues routes." : "Selle renforcée : absente. Une amélioration peut rendre les longues routes plus fiables.");
                lines.push_back(hasComfortBridle ? "Bridon confortable : installé, utile pour les trajets propres et répétés." : "Bridon confortable : absent, les longues routes restent plus rugueuses.");
                lines.push_back(hasWeatherBlanket ? "Couverture météo : installée, utile contre pluie, froid léger et bivouacs." : "Couverture météo : absente, les routes froides ou humides restent moins propres.");
                lines.push_back(hasPackHarness ? "Harnais de bât : ajusté, les petites charges pèsent moins sur les longs trajets." : "Harnais de bât : absent, les sacoches longues restent plus fatigantes.");
                lines.push_back(hasRoadShoes ? "Ferrage de route : installé, utile sur les très longues distances." : "Ferrage de route : absent, mais ce n'est pas indispensable pour jouer normalement.");
                lines.push_back("Mémoire de route : " + std::to_string(routeMemory) + "/2. Assurance : " + std::to_string(surefoot) + "/2.");
                if (hasMinorInjury)
                {
                    lines.push_back("Blessure légère : présente. L'écurie déconseille toute longue sortie avant soin.");
                }
                else if (fatigue >= fatigueLimit)
                {
                    lines.push_back("Conseil de Tavia : repos obligatoire avant de refaire confiance à l'animal sur une longue sortie.");
                }
                else if (fatigue >= fatigueLimit - 1)
                {
                    lines.push_back("Conseil de Tavia : encore utilisable, mais le prochain départ risque de la bloquer.");
                }
                else
                {
                    lines.push_back("Conseil de Tavia : état correct pour un trajet, mais pas pour enchaîner sans repos.");
                }
                showShopResult("ÉTAT DE LA MONTURE", "shop.lodging.mount_check", lines);
            }
            else if (choice == 13)
            {
                std::vector<std::string> lines;
                const int price = 8;
                if (!player.getInventory().spendEconomyUnits(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(price) + ".");
                    lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
                    showShopResult("NOM REFUSÉ", "shop.lodging.mount_name.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_name_tag", 1));
                lines.push_back("Tavia grave un nom de terrain sur une petite plaque d'écurie.");
                lines.push_back("Pas besoin de système de saisie lourd : le jeu retient surtout que la monture est reconnue comme compagnon, pas comme simple location.");
                showShopResult("NOM DE MONTURE NOTÉ", "shop.lodging.mount_name.success", lines);
            }
            else if (choice == 14)
            {
                std::vector<std::string> lines;
                const int bond = std::min(3, player.getInventory().countMaterialById("mount_bond_marker"));
                if (bond >= 3)
                {
                    lines.push_back("Lien déjà au maximum actuel : 3/3.");
                    showShopResult("LIEN STABLE", "shop.lodging.mount_bond.max", lines);
                    continue;
                }
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "mount_grooming_kit", "Kit de pansage de monture", 28, lines))
                {
                    showShopResult("LIEN REFUSÉ", "shop.lodging.mount_bond.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_bond_marker", 1));
                lines.push_back("Tavia te laisse marcher avec l'animal, brosser les zones sensibles et répéter les arrêts sans le brusquer.");
                lines.push_back("Lien de monture : " + std::to_string(bond + 1) + "/3.");
                showLocalServiceResult("LIEN DE MONTURE", "shop.lodging.mount_bond.success", player, lines, 1);
            }
            else if (choice == 15)
            {
                std::vector<std::string> lines;
                if (player.getInventory().countMaterialById("mount_minor_injury_marker") <= 0)
                {
                    lines.push_back("Aucune blessure légère de monture détectée.");
                    showShopResult("SOIN INUTILE", "shop.lodging.mount_injury.none", lines);
                    continue;
                }
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "mount_rest_care", "Soin et repos de monture", 65, lines))
                {
                    showShopResult("SOIN REFUSÉ", "shop.lodging.mount_injury.failed", lines);
                    continue;
                }
                const int injuries = player.getInventory().countMaterialById("mount_minor_injury_marker");
                player.getInventory().removeMaterialQuantityById("mount_minor_injury_marker", injuries);
                lines.push_back("L'écurie vérifie les appuis, pose un baume simple et interdit de repartir en sprintant immédiatement.");
                lines.push_back("Blessure légère retirée.");
                showLocalServiceResult("MONTURE SOIGNÉE", "shop.lodging.mount_injury.success", player, lines, 1);
            }
            else if (choice == 16)
            {
                std::vector<std::string> lines;
                const int price = 86;
                if (!player.getInventory().spendEconomyUnits(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(price) + ".");
                    lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
                    showShopResult("BRIDON REFUSÉ", "shop.lodging.mount_bridle.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_comfort_bridle", 1));
                lines.push_back("Bridon confortable installé : pas un turbo magique, juste moins de frottements et de panique sur les départs répétés.");
                lines.push_back("Effet : avec un bon lien, certains trajets courts peuvent ne pas ajouter de fatigue.");
                showLocalServiceResult("BRIDON INSTALLÉ", "shop.lodging.mount_bridle.success", player, lines, 1);
            }
            else if (choice == 17)
            {
                std::vector<std::string> lines;
                const int price = 74;
                if (!player.getInventory().spendEconomyUnits(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(price) + ".");
                    lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
                    showShopResult("COUVERTURE REFUSÉE", "shop.lodging.mount_blanket.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_weather_blanket", 1));
                lines.push_back("Couverture météo installée : Tavia ajuste les attaches pour éviter que le tissu tourne sous la selle.");
                lines.push_back("Effet : aide la monture dans les routes froides/humides et limite une partie de l'exposition météo en exploration.");
                showLocalServiceResult("COUVERTURE INSTALLÉE", "shop.lodging.mount_blanket.success", player, lines, 1);
            }
            else if (choice == 18)
            {
                std::vector<std::string> lines;
                const int price = 92;
                if (!player.getInventory().spendEconomyUnits(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(price) + ".");
                    lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
                    showShopResult("HARNAIS REFUSÉ", "shop.lodging.mount_harness.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_pack_harness", 1));
                lines.push_back("Harnais de bât ajusté : les charges ne tirent plus toutes du même côté comme une mauvaise blague de gobelin.");
                lines.push_back("Effet : les longues explorations chargées deviennent plus propres, avec moins de risque de gêne légère.");
                showLocalServiceResult("HARNAIS AJUSTÉ", "shop.lodging.mount_harness.success", player, lines, 1);
            }
            else if (choice == 19)
            {
                std::vector<std::string> lines;
                const int routeMemory = std::min(2, player.getInventory().countMaterialById("mount_route_memory_marker"));
                if (routeMemory >= 2)
                {
                    lines.push_back("Mémoire de route déjà au maximum actuel : 2/2.");
                    showShopResult("ROUTE DÉJÀ CONNUE", "shop.lodging.mount_route_memory.max", lines);
                    continue;
                }
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_stall_ticket", "Ticket d'écurie", 34, lines))
                {
                    showShopResult("ROUTE REFUSÉE", "shop.lodging.mount_route_memory.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_route_memory_marker", 1));
                lines.push_back("Tavia fait répéter un départ, deux arrêts et le retour au relais. La monture comprend mieux le rythme attendu.");
                lines.push_back("Mémoire de route : " + std::to_string(routeMemory + 1) + "/2.");
                showLocalServiceResult("ROUTE RÉPÉTÉE", "shop.lodging.mount_route_memory.success", player, lines, 1);
            }

            else if (choice == 20)
            {
                std::vector<std::string> lines;
                const int surefoot = std::min(2, player.getInventory().countMaterialById("mount_surefoot_training_marker"));
                if (surefoot >= 2)
                {
                    lines.push_back("Assurance déjà au maximum actuel : 2/2.");
                    showShopResult("APPUIS STABLES", "shop.lodging.mount_surefoot.max", lines);
                    continue;
                }
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "mount_grooming_kit", "Kit de pansage de monture", 32, lines))
                {
                    showShopResult("SÉANCE REFUSÉE", "shop.lodging.mount_surefoot.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_surefoot_training_marker", 1));
                lines.push_back("Tavia fait passer la monture sur des planches, des pierres plates et un petit pont qui craque juste assez pour apprendre sans traumatiser.");
                lines.push_back("Assurance de monture : " + std::to_string(surefoot + 1) + "/2.");
                showLocalServiceResult("APPUIS TRAVAILLÉS", "shop.lodging.mount_surefoot.success", player, lines, 1);
            }
            else if (choice == 21)
            {
                std::vector<std::string> lines;
                const int price = 88;
                if (!player.getInventory().spendEconomyUnits(price))
                {
                    lines.push_back("Paiement refusé : il faut " + Money::formatEconomyUnits(price) + ".");
                    lines.push_back("Argent disponible : " + player.getInventory().getWalletLine() + ".");
                    showShopResult("FERRAGE REFUSÉ", "shop.lodging.mount_road_shoes.failed", lines);
                    continue;
                }
                player.getInventory().addMaterial(MaterialCatalog::createById("mount_road_shoes", 1));
                lines.push_back("Le palefrenier ajuste les fers sans transformer la monture en machine de guerre : juste de quoi éviter les petites douleurs de route.");
                lines.push_back("Effet : moins de risques de blessure légère quand une longue sortie s'enchaîne mal.");
                showLocalServiceResult("FERRAGE INSTALLÉ", "shop.lodging.mount_road_shoes.success", player, lines, 1);
            }
            else if (choice == 9)
            {
                openSubscriptionMenu(
                    player,
                    "ABONNEMENTS DE L'AUBERGE",
                    {
                        {"lodging_modest_weekly", "Forfait hebdo auberge modeste", 110, "Couvre les repas simples et la chambre simple pendant 7 jours."},
                        {"stable_relay_weekly", "Forfait écurie et relais", 135, "Couvre les préparations d'écurie simples pendant 7 jours."},
                        {"guild_adventurer_standard_weekly", "Cotisation aventurier standard", 160, "Cotisation de guilde adaptée : lit partagé, salle commune et petits services locaux pendant 7 jours."}
                    }
                );
            }
        }
    }

    void openTransportServiceMenu(Player& player)
    {
        bool stayInServices = true;

        while (stayInServices)
        {
            MenuScreen screen("RELAIS DES ROUTES", "shop.transport.services");
            screen.addLine("Noro transforme les papiers de transport en vrais préparatifs : route, écurie, bagages, péages et convoi.");
            screen.addLine("Temps : " + worldTimeLineForPlayer(player));
            if (subscriptionCoversService(player, "transport"))
            {
                screen.addLine("Abonnement : les préparatifs de route simples sont couverts par la période active.");
            }
            screen.addLine(serviceCostLine(player, "stable_stall_ticket", "Ticket d'écurie", 30));
            screen.addLine(serviceCostLine(player, "caravan_seat_ticket", "Place de caravane", 60));
            screen.addLine(serviceCostLine(player, "guarded_transport_pass", "Pass de transport gardé", 105));
            screen.addOption(0, "Retour", "Revenir au comptoir du relais.", true, "shop.transport.back");
            screen.addOption(1, "Préparer monture et bagages", "Consomme 1 segment et produit une preuve de route simple.", true, "shop.transport.stable");
            screen.addOption(2, "Préparer un départ de caravane", "Consomme 2 segments et produit un reçu de péage.", true, "shop.transport.caravan");
            screen.addOption(3, "Organiser un convoi gardé", "Consomme 2 segments et produit une preuve locale solide.", true, "shop.transport.guarded");
            screen.addOption(4, "Faire relire l'itinéraire", "Produit une note d'éclaireur de route pour éviter un détour futur.", true, "shop.transport.scout");
            screen.addOption(5, "Obtenir un badge de route du relais", "Preuve plus forte qu'un reçu simple, consomme 1 segment.", true, "shop.transport.badge");
            screen.addOption(6, "Abonnements de transport", "Pass de 7 jours : écurie, relais, commerce ou route gardée.", true, "shop.transport.subscriptions");

            Console::clear();
            const int choice = TerminalInterface::askMenuChoiceFromOptions(
                screen,
                "Choisis un service de relais, ou 0 pour revenir."
            );

            if (choice == 0)
            {
                stayInServices = false;
                continue;
            }

            if (choice == 1)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "stable", "stable_stall_ticket", "Ticket d'écurie", 30, lines))
                {
                    showShopResult("PRÉPARATION REFUSÉE", "shop.transport.stable.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("route_toll_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("travel_distance_mark", 1));
                lines.push_back("Monture et bagages préparés : le relais sait maintenant quoi charger, garder et laisser respirer.");
                lines.push_back("Preuves obtenues : Reçu de péage de route x1, Marque de distance de trajet x1.");
                showLocalServiceResult("RELAIS PRÉPARÉ", "shop.transport.stable.success", player, lines, 1);
            }
            else if (choice == 2)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "transport", "caravan_seat_ticket", "Place de caravane", 60, lines))
                {
                    showShopResult("CARAVANE REFUSÉE", "shop.transport.caravan.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("route_toll_receipt", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("travel_distance_mark", 2));
                lines.push_back("Départ préparé : nom inscrit, place notée, caisse attachée avec une confiance relative.");
                lines.push_back("Preuves obtenues : Reçu de péage de route x1, Marque de distance de trajet x2.");
                lines.push_back("Le trajet n'est pas joué automatiquement : ce service prépare surtout les quêtes et justificatifs de transport.");
                showLocalServiceResult("CARAVANE PRÉPARÉE", "shop.transport.caravan.success", player, lines, 2);
            }
            else if (choice == 3)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "transport", "guarded_transport_pass", "Pass de transport gardé", 105, lines))
                {
                    showShopResult("CONVOI REFUSÉ", "shop.transport.guarded.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("local_reputation_note", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("travel_distance_mark", 2));
                lines.push_back("Convoi préparé : garde prévu, itinéraire noté, et quelqu'un d'autre que toi sera payé pour surveiller les roues.");
                lines.push_back("Preuves obtenues : Note de réputation locale x1, Marque de distance de trajet x2.");
                lines.push_back("Ce service coûte cher mais peut aider les demandes de haut niveau liées aux routes gardées.");
                showLocalServiceResult("CONVOI GARDÉ PRÉPARÉ", "shop.transport.guarded.success", player, lines, 2);
            }
            else if (choice == 4)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "transport", "route_toll_receipt", "Reçu de péage de route", 38, lines))
                {
                    showShopResult("ITINÉRAIRE REFUSÉ", "shop.transport.scout.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("route_scout_note", 1));
                lines.push_back("Itinéraire relu : Noro note un raccourci, une route fermée et un relais à éviter si tu tiens à tes bottes.");
                lines.push_back("Preuve obtenue : Note d'éclaireur de route x1.");
                lines.push_back("Cette préparation facilite les longues sorties vers les biomes éloignés.");
                showLocalServiceResult("ITINÉRAIRE RELU", "shop.transport.scout.success", player, lines, 1);
            }
            else if (choice == 5)
            {
                std::vector<std::string> lines;
                if (!payServiceWithSubscriptionVoucherOrGold(player, "transport", "relay_route_badge", "Badge de route du relais", 55, lines))
                {
                    showShopResult("BADGE REFUSÉ", "shop.transport.badge.failed", lines);
                    continue;
                }

                player.getInventory().addMaterial(MaterialCatalog::createById("relay_route_badge", 1));
                player.getInventory().addMaterial(MaterialCatalog::createById("route_scout_note", 1));
                lines.push_back("Badge signé : le relais confirme que ton passage est préparé et que ton itinéraire n'est pas improvisé au hasard.");
                lines.push_back("Preuves obtenues : Badge de route du relais x1, Note d'éclaireur de route x1.");
                lines.push_back("Ce justificatif accompagne les routes contrôlées et les transports préparés.");
                showLocalServiceResult("BADGE DE ROUTE", "shop.transport.badge.success", player, lines, 1);
            }
            else if (choice == 6)
            {
                openSubscriptionMenu(
                    player,
                    "ABONNEMENTS DU RELAIS",
                    {
                        {"stable_relay_weekly", "Forfait écurie et relais", 135, "Préparations de monture, box et bagages simples pendant 7 jours."},
                        {"trade_route_weekly", "Pass de commerce hebdomadaire", 240, "Préparatifs de caravanes, routes et contrôles marchands simples pendant 7 jours."},
                        {"merchant_cotisation_weekly", "Cotisation de la confrérie du commerce", 210, "Protection contractuelle légère et meilleurs papiers de ville pendant 7 jours."}
                    }
                );
            }
        }
    }

} // namespace ShopCityServiceMenu
