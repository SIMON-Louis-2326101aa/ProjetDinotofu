// EN: QuestCatalog.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: QuestCatalog.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu.
// Description: Builds quest templates for guild and notable NPCs.

#include "quest/QuestCatalog.hpp"

#include <array>
#include <random>
#include <string>
#include <vector>

namespace
{
    struct RequestedMaterial
    {
        std::string id;
        std::string name;
        int baseQuantity;
        int minLevel;
    };

    Quest buildQuest(
        const std::string& id,
        const std::string& rank,
        const std::string& title,
        const std::string& origin,
        const std::string& client,
        const std::string& location,
        const std::string& objective,
        const std::string& objectiveType,
        const std::string& targetFamily,
        int experience,
        int gold,
        int target,
        bool guildQuest,
        const std::string& requiredMaterialId = "",
        const std::string& requiredMaterialName = "",
        int requiredMaterialQuantity = 0
    )
    {
        Quest quest;
        quest.id = id;
        quest.rank = rank;
        quest.title = title;
        quest.origin = origin;
        quest.client = client;
        quest.location = location;
        quest.objective = objective;
        quest.objectiveType = objectiveType;
        quest.targetFamily = targetFamily;
        quest.rewardExperience = experience;
        quest.rewardGold = gold;
        quest.requiredMaterialId = requiredMaterialId;
        quest.requiredMaterialName = requiredMaterialName;
        quest.requiredMaterialQuantity = requiredMaterialQuantity;
        quest.progress = 0;
        quest.target = target;
        quest.guildQuest = guildQuest;
        quest.accepted = true;
        quest.completed = false;
        quest.turnedIn = false;
        return quest;
    }

    std::mt19937& questGenerator()
    {
        static std::random_device device;
        static std::mt19937 generator(device());
        return generator;
    }

    // EN: randomBetween declares or implements a focused behavior used by this module.
    // FR: randomBetween déclare ou implémente un comportement précis utilisé par ce module.
    int randomBetween(int min, int max)
    {
        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(questGenerator());
    }

    // EN: chooseMaterial declares or implements a focused behavior used by this module.
    // FR: chooseMaterial déclare ou implémente un comportement précis utilisé par ce module.
    RequestedMaterial chooseMaterial(const std::vector<RequestedMaterial>& materials, int playerLevel)
    {
        std::vector<RequestedMaterial> available;

        for (const RequestedMaterial& material : materials)
        {
            if (playerLevel >= material.minLevel)
            {
                available.push_back(material);
            }
        }

        if (available.empty())
        {
            return materials.front();
        }

        return available[randomBetween(0, static_cast<int>(available.size()) - 1)];
    }

    std::string questIdWithMaterial(const std::string& prefix, const RequestedMaterial& material, int playerLevel)
    {
        return prefix + "_" + material.id + "_lvl_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(100, 999));
    }

    // EN: materialQuantity declares or implements a focused behavior used by this module.
    // FR: materialQuantity déclare ou implémente un comportement précis utilisé par ce module.
    int materialQuantity(const RequestedMaterial& material, int playerLevel)
    {
        int quantity = material.baseQuantity;

        if (playerLevel >= 5)
        {
            quantity++;
        }

        if (playerLevel >= 10 && material.baseQuantity <= 3)
        {
            quantity++;
        }

        return quantity;
    }

    // EN: rankPower declares or implements a focused behavior used by this module.
    // FR: rankPower déclare ou implémente un comportement précis utilisé par ce module.
    int rankPower(const std::string& rank)
    {
        if (rank == "F") return 1;
        if (rank == "E") return 2;
        if (rank == "D") return 3;
        if (rank == "C") return 4;
        if (rank == "B") return 5;
        if (rank == "A") return 7;
        if (rank == "S") return 10;
        return 1;
    }

    // EN: questExperience declares or implements a focused behavior used by this module.
    // FR: questExperience déclare ou implémente un comportement précis utilisé par ce module.
    int questExperience(const std::string& rank, int playerLevel, int target)
    {
        return 18 + playerLevel * 7 + target * 8 + rankPower(rank) * 14;
    }

    // EN: questGold declares or implements a focused behavior used by this module.
    // FR: questGold déclare ou implémente un comportement précis utilisé par ce module.
    int questGold(const std::string& rank, int playerLevel, int target)
    {
        return 14 + playerLevel * 5 + target * 7 + rankPower(rank) * 10;
    }

    std::string materialRank(int playerLevel, int minRankPower)
    {
        if (playerLevel >= 12 && minRankPower >= 5) return "B";
        if (playerLevel >= 8 && minRankPower >= 4) return "C";
        if (playerLevel >= 5) return "D";
        return "E";
    }

    std::string chooseText(const std::vector<std::string>& values)
    {
        if (values.empty())
        {
            return "";
        }

        return values[randomBetween(0, static_cast<int>(values.size()) - 1)];
    }

    std::string questId(const std::string& prefix, int playerLevel)
    {
        return prefix + "_lvl_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(1000, 9999));
    }

    struct GuildTemplate
    {
        std::string rank;
        std::string title;
        std::string objective;
        std::string type;
        std::string family;
        int target;
        int minLevel;
    };

    // EN: chooseGuildTemplate declares or implements a focused behavior used by this module.
    // FR: chooseGuildTemplate déclare ou implémente un comportement précis utilisé par ce module.
    GuildTemplate chooseGuildTemplate(const std::vector<GuildTemplate>& templates, int playerLevel)
    {
        std::vector<GuildTemplate> available;

        for (const GuildTemplate& questTemplate : templates)
        {
            if (playerLevel >= questTemplate.minLevel)
            {
                available.push_back(questTemplate);
            }
        }

        if (available.empty())
        {
            return templates.front();
        }

        return available[randomBetween(0, static_cast<int>(available.size()) - 1)];
    }

    // EN: buildGuildQuest declares or implements a focused behavior used by this module.
    // FR: buildGuildQuest déclare ou implémente un comportement précis utilisé par ce module.
    Quest buildGuildQuest(const std::string& idPrefix, int playerLevel, const GuildTemplate& questTemplate)
    {
        return buildQuest(
            questId(idPrefix, playerLevel),
            questTemplate.rank, questTemplate.title, "Guilde", "Maître de guilde", "Guilde",
            questTemplate.objective, questTemplate.type, questTemplate.family,
            questExperience(questTemplate.rank, playerLevel, questTemplate.target),
            questGold(questTemplate.rank, playerLevel, questTemplate.target),
            questTemplate.target, true
        );
    }

    struct BiomeQuestProfile
    {
        std::string location;
        std::string family;
        std::vector<RequestedMaterial> materials;
        std::vector<std::string> explorationObjectives;
        std::vector<std::string> combatObjectives;
    };

    // EN: profileForBiome declares or implements a focused behavior used by this module.
    // FR: profileForBiome déclare ou implémente un comportement précis utilisé par ce module.
    BiomeQuestProfile profileForBiome(const std::string& biomeName)
    {
        if (biomeName == "Forêt ancienne")
        {
            return {
                biomeName, "Forêt ancienne / créatures naturelles",
                {{"bitter_healing_leaf", "Feuille amère de soin", 4, 1}, {"mountain_blue_flower", "Fleur bleue de montagne", 2, 1}, {"wolf_fang", "Croc de loup", 2, 2}, {"beast_hide", "Peau de bête robuste", 1, 3}},
                {"Relever des traces propres sans arracher la zone aux racines.", "Cartographier les plantes utiles et éviter les zones où la forêt devient silencieuse.", "Récupérer des indices naturels sans provoquer les gardiens de mousse."},
                {"Éloigner les bêtes qui rôdent autour des sentiers sans ravager leur territoire.", "Repousser une menace locale liée aux ronces et aux crocs."}
            };
        }

        if (biomeName == "Montagne froide")
        {
            return {
                biomeName, "Montagne froide / bêtes de givre",
                {{"rusted_metal_fragment", "Fragment de métal rouillé", 4, 1}, {"mountain_blue_flower", "Fleur bleue de montagne", 2, 1}, {"arcane_dust", "Poussière arcanique", 1, 4}, {"draconic_scale_fragment", "Fragment d'écaille draconique", 1, 7}},
                {"Suivre les marques laissées dans la neige avant qu'elles ne disparaissent.", "Explorer un ancien passage gelé sans déclencher tout l'éboulement.", "Trouver une ressource froide assez stable pour être rapportée."},
                {"Repousser une créature adaptée au froid avant qu'elle ne descende vers les routes.", "Survivre à une embuscade dans une pente glacée."}
            };
        }

        if (biomeName == "Marais trouble")
        {
            return {
                biomeName, "Marais trouble / slimes et noyés",
                {{"slime_residue", "Résidu de slime", 4, 1}, {"arcane_dust", "Poussière arcanique", 2, 2}, {"cracked_bone", "Os fissuré", 2, 2}, {"unstable_core", "Noyau instable", 1, 7}},
                {"Prélever ce qui flotte encore sans tomber dans ce qui respire dessous.", "Marquer les flaques qui bougent toutes seules pour les prochains aventuriers.", "Récupérer un échantillon collant sans le laisser manger le sac."},
                {"Nettoyer une poche de slimes avant qu'elle ne bloque un passage.", "Affronter une chose noyée qui attire les voyageurs hors du sentier."}
            };
        }

        if (biomeName == "Route commerciale")
        {
            return {
                biomeName, "Route commerciale / humanoïdes et embuscades",
                {{"battle_torn_badge", "Insigne abîmé d'aventurier", 2, 1}, {"worn_leather_piece", "Morceau de cuir abîmé", 3, 1}, {"goblin_ear", "Oreille de gobelin", 3, 1}, {"shadow_thread", "Fil d'ombre", 1, 5}},
                {"Retrouver une caisse perdue avant que les gobelins ne l'appellent investissement.", "Vérifier une portion de route où les marchands disparaissent toujours au même virage.", "Repérer les faux péages et les traces de pillards."},
                {"Dégager une petite bande d'embuscadeurs de la route.", "Faire comprendre à des voleurs que la taxe gobeline n'est pas reconnue par la guilde."}
            };
        }

        if (biomeName == "Ruines effondrées")
        {
            return {
                biomeName, "Ruines effondrées / morts-vivants et reliques",
                {{"cracked_bone", "Os fissuré", 3, 1}, {"arcane_dust", "Poussière arcanique", 2, 1}, {"battle_torn_badge", "Insigne abîmé d'aventurier", 1, 2}, {"shadow_thread", "Fil d'ombre", 1, 5}},
                {"Copier des symboles de ruines avant qu'ils ne se déplacent encore.", "Explorer une salle fissurée et revenir avec une preuve qu'elle existe vraiment.", "Récupérer une poussière ancienne sans réveiller tout le couloir."},
                {"Briser quelques gardiens fissurés qui bloquent l'accès aux fouilles.", "Nettoyer une alcôve où les os ont recommencé à s'organiser."}
            };
        }

        return {
            biomeName.empty() ? "Plaine sauvage" : biomeName, "Plaine sauvage / créatures locales",
            {{"worn_leather_piece", "Morceau de cuir abîmé", 3, 1}, {"wolf_fang", "Croc de loup", 2, 1}, {"beast_hide", "Peau de bête robuste", 1, 3}, {"bitter_healing_leaf", "Feuille amère de soin", 3, 1}},
            {"Suivre des traces fraîches dans l'herbe sans attirer toute la meute.", "Chercher un petit trésor de plaine avant qu'un animal curieux ne parte avec.", "Cartographier une zone calme qui ne le restera probablement pas."},
            {"Repousser une bête territoriale qui approche trop près des chemins.", "Faire fuir une meute locale sans transformer la plaine en cimetière."}
        };
    }
}

std::vector<Quest> QuestCatalog::createGuildBoard(int playerLevel)
{
    std::vector<Quest> board;

    const std::vector<GuildTemplate> fTemplates = {
        {"F", "Livrer des provisions à un camp proche", "Terminer une mission courte pour sécuriser une livraison locale.", "exploration", "Route / livraison", 1, 1},
        {"F", "Repérer une trace facile", "Sortir brièvement et confirmer une trace signalée par un éclaireur débutant.", "exploration", "Plaine sauvage / traces", 1, 1},
        {"F", "Aider un garde qui débute", "Écarter une petite menace sans transformer la mission en duel héroïque inutile.", "combat", "Créatures faibles", 1, 1}
    };

    const std::vector<GuildTemplate> eTemplates = {
        {"E", "Nettoyer une petite route infestée", "Écarter quelques monstres faibles d'une route locale.", "combat", "Créatures faibles", 2, 1},
        {"E", "Cartographier un détour douteux", "Explorer une zone simple et revenir avec assez de détails pour corriger la carte de la guilde.", "exploration", "Route / exploration", 2, 1},
        {"E", "Surveiller une caisse suspecte", "Vérifier une livraison abandonnée et survivre à ce qui pourrait se cacher dedans.", "exploration", "Route commerciale", 2, 2}
    };

    const std::vector<GuildTemplate> dTemplates = {
        {"D", "Récupérer des matériaux près d'une zone instable", "Revenir avec des matériaux exploitables après plusieurs affrontements ou fouilles.", "exploration", "Matériaux / fouille", 3, 1},
        {"D", "Traquer une meute locale", "Réduire la pression d'un groupe de créatures qui rôde trop près des voyageurs.", "combat", "Créatures locales", 3, 3},
        {"D", "Observer un monstre évolué", "Confirmer les signes d'évolution d'une créature sans mourir pour la science.", "bestiaire", "Créature évoluée", 3, 4}
    };

    const std::vector<GuildTemplate> cTemplates = {
        {"C", "Escorter un apprenti marchand nerveux", "Protéger un civil assez longtemps pour qu'il arrête de trembler.", "combat", "Humanoïdes / embuscades", 3, 1},
        {"C", "Fouille de ruines encadrée", "Explorer une ruine instable et revenir avec des notes utilisables par la guilde.", "exploration", "Ruines effondrées", 3, 5},
        {"C", "Contrat anti-embuscade", "Affronter plusieurs menaces intelligentes qui testent les routes commerciales.", "combat", "Humanoïdes / embuscades", 4, 6}
    };

    const std::vector<GuildTemplate> bTemplates = {
        {"B", "Traquer une menace signalée par la guilde", "Identifier puis vaincre une menace plus sérieuse.", "combat", "Élite / menace", 4, 1},
        {"B", "Mini-boss régional", "Forcer une menace locale à se montrer et survivre au rapport de mission.", "combat", "Mini-boss / menace évoluée", 4, 7},
        {"B", "Zone dangereuse sous surveillance", "Revenir d'un lieu dangereux avec assez d'informations pour éviter un massacre de novices.", "exploration", "Menace avancée", 4, 8}
    };

    board.push_back(buildGuildQuest("guild_f_dynamic", playerLevel, chooseGuildTemplate(fTemplates, playerLevel)));
    board.push_back(buildGuildQuest("guild_e_dynamic", playerLevel, chooseGuildTemplate(eTemplates, playerLevel)));
    board.push_back(buildGuildQuest("guild_d_dynamic", playerLevel, chooseGuildTemplate(dTemplates, playerLevel)));
    board.push_back(buildGuildQuest("guild_c_dynamic", playerLevel, chooseGuildTemplate(cTemplates, playerLevel)));
    board.push_back(buildGuildQuest("guild_b_dynamic", playerLevel, chooseGuildTemplate(bTemplates, playerLevel)));

    if (playerLevel >= 8)
    {
        const std::vector<GuildTemplate> aTemplates = {
            {"A", "Contrat dangereux de la guilde", "Enchaîner plusieurs sorties contre des adversaires solides sans abandonner le contrat.", "combat", "Menace avancée", 5, 8},
            {"A", "Anomalie régionale mineure", "Explorer et stabiliser une variation anormale avant qu'elle n'attire un vrai boss.", "exploration", "Variation d'énergie", 5, 8},
            {"A", "Chasse d'élite", "Affronter une menace évoluée qui a déjà survécu à plusieurs groupes.", "combat", "Mini-boss / menace évoluée", 5, 10}
        };

        board.push_back(buildGuildQuest("guild_a_dynamic", playerLevel, chooseGuildTemplate(aTemplates, playerLevel)));
    }

    if (playerLevel >= 14)
    {
        const std::vector<GuildTemplate> sTemplates = {
            {"S", "Mission classée S", "Un contrat que la guilde ne donne pas aux aventuriers qui tiennent à leurs dents.", "combat", "Menace majeure", 6, 14},
            {"S", "Registre interdit", "Vérifier une entrée qui ne devrait pas apparaître dans un registre public.", "bestiaire", "Boss potentiel / variation majeure", 6, 14},
            {"S", "Route condamnée", "Nettoyer une zone où la guilde a déjà perdu trop de noms.", "combat", "Menace majeure", 7, 16}
        };

        board.push_back(buildGuildQuest("guild_s_dynamic", playerLevel, chooseGuildTemplate(sTemplates, playerLevel)));
    }

    return board;
}

// EN: createForgemasterMaterialRequest declares or implements a focused behavior used by this module.
// FR: createForgemasterMaterialRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createForgemasterMaterialRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"rusted_metal_fragment", "Fragment de métal rouillé", 3, 1},
        {"worn_leather_piece", "Morceau de cuir abîmé", 3, 1},
        {"battle_torn_badge", "Insigne abîmé d'aventurier", 2, 2},
        {"beast_hide", "Peau de bête robuste", 2, 3},
        {"draconic_scale_fragment", "Fragment d'écaille draconique", 1, 6}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 4);

    return buildQuest(
        questIdWithMaterial("npc_forge_materials", material, playerLevel), rank, "Commande du forgeron", "PNJ client", "Forgeron", "Forge",
        "Récupérer des matériaux utiles à la forge, puis revenir les vendre au forgeron.",
        "livraison", "Métal / cuir / forge", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createAlchemistIngredientRequest declares or implements a focused behavior used by this module.
// FR: createAlchemistIngredientRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createAlchemistIngredientRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"bitter_healing_leaf", "Feuille amère de soin", 3, 1},
        {"mountain_blue_flower", "Fleur bleue de montagne", 2, 1},
        {"slime_residue", "Résidu de slime", 3, 1},
        {"arcane_dust", "Poussière arcanique", 2, 2},
        {"kitsune_ember", "Braise kitsune", 1, 5},
        {"unstable_core", "Noyau instable", 1, 7}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 3);

    return buildQuest(
        questIdWithMaterial("npc_alchemist_ingredients", material, playerLevel), rank, "Ingrédients instables", "PNJ client", "Alchimiste", "Herboristerie",
        "Trouver des plantes, résidus ou catalyseurs utiles pour une préparation dangereusement intéressante.",
        "livraison", "Plantes / alchimie", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createVillagerMonsterFearRequest declares or implements a focused behavior used by this module.
// FR: createVillagerMonsterFearRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createVillagerMonsterFearRequest(int playerLevel)
{
    std::string rank = playerLevel >= 6 ? "D" : "F";
    int target = playerLevel >= 6 ? 3 : 2;

    return buildQuest(
        "npc_villager_fear_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(100, 999)),
        rank, "Peur de villageois", "Événement personnel", "Villageois nerveux", "Place du village",
        "Éliminer ou éloigner des monstres qui terrorisent un habitant. Ses raisons peuvent être valables... ou pas très glorieuses.",
        "combat", "Créatures locales", questExperience(rank, playerLevel, target), questGold(rank, playerLevel, target), target, false
    );
}

// EN: createMerchantDeliveryRequest declares or implements a focused behavior used by this module.
// FR: createMerchantDeliveryRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createMerchantDeliveryRequest(int playerLevel)
{
    std::string rank = playerLevel >= 5 ? "D" : "E";
    int target = playerLevel >= 7 ? 2 : 1;

    return buildQuest(
        "npc_merchant_delivery_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(100, 999)),
        rank, "Marchandise en retard", "PNJ client", "Marchand inquiet", "Route commerciale",
        "Récupérer une caisse perdue avant que quelqu'un de moins honnête ne la trouve.",
        "exploration", "Route commerciale", questExperience(rank, playerLevel, target), questGold(rank, playerLevel, target), target, false
    );
}

// EN: createMonsterMaterialVendorRequest declares or implements a focused behavior used by this module.
// FR: createMonsterMaterialVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createMonsterMaterialVendorRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"goblin_ear", "Oreille de gobelin", 3, 1},
        {"wolf_fang", "Croc de loup", 2, 1},
        {"cracked_bone", "Os fissuré", 2, 2},
        {"slime_residue", "Résidu de slime", 3, 2},
        {"shadow_thread", "Fil d'ombre", 1, 5}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 3);

    return buildQuest(
        questIdWithMaterial("npc_monster_vendor", material, playerLevel), rank, "Commande de composants de monstres", "PNJ client", "Vendeur de composants", "Boutique de monstres",
        "Rapporter des composants récupérés sur des créatures pour alimenter les contrats et les recettes étranges.",
        "livraison", "Composants de monstres", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createMaterialVendorRequest declares or implements a focused behavior used by this module.
// FR: createMaterialVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createMaterialVendorRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"rusted_metal_fragment", "Fragment de métal rouillé", 4, 1},
        {"worn_leather_piece", "Morceau de cuir abîmé", 3, 1},
        {"beast_hide", "Peau de bête robuste", 2, 3},
        {"arcane_dust", "Poussière arcanique", 2, 4}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 3);

    return buildQuest(
        questIdWithMaterial("npc_material_vendor", material, playerLevel), rank, "Réassort de matériaux", "PNJ client", "Vendeur de matériaux", "Boutique de matériaux",
        "Aider le vendeur à refaire son stock avec des matériaux exploitables.",
        "livraison", "Matériaux généraux", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createHerbalistRequest declares or implements a focused behavior used by this module.
// FR: createHerbalistRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createHerbalistRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"bitter_healing_leaf", "Feuille amère de soin", 4, 1},
        {"mountain_blue_flower", "Fleur bleue de montagne", 2, 1},
        {"kitsune_ember", "Braise kitsune", 1, 6}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 3);

    return buildQuest(
        questIdWithMaterial("npc_herbalist", material, playerLevel), rank, "Commande de l'herboriste", "PNJ client", "Herboriste", "Herboristerie",
        "Trouver des plantes ou composants naturels pour préparer des remèdes.",
        "livraison", "Plantes", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createArmorerRequest declares or implements a focused behavior used by this module.
// FR: createArmorerRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createArmorerRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"worn_leather_piece", "Morceau de cuir abîmé", 4, 1},
        {"beast_hide", "Peau de bête robuste", 2, 3},
        {"draconic_scale_fragment", "Fragment d'écaille draconique", 1, 7},
        {"atlas_broken_plate", "Plaque brisée d'Atlas", 1, 10}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 4);

    return buildQuest(
        questIdWithMaterial("npc_armorer", material, playerLevel), rank, "Pièces pour armures", "PNJ client", "Armurier", "Armurerie défensive",
        "Ramener des matériaux défensifs pour réparer ou préparer de futures armures.",
        "livraison", "Armures / défense", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createWeaponVendorRequest declares or implements a focused behavior used by this module.
// FR: createWeaponVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createWeaponVendorRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"rusted_metal_fragment", "Fragment de métal rouillé", 4, 1},
        {"wolf_fang", "Croc de loup", 2, 2},
        {"arcane_dust", "Poussière arcanique", 2, 4},
        {"zelef_demon_blood", "Sang démoniaque de Zelef", 1, 10}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 4);

    return buildQuest(
        questIdWithMaterial("npc_weapon_vendor", material, playerLevel), rank, "Approvisionnement de forge d'armes", "PNJ client", "Vendeur d'armes", "Forge d'armes",
        "Fournir de quoi entretenir ou améliorer les armes vendues aux aventuriers.",
        "livraison", "Armes", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createConsumableVendorRequest declares or implements a focused behavior used by this module.
// FR: createConsumableVendorRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createConsumableVendorRequest(int playerLevel)
{
    std::vector<RequestedMaterial> materials = {
        {"bitter_healing_leaf", "Feuille amère de soin", 4, 1},
        {"slime_residue", "Résidu de slime", 3, 1},
        {"arcane_dust", "Poussière arcanique", 2, 3},
        {"unstable_core", "Noyau instable", 1, 6}
    };

    RequestedMaterial material = chooseMaterial(materials, playerLevel);
    int quantity = materialQuantity(material, playerLevel);
    std::string rank = materialRank(playerLevel, 3);

    return buildQuest(
        questIdWithMaterial("npc_consumable_vendor", material, playerLevel), rank, "Stocks de consommables", "PNJ client", "Vendeur de consommables", "Boutique de consommables",
        "Rapporter des ingrédients pour que les consommables ne coûtent pas un rein à tout le monde.",
        "livraison", "Consommables", questExperience(rank, playerLevel, quantity), questGold(rank, playerLevel, quantity), quantity, false,
        material.id, material.name, quantity
    );
}

// EN: createLibrarianRequest declares or implements a focused behavior used by this module.
// FR: createLibrarianRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createLibrarianRequest(int playerLevel)
{
    std::string rank = playerLevel >= 8 ? "C" : (playerLevel >= 4 ? "D" : "E");
    int target = 2 + (playerLevel >= 5 ? 1 : 0) + (playerLevel >= 10 ? 1 : 0);

    return buildQuest(
        "npc_librarian_notes_" + std::to_string(playerLevel) + "_" + std::to_string(randomBetween(100, 999)),
        rank, "Notes à vérifier", "PNJ client", "Bibliothécaire", "Bibliothèque",
        "Sortir sur le terrain pour confirmer des informations de bestiaire que personne n'a pris le temps de vérifier proprement.",
        "bestiaire", "Bestiaire / observation", questExperience(rank, playerLevel, target), questGold(rank, playerLevel, target), target, false
    );
}


// EN: createBiomeRequest declares or implements a focused behavior used by this module.
// FR: createBiomeRequest déclare ou implémente un comportement précis utilisé par ce module.
Quest QuestCatalog::createBiomeRequest(int playerLevel, const std::string& biomeName, const std::string& preferredClient)
{
    BiomeQuestProfile profile = profileForBiome(biomeName);
    bool materialQuest = randomBetween(1, 100) <= 45;
    bool combatQuest = randomBetween(1, 100) <= 50;

    std::string client = preferredClient.empty() ? chooseText({
        "Éclaireur de guilde", "Villageois nerveux", "Marchand inquiet", "Herboriste", "Bibliothécaire", "Vendeur de composants"
    }) : preferredClient;

    if (client == "Forgeron" || client == "Armurier" || client == "Vendeur d'armes")
    {
        materialQuest = true;
    }

    if (client == "Bibliothécaire")
    {
        materialQuest = false;
        combatQuest = false;
    }

    std::string rank = playerLevel >= 12 ? "B" : (playerLevel >= 8 ? "C" : (playerLevel >= 5 ? "D" : "E"));
    int target = 2 + (playerLevel >= 6 ? 1 : 0) + (playerLevel >= 12 ? 1 : 0);

    if (materialQuest)
    {
        RequestedMaterial material = chooseMaterial(profile.materials, playerLevel);
        int quantity = materialQuantity(material, playerLevel);
        std::string title = chooseText({
            "Demande locale : " + profile.location,
            "Ressource recherchée — " + profile.location,
            "Commande ciblée du biome",
            "Prélèvement utile en zone connue"
        });

        return buildQuest(
            questIdWithMaterial("npc_biome_material", material, playerLevel), rank, title,
            "Demande de biome", client, profile.location,
            "Rapporter " + material.name + " depuis " + profile.location + ". La demande est liée au terrain, pas à une liste générique.",
            "livraison", profile.family,
            questExperience(rank, playerLevel, quantity + 1), questGold(rank, playerLevel, quantity + 1), quantity, false,
            material.id, material.name, quantity
        );
    }

    if (combatQuest)
    {
        std::string title = chooseText({
            "Menace locale — " + profile.location,
            "Contrat de terrain ciblé",
            "Présence hostile confirmée",
            "Nettoyage prudent du biome"
        });

        return buildQuest(
            questId("npc_biome_combat", playerLevel), rank, title,
            "Demande de biome", client, profile.location,
            chooseText(profile.combatObjectives),
            "combat", profile.family,
            questExperience(rank, playerLevel, target), questGold(rank, playerLevel, target), target, false
        );
    }

    std::string title = chooseText({
        "Notes de terrain — " + profile.location,
        "Observation ciblée du biome",
        "Carte incomplète",
        "Hypothèse à vérifier"
    });

    return buildQuest(
        questId("npc_biome_exploration", playerLevel), rank, title,
        "Demande de biome", client, profile.location,
        chooseText(profile.explorationObjectives),
        client == "Bibliothécaire" ? "bestiaire" : "exploration", profile.family,
        questExperience(rank, playerLevel, target), questGold(rank, playerLevel, target), target, false
    );
}
