// EN: MenuScreen.cpp implements a display-neutral screen model used during GUI preparation.
// FR: MenuScreen.cpp implémente un modèle d'écran neutre utilisé pour préparer l'IG.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/model/MenuScreen.hpp"

#include <algorithm>
#include <cctype>
#include <string>

namespace
{
    std::string toLowerAscii(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool containsText(const std::string& text, const std::string& needle)
    {
        return toLowerAscii(text).find(toLowerAscii(needle)) != std::string::npos;
    }

    bool containsAny(const std::string& text, const std::vector<std::string>& needles)
    {
        for (const std::string& needle : needles)
        {
            if (containsText(text, needle))
            {
                return true;
            }
        }

        return false;
    }

    void addUniqueTag(std::vector<std::string>& tags, const std::string& tag)
    {
        if (tag.empty())
        {
            return;
        }

        if (std::find(tags.begin(), tags.end(), tag) == tags.end())
        {
            tags.push_back(tag);
        }
    }



    std::string trimCopy(const std::string& value)
    {
        std::size_t first = 0;
        while (first < value.size() && std::isspace(static_cast<unsigned char>(value[first])))
        {
            ++first;
        }

        std::size_t last = value.size();
        while (last > first && std::isspace(static_cast<unsigned char>(value[last - 1])))
        {
            --last;
        }

        return value.substr(first, last - first);
    }

    bool startsWithNumberChoice(const std::string& value)
    {
        std::size_t index = 0;
        while (index < value.size() && std::isspace(static_cast<unsigned char>(value[index])))
        {
            ++index;
        }

        bool hasDigit = false;
        while (index < value.size() && std::isdigit(static_cast<unsigned char>(value[index])))
        {
            hasDigit = true;
            ++index;
        }

        while (index < value.size() && std::isspace(static_cast<unsigned char>(value[index])))
        {
            ++index;
        }

        return hasDigit && index < value.size() && value[index] == ':';
    }

    std::string classifyInfoKind(const std::string& label, const std::string& value)
    {
        const std::string text = label + " " + value;

        if (containsAny(text, {"pv", "hp", "vie", "sante", "santé", "soin", "heal"})) return "health";
        if (containsAny(text, {"mort", "morts", "lethal", "létal", "perdu", "perdue", "perdus", "brisée", "brise", "volée", "volee", "survie", "corrompue", "effondrement"})) return "death";
        if (containsAny(text, {"or", "gold", "prix", "cout", "coût", "argent", "monnaie", "vente", "achat", "valeur", "pièce", "pièces", "pieces"})) return "economy";
        if (containsAny(text, {"niveau", "level", "xp", "experience", "expérience", "rang", "classe", "compétence", "competence", "skill", "progression"})) return "progression";
        if (containsAny(text, {"force", "dextérité", "dexterite", "constitution", "intelligence", "sagesse", "charisme", "attribut"})) return "attribute";
        if (containsAny(text, {"dégâts", "degats", "damage", "critique", "attaque", "puissance"})) return "damage";
        if (containsAny(text, {"durabilite", "durabilité", "arme", "armure", "equipement", "équipement", "réduction", "reduction", "bonus"})) return "equipment";
        if (containsAny(text, {"race", "type", "nom", "catégorie", "categorie", "qualité", "qualite"})) return "identity";
        if (containsAny(text, {"stock", "quantite", "quantité", "nombre", "exemplaire"})) return "count";
        if (containsAny(text, {"danger", "boss", "risque", "alerte"})) return "danger";
        if (containsAny(text, {"quete", "quête", "mission", "objectif", "guilde"})) return "quest";
        if (containsAny(text, {"description", "lore", "utilité", "utilite", "texte"})) return "lore";

        return "info";
    }

    bool splitInfoLine(const std::string& rawLine, std::string& label, std::string& value)
    {
        const std::string line = trimCopy(rawLine);
        if (line.empty() || startsWithNumberChoice(line))
        {
            return false;
        }

        const std::size_t separator = line.find(':');
        if (separator == std::string::npos || separator == 0 || separator + 1 >= line.size())
        {
            return false;
        }

        label = trimCopy(line.substr(0, separator));
        value = trimCopy(line.substr(separator + 1));

        if (label.size() < 2 || label.size() > 42 || value.empty() || value.size() > 120)
        {
            return false;
        }

        return true;
    }

    void addInfoCardFromLine(
        GuiMenuSnapshot& snapshot,
        const std::string& rawLine,
        const std::string& source,
        std::size_t maxCards
    )
    {
        if (snapshot.infoCards.size() >= maxCards)
        {
            return;
        }

        std::string label;
        std::string value;
        if (!splitInfoLine(rawLine, label, value))
        {
            return;
        }

        for (const GuiMenuInfoSnapshot& existing : snapshot.infoCards)
        {
            if (toLowerAscii(existing.label) == toLowerAscii(label) && toLowerAscii(existing.value) == toLowerAscii(value))
            {
                return;
            }
        }

        GuiMenuInfoSnapshot card;
        card.id = "info." + std::to_string(snapshot.infoCards.size() + 1);
        card.label = label;
        card.value = value;
        card.source = source;
        card.kind = classifyInfoKind(label, value);
        card.important = card.kind == "health" || card.kind == "economy" || card.kind == "danger" || card.kind == "death";
        snapshot.infoCards.push_back(card);
    }

    void populateInfoCards(GuiMenuSnapshot& snapshot)
    {
        if (snapshot.specializedView == "generic")
        {
            return;
        }

        const std::size_t maxCards =
            snapshot.specializedView == "equipment" ||
            snapshot.specializedView == "inventory" ||
            snapshot.specializedView == "progression" ||
            snapshot.specializedView == "combat"
                ? 14
                : 8;

        for (const std::string& subtitle : snapshot.subtitles)
        {
            addInfoCardFromLine(snapshot, subtitle, "subtitle", maxCards);
        }

        for (const std::string& line : snapshot.lines)
        {
            addInfoCardFromLine(snapshot, line, "line", maxCards);
        }

        for (const std::string& footerLine : snapshot.footerLines)
        {
            addInfoCardFromLine(snapshot, footerLine, "footer", maxCards);
        }
    }


    std::string classifyNoticeKind(const std::string& rawText)
    {
        const std::string text = trimCopy(rawText);

        if (containsAny(text, {"mort", "morts", "lethal", "létal", "survie", "corrompue", "effondrement"})) return "death";
        if (containsAny(text, {"danger", "boss", "risque", "alerte"})) return "danger";
        if (containsAny(text, {"attention", "impossible", "indisponible", "verrouille", "verrouillé", "bloque", "bloqué", "pas assez"})) return "warning";
        if (containsAny(text, {"victoire", "reussite", "réussite", "terminee", "terminée", "obtenu", "gagne", "gagné", "recompense", "récompense"})) return "success";
        if (containsAny(text, {"quete", "quête", "mission", "objectif", "client", "guilde"})) return "quest";
        if (containsAny(text, {"aucun", "aucune", "vide", "rien"})) return "empty";
        if (containsAny(text, {"choisis", "choisir", "selectionne", "sélectionne", "appuie", "utilise", "entre un", "saisis"})) return "help";
        if (text.size() >= 72) return "lore";

        return "info";
    }

    bool isSeparatorLine(const std::string& rawLine)
    {
        const std::string line = trimCopy(rawLine);
        if (line.size() < 3)
        {
            return false;
        }

        for (char character : line)
        {
            if (character != '-' && character != '=' && character != '_' && character != '*')
            {
                return false;
            }
        }

        return true;
    }

    void addNoticeCardFromLine(
        GuiMenuSnapshot& snapshot,
        const std::string& rawLine,
        const std::string& source,
        std::size_t maxCards
    )
    {
        if (snapshot.noticeCards.size() >= maxCards)
        {
            return;
        }

        const std::string text = trimCopy(rawLine);
        if (text.empty() || text.size() < 4 || text.size() > 220 || startsWithNumberChoice(text) || isSeparatorLine(text))
        {
            return;
        }

        std::string ignoredLabel;
        std::string ignoredValue;
        if (splitInfoLine(text, ignoredLabel, ignoredValue))
        {
            return;
        }

        for (const GuiMenuNoticeSnapshot& existing : snapshot.noticeCards)
        {
            if (toLowerAscii(existing.text) == toLowerAscii(text))
            {
                return;
            }
        }

        GuiMenuNoticeSnapshot card;
        card.id = "notice." + std::to_string(snapshot.noticeCards.size() + 1);
        card.text = text;
        card.source = source;
        card.kind = classifyNoticeKind(text);
        card.important = card.kind == "danger" || card.kind == "death" || card.kind == "warning" || card.kind == "success";
        snapshot.noticeCards.push_back(card);
    }

    void populateNoticeCards(GuiMenuSnapshot& snapshot)
    {
        if (snapshot.specializedView == "generic")
        {
            return;
        }

        constexpr std::size_t maxCards = 6;

        for (const std::string& subtitle : snapshot.subtitles)
        {
            addNoticeCardFromLine(snapshot, subtitle, "subtitle", maxCards);
        }

        for (const std::string& line : snapshot.lines)
        {
            addNoticeCardFromLine(snapshot, line, "line", maxCards);
        }

        for (const std::string& footerLine : snapshot.footerLines)
        {
            addNoticeCardFromLine(snapshot, footerLine, "footer", maxCards);
        }
    }

    std::string inferScreenCategory(const std::string& screenId, const std::string& title)
    {
        const std::string text = screenId + " " + title;

        // FR: les catégories les plus spécifiques passent avant "combat" pour éviter
        // qu'un écran post_combat, catalogue de boss ou détail d'objet soit rangé trop tôt.
        if (containsAny(text, {"item.weapon.display", "item.armor.display", "weapon.display", "armor.display"})) return "equipement";
        if (containsAny(text, {"item.material.display", "item.consumable.display", "item.display", "material.display", "consumable.display"})) return "inventaire";
        if (containsAny(text, {"monster.stats", "boss.stats", "entity.stats", "player.stats", "player.attributes", "player.skills", "player.career", "career.statistics"})) return "progression";
        if (containsAny(text, {"activity.", "activite", "activité", "activités", "activity", "combats", "boutiques / lieux", "pnj notables", "échange / don", "echange / don", "menu après-combat", "menu apres-combat"})) return "activite";
        if (containsAny(text, {"session.party", "session coop", "session solo", "multi local", "groupe actif"})) return "session";
        if (containsAny(text, {"character.creation", "difficulté", "difficulte", "race validée", "race validee", "personnage gravé", "personnage grave", "nom gravé", "nom grave"})) return "creation";
        if (containsAny(text, {"catalog", "catalogue", "classes", "races", "race", "personnages spéciaux", "personnages speciaux", "groupes spéciaux", "groupes speciaux", "monstres préparés", "monstres prepares", "boss disponibles"})) return "catalogue";
        if (containsAny(text, {"post_combat", "apres combat", "après combat", "recompense", "récompense"})) return "apres_combat";
        if (containsAny(text, {"quick_heal", "potion", "potions", "curative", "defensive", "défensive", "offensive", "buff", "debuff"})) return "potions";
        if (containsAny(text, {"shop", "boutique", "achat", "vente", "marchand", "stock", "troquer", "troc"})) return "boutique";
        if (containsAny(text, {"exploration", "biome", "biomes", "intensité", "intensite", "sortie prudente", "sortie normale", "sortie audacieuse"})) return "exploration";
        if (containsAny(text, {"quest", "quete", "quête", "guilde", "demande", "mission", "lieux visitables"})) return "quete";
        if (containsAny(text, {"bestiary", "bestiaire", "knowledge", "connaissance", "statistique", "statistics", "stats", "materiau", "matériau", "material", "player.", "joueur", "monstre", "monster", "entité", "entite", "attribut", "attributs", "compétence", "competence", "compétences", "competences", "skill", "career", "parcours"})) return "progression";
        if (containsAny(text, {"inventory", "inventaire", "item", "objet", "consommable"})) return "inventaire";
        if (containsAny(text, {"equipment", "equipement", "équipement", "arme", "armure"})) return "equipement";
        if (containsAny(text, {"death", "mort", "morts", "lethal", "létal", "penalty", "pénalité", "penalite", "anomalie de survie", "survie", "corrompue"})) return "mort";
        if (containsAny(text, {"save", "sauvegarde", "account", "compte", "character", "personnage"})) return "sauvegarde";
        if (containsAny(text, {"cheat", "triche", "alter", "altéré", "altere", "données altérées", "donnees alterees"})) return "donnees_alterees";
        if (containsAny(text, {"combat", "battle", "boss", "target", "cible", "pvp", "duel", "arène", "arene"})) return "combat";
        if (containsAny(text, {"story", "histoire", "archives", "chapitre"})) return "progression";

        return "menu";
    }

    std::string inferSpecializedView(const std::string& screenCategory)
    {
        if (screenCategory == "inventaire") return "inventory";
        if (screenCategory == "equipement") return "equipment";
        if (screenCategory == "potions") return "potions";
        if (screenCategory == "boutique") return "shop";
        if (screenCategory == "exploration") return "exploration";
        if (screenCategory == "quete") return "quest";
        if (screenCategory == "progression") return "progression";
        if (screenCategory == "catalogue") return "catalog";
        if (screenCategory == "activite") return "activity";
        if (screenCategory == "session") return "session";
        if (screenCategory == "creation") return "creation";
        if (screenCategory == "apres_combat") return "post_combat";
        if (screenCategory == "sauvegarde") return "save";
        if (screenCategory == "mort") return "death";
        if (screenCategory == "donnees_alterees") return "altered_data";
        if (screenCategory == "combat") return "combat";
        return "generic";
    }

    std::string inferSpecializedTitle(const std::string& specializedView)
    {
        if (specializedView == "inventory") return "Vue inventaire";
        if (specializedView == "equipment") return "Vue équipement";
        if (specializedView == "potions") return "Vue potions";
        if (specializedView == "shop") return "Vue boutique";
        if (specializedView == "exploration") return "Vue exploration";
        if (specializedView == "quest") return "Vue quêtes";
        if (specializedView == "progression") return "Vue progression";
        if (specializedView == "catalog") return "Vue catalogue";
        if (specializedView == "activity") return "Vue activités";
        if (specializedView == "session") return "Vue session";
        if (specializedView == "creation") return "Vue création";
        if (specializedView == "post_combat") return "Vue après-combat";
        if (specializedView == "save") return "Vue sauvegarde";
        if (specializedView == "death") return "Vue mort";
        if (specializedView == "altered_data") return "Vue données altérées";
        if (specializedView == "combat") return "Vue combat";
        return "Vue menu";
    }

    std::string inferSpecializedHint(const std::string& specializedView)
    {
        if (specializedView == "inventory") return "Affiche les objets, détails inspectés et chemins utiles sans masquer le choix terminal.";
        if (specializedView == "equipment") return "Sépare les décisions d'équipement, d'inspection, de durabilité et de retour.";
        if (specializedView == "potions") return "Met en valeur les familles de potions et les soins sans forcer l'action.";
        if (specializedView == "shop") return "Prépare les blocs achat, vente, stock, inspection et troc.";
        if (specializedView == "exploration") return "Sépare biomes, risques, intensités, ressources et objectifs de terrain.";
        if (specializedView == "quest") return "Prépare les blocs journal, guilde, clients et lieux liés aux quêtes.";
        if (specializedView == "progression") return "Prépare les blocs bestiaire, matériaux, statistiques et connaissances.";
        if (specializedView == "catalog") return "Transforme les listes de races, classes, monstres, boss et personnages spéciaux en cartes consultables.";
        if (specializedView == "activity") return "Regroupe les grandes routes jouables : histoire, combats, exploration, quêtes, lieux et gestion.";
        if (specializedView == "session") return "Structure le choix solo/coop local et les personnages secondaires.";
        if (specializedView == "creation") return "Structure la création du personnage : difficulté, race, classe, identité et validation.";
        if (specializedView == "post_combat") return "Regroupe les actions logiques disponibles entre deux combats.";
        if (specializedView == "save") return "Structure les comptes, personnages, versions et actions de sauvegarde.";
        if (specializedView == "death") return "Structure les pertes, protections et anomalies liées à la mort.";
        if (specializedView == "altered_data") return "Isole les informations d'altération sans les mélanger aux menus normaux.";
        if (specializedView == "combat") return "Sépare les décisions de combat tout en évitant les recommandations offensives forcées.";
        return "Affichage générique en attendant un rendu dédié.";
    }

    std::string inferInputIntent(const std::string& inputMode)
    {
        if (inputMode == "text") return "texte libre";
        if (inputMode == "quantity") return "quantite";
        if (inputMode == "confirmation") return "confirmation exacte";
        if (inputMode == "continue") return "continuer";
        if (inputMode == "display") return "lecture";
        return "choix";
    }

    std::string inferExpectedInputPattern(const std::string& inputMode)
    {
        if (inputMode == "choice") return "integer_or_hidden_code";
        if (inputMode == "quantity") return "integer_in_range";
        if (inputMode == "confirmation") return "exact_keyword";
        if (inputMode == "continue") return "empty_enter";
        if (inputMode == "text") return "free_text";
        if (inputMode == "display") return "none";
        return "unknown";
    }

    std::string inferPrimarySubmitLabel(const std::string& inputMode)
    {
        if (inputMode == "choice") return "Envoyer le choix";
        if (inputMode == "quantity") return "Valider la quantite";
        if (inputMode == "confirmation") return "Confirmer";
        if (inputMode == "continue") return "Continuer";
        if (inputMode == "text") return "Envoyer le texte";
        if (inputMode == "display") return "Lecture seule";
        return "Envoyer";
    }

    bool inferInputRequired(const std::string& inputMode)
    {
        return inputMode != "display";
    }

    bool isDangerousAction(const MenuOption& option)
    {
        const std::string text = option.getActionId() + " " + option.getLabel() + " " + option.getHint();
        return containsAny(text, {
            "delete", "remove", "suppr", "effacer", "vider", "abandon",
            "ecraser", "écraser", "detruire", "détruire", "irreversible",
            "irréversible", "lethal", "létal", "mort", "definitif", "définitif"
        });
    }

    bool isBackAction(const MenuOption& option)
    {
        const std::string text = option.getActionId() + " " + option.getLabel();

        if (containsAny(text, {"back", "return", "retour", "annuler", "cancel", "quitter"}))
        {
            return true;
        }

        return option.getNumber() == 0 &&
            !containsAny(text, {"create", "creer", "créer", "nouveau", "new", "incarner", "continuer"});
    }

    bool isPaginationAction(const MenuOption& option)
    {
        const std::string text = option.getActionId() + " " + option.getLabel();
        return option.getNumber() == 98 || option.getNumber() == 99 || containsAny(text, {"page", "suivante", "precedente", "précédente"});
    }

    bool isKnowledgeSensitiveAction(const MenuOption& option)
    {
        const std::string text = option.getActionId() + " " + option.getLabel() + " " + option.getHint();

        return containsAny(text, {
            "faiblesse", "faible", "point faible", "resistance", "résistance",
            "vulnerabilite", "vulnérabilité", "vulnerable", "vulnérable",
            "element oppose", "élément opposé", "element opposé", "élément oppose",
            "weakness", "resistance", "vulnerability", "counter element",
            "secret tactique", "information cachee", "information cachée"
        });
    }


    bool isCombatAttackAction(const MenuOption& option)
    {
        const std::string text = option.getActionId() + " " + option.getLabel() + " " + option.getHint();

        return containsAny(text, {
            "combat.attack", "attack", "attaque", "attaquer", "frapper",
            "technique d'arme", "attaque simple", "attaque lourde", "attaque rapide",
            "sort offensif", "offensive", "tirer", "lancer un sort"
        });
    }

    bool isCombatHealRecommendation(const MenuOption& option)
    {
        const std::string text = option.getActionId() + " " + option.getLabel() + " " + option.getHint();

        return containsAny(text, {
            "recommend_heal", "combat.quick_heal.low_hp", "heal_recommended",
            "soin recommande", "soin recommandé"
        });
    }

    std::string buildRecommendationReason(const GuiMenuActionSnapshot& action)
    {
        if (containsText(action.actionId, "recommend_heal") || containsText(action.actionId, "low_hp"))
        {
            return "PV bas : le soin peut etre signale sans imposer l'action au joueur.";
        }

        if (action.role == "primary")
        {
            return "Unique action principale sure du menu, sans lecture tactique cachee.";
        }

        return "Unique action sure du menu, sans lecture tactique cachee.";
    }

    std::string inferActionRole(const MenuOption& option)
    {
        if (!option.isEnabled()) return "disabled";
        if (isDangerousAction(option)) return "danger";
        if (isBackAction(option)) return "back";
        if (isPaginationAction(option)) return "pagination";
        if (option.getNumber() == 1) return "primary";
        return "normal";
    }

    std::string inferActionStyle(const std::string& role)
    {
        if (role == "danger") return "danger";
        if (role == "primary") return "primary";
        if (role == "back") return "muted";
        if (role == "pagination") return "utility";
        if (role == "disabled") return "disabled";
        return "neutral";
    }

    std::string inferActionGroup(const std::string& role)
    {
        if (role == "danger") return "danger";
        if (role == "disabled") return "locked";
        if (role == "back" || role == "pagination") return "navigation";
        return "main";
    }

    bool actionSnapshotContainsAny(const GuiMenuActionSnapshot& action, const std::vector<std::string>& needles)
    {
        const std::string text = action.actionId + " " + action.label + " " + action.hint + " " + action.role + " " + action.group;
        return containsAny(text, needles);
    }

    void addFocusCard(
        GuiMenuSnapshot& snapshot,
        const std::string& id,
        const std::string& label,
        const std::string& detail,
        const std::vector<std::string>& needles
    )
    {
        GuiMenuFocusSnapshot card;
        card.id = id;
        card.label = label;
        card.detail = detail;

        for (const GuiMenuActionSnapshot& action : snapshot.actions)
        {
            if (action.enabled && actionSnapshotContainsAny(action, needles))
            {
                card.actionNumber = action.number;
                card.actionId = action.actionId;
                card.actionLabel = action.label;
                card.actionDisplayLabel = action.actionDisplayLabel.empty()
                    ? std::to_string(action.number) + " - " + action.label
                    : action.actionDisplayLabel;
                card.available = true;
                break;
            }
        }

        if (card.available)
        {
            snapshot.focusCards.push_back(card);
        }
    }


    std::vector<std::string> splitByPipe(const std::string& rawText)
    {
        std::vector<std::string> parts;
        std::size_t start = 0;

        while (start <= rawText.size())
        {
            const std::size_t separator = rawText.find('|', start);
            const std::size_t end = separator == std::string::npos ? rawText.size() : separator;
            const std::string part = trimCopy(rawText.substr(start, end - start));

            if (!part.empty())
            {
                parts.push_back(part);
            }

            if (separator == std::string::npos)
            {
                break;
            }

            start = separator + 1;
        }

        return parts;
    }

    std::string joinTextParts(const std::vector<std::string>& parts, std::size_t firstIndex)
    {
        std::string output;

        for (std::size_t index = firstIndex; index < parts.size(); ++index)
        {
            if (!output.empty())
            {
                output += " · ";
            }

            output += parts[index];
        }

        return output;
    }

    std::string extractQuantityFromName(std::string& name)
    {
        const std::string lowerName = toLowerAscii(name);
        const std::size_t marker = lowerName.rfind(" x");

        if (marker == std::string::npos || marker + 2 >= name.size())
        {
            return "";
        }

        std::size_t digitStart = marker + 2;
        std::size_t digitEnd = digitStart;

        while (digitEnd < name.size() && std::isdigit(static_cast<unsigned char>(name[digitEnd])))
        {
            ++digitEnd;
        }

        if (digitEnd == digitStart)
        {
            return "";
        }

        const std::string quantity = name.substr(digitStart, digitEnd - digitStart);
        name = trimCopy(name.substr(0, marker) + name.substr(digitEnd));
        return quantity;
    }

    std::string inferItemCardKind(const GuiMenuSnapshot& snapshot, const GuiMenuActionSnapshot& action)
    {
        const std::string text = snapshot.screenId + " " + snapshot.specializedView + " " + action.actionId + " " + action.label + " " + action.hint;

        if (snapshot.specializedView == "activity" || containsAny(action.actionId, {"activity."})) return "activity";
        if (snapshot.specializedView == "session" || containsAny(snapshot.screenId, {"session.party"})) return "session";
        if (snapshot.specializedView == "creation" || containsAny(snapshot.screenId, {"character.creation", "save.characters.create"})) return "creation";
        if (containsAny(text, {"special_characters", "personnage spécial", "personnages spéciaux", "personnage special", "personnages speciaux"})) return "special_character";
        if (containsAny(text, {"special_groups", "groupe spécial", "groupes spéciaux", "groupe special", "groupes speciaux"})) return "special_group";
        if (containsAny(text, {"classes", "classe", "class"})) return "class";
        if (containsAny(text, {"races", "race"})) return "race";
        if (containsAny(text, {"monsters", "monstres", "monstre"})) return "monster";
        if (containsAny(text, {"boss"})) return "boss";
        if (containsAny(text, {"catalog", "catalogue"})) return "catalog";
        if (containsAny(text, {"weapon", "arme"})) return "weapon";
        if (containsAny(text, {"armor", "armure", "tenue"})) return "armor";
        if (containsAny(text, {"consumable", "potion", "consommable", "soin", "curative"})) return "consumable";
        if (containsAny(text, {"material", "materiau", "matériau", "plante", "ressource"})) return "material";
        if (containsAny(text, {"shop", "boutique", "achat", "vente", "troc", "stock"})) return "shop";
        if (containsAny(text, {"exploration", "biome", "intensité", "intensite", "sortie prudente", "sortie normale", "sortie audacieuse"})) return "exploration";
        if (containsAny(text, {"quest", "quete", "quête", "mission", "guilde", "client"})) return "quest";
        if (containsAny(text, {"bestiary", "bestiaire", "statistics", "statistique", "knowledge", "connaissance"})) return "progression";
        if (containsAny(text, {"death", "mort", "morts", "lethal", "létal", "survie", "corrompue"})) return "death";
        if (containsAny(text, {"cheat", "triche", "altéré", "altere", "altération", "alteration", "données altérées", "donnees alterees"})) return "altered_data";
        if (containsAny(text, {"save", "sauvegarde", "compte", "personnage", "creation", "création"})) return "save";

        return snapshot.specializedView == "generic" ? "entry" : snapshot.specializedView;
    }

    std::string inferItemCardStatus(const std::string& text)
    {
        if (containsAny(text, {"cassée", "cassee", "brisé", "brise", "broken"})) return "cassé";
        if (containsAny(text, {"verrouillé", "verrouille", "bloqué", "bloque", "indisponible"})) return "verrouillé";
        if (containsAny(text, {"terminée", "terminee", "complétée", "completee"})) return "terminé";
        if (containsAny(text, {"active", "actif", "en cours"})) return "actif";
        if (containsAny(text, {"rare", "épique", "epique", "légendaire", "legendaire", "relique", "héroïque", "heroique"})) return "important";
        return "";
    }



    std::string cleanMetadataValue(const std::string& rawPart)
    {
        std::string part = trimCopy(rawPart);
        const std::size_t separator = part.find(':');
        if (separator != std::string::npos && separator + 1 < part.size())
        {
            return trimCopy(part.substr(separator + 1));
        }
        return part;
    }

    std::string findMetadataPart(const std::vector<std::string>& parts, const std::vector<std::string>& needles)
    {
        for (std::size_t i = 1; i < parts.size(); ++i)
        {
            const std::string part = trimCopy(parts[i]);
            if (containsAny(part, needles))
            {
                return cleanMetadataValue(part);
            }
        }
        return "";
    }

    std::string inferItemCardSection(const GuiMenuSnapshot& snapshot, const GuiMenuActionSnapshot& action)
    {
        const std::string text = snapshot.screenId + " " + action.actionId + " " + action.label;

        if (snapshot.specializedView == "activity" || containsAny(action.actionId, {"activity."})) return "Activités";
        if (snapshot.specializedView == "session" || containsAny(snapshot.screenId, {"session.party"})) return "Session";
        if (snapshot.specializedView == "creation" || containsAny(snapshot.screenId, {"character.creation", "save.characters.create"})) return "Création";
        if (containsAny(text, {"special_characters", "personnage spécial", "personnages spéciaux", "personnage special", "personnages speciaux"})) return "Personnages spéciaux";
        if (containsAny(text, {"special_groups", "groupe spécial", "groupes spéciaux", "groupe special", "groupes speciaux"})) return "Groupes spéciaux";
        if (containsAny(text, {"classes", "classe", "class"})) return "Classes";
        if (containsAny(text, {"races", "race"})) return "Races";
        if (containsAny(text, {"monsters", "monstres", "monstre"})) return "Monstres";
        if (containsAny(text, {"boss"})) return "Boss";
        if (containsAny(text, {"catalog", "catalogue"})) return "Catalogue";
        if (containsAny(text, {"weapon", "armes", "arme"})) return "Armes";
        if (containsAny(text, {"armor", "armure", "armures", "tenue"})) return "Armures";
        if (containsAny(text, {"consumable", "consommable", "potion", "potions"})) return "Consommables";
        if (containsAny(text, {"material", "materiau", "matériau", "plante", "plantes", "ressource"})) return "Matériaux";
        if (containsAny(text, {"craft", "schema", "schéma", "fabrication"})) return "Craft";
        if (containsAny(text, {"shop", "boutique", "acheter", "vendre", "vente", "prix", "stock", "troquer", "troc"})) return "Boutique";
        if (containsAny(text, {"exploration", "biome", "biomes", "intensité", "intensite", "sortie prudente", "sortie normale", "sortie audacieuse"})) return "Exploration";
        if (containsAny(text, {"guild", "guilde", "quest", "quête", "quete", "mission", "client", "journal"})) return "Quêtes";
        if (containsAny(text, {"monster.stats", "monstre", "monster"})) return "Monstres";
        if (containsAny(text, {"boss.stats", "boss"})) return "Boss";
        if (containsAny(text, {"entity.stats", "entité", "entite"})) return "Entités";
        if (containsAny(text, {"statistic", "statistique", "progression", "bestiary", "bestiaire", "knowledge", "connaissance"})) return "Progression";
        if (containsAny(text, {"death", "mort", "morts", "lethal", "létal", "survie", "corrompue"})) return "Mort";
        if (containsAny(text, {"cheat", "triche", "altéré", "altere", "altération", "alteration", "données altérées", "donnees alterees"})) return "Données altérées";
        if (containsAny(text, {"save", "sauvegarde", "compte", "personnage", "creation", "création"})) return "Sauvegarde";
        if (containsAny(text, {"continuer", "post_combat", "après-combat", "apres-combat"})) return "Après-combat";
        if (containsAny(text, {"location", "lieu", "lieux", "exploration", "forge", "bibliothèque", "bibliotheque"})) return "Lieux";
        if (containsAny(text, {"npc", "pnj", "parler", "discuter"})) return "PNJ";

        if (snapshot.specializedView == "inventory") return "Inventaire";
        if (snapshot.specializedView == "equipment") return "Équipement";
        if (snapshot.specializedView == "potions") return "Potions";
        if (snapshot.specializedView == "shop") return "Boutique";
        if (snapshot.specializedView == "exploration") return "Exploration";
        if (snapshot.specializedView == "quest") return "Quêtes";
        if (snapshot.specializedView == "progression") return "Progression";
        if (snapshot.specializedView == "catalog") return "Catalogue";
        if (snapshot.specializedView == "activity") return "Activités";
        if (snapshot.specializedView == "session") return "Session";
        if (snapshot.specializedView == "creation") return "Création";
        if (snapshot.specializedView == "post_combat") return "Après-combat";
        if (snapshot.specializedView == "altered_data") return "Données altérées";

        return "Général";
    }

    std::string inferItemActionType(const GuiMenuActionSnapshot& action)
    {
        const std::string text = action.actionId + " " + action.label + " " + action.hint;

        if (containsAny(text, {"buyback", "racheter", "rachat"})) return "buyback";
        if (containsAny(text, {"buy", "acheter", "achat"})) return "buy";
        if (containsAny(text, {"sell", "vendre", "vente", "revente"})) return "sell";
        if (containsAny(text, {"barter", "troquer", "troc"})) return "barter";
        if (containsAny(text, {"equip", "équiper", "equiper"})) return "equip";
        if (containsAny(text, {"use", "utiliser", "boire"})) return "use";
        if (containsAny(text, {"repair", "réparer", "reparer"})) return "repair";
        if (containsAny(text, {"inspect", "inspecter", "voir", "consulter", "journal"})) return "inspect";
        if (containsAny(text, {"catalog", "catalogue", "classe", "race", "boss", "monstre", "personnage spécial", "personnage special"})) return "inspect";
        if (containsAny(text, {"activity.story", "histoire"})) return "story";
        if (containsAny(text, {"activity.combat", "combat.", "combats"})) return "combat";
        if (containsAny(text, {"session.", "solo", "coop", "multi local"})) return "session";
        if (containsAny(text, {"character.creation", "difficulty", "difficulté", "difficulte", "race", "class", "classe"})) return "create";
        if (containsAny(text, {"exploration", "biome", "intensité", "intensite", "sortie prudente", "sortie normale", "sortie audacieuse"})) return "travel";
        if (containsAny(text, {"quest", "quête", "quete", "mission", "guilde"})) return "quest";
        if (containsAny(text, {"death", "mort", "morts", "lethal", "létal", "survie", "corrompue"})) return "death";
        if (containsAny(text, {"cheat", "triche", "altéré", "altere", "altération", "alteration", "données altérées", "donnees alterees"})) return "altered_data";
        if (containsAny(text, {"save", "sauvegarde", "compte", "personnage", "creation", "création"})) return "save";
        if (containsAny(text, {"continue", "continuer", "retourner"})) return "continue";
        if (containsAny(text, {"location", "lieu", "lieux", "exploration"})) return "travel";
        if (containsAny(text, {"talk", "parler", "discuter"})) return "talk";
        if (containsAny(text, {"select", "choisir", "sélection", "selection"})) return "select";
        return "open";
    }

    std::string labelForItemActionType(const std::string& actionType)
    {
        if (actionType == "buyback") return "Racheter";
        if (actionType == "buy") return "Acheter";
        if (actionType == "sell") return "Vendre";
        if (actionType == "barter") return "Troquer";
        if (actionType == "equip") return "Équiper";
        if (actionType == "use") return "Utiliser";
        if (actionType == "repair") return "Réparer";
        if (actionType == "inspect") return "Inspecter";
        if (actionType == "story") return "Histoire";
        if (actionType == "combat") return "Combat";
        if (actionType == "session") return "Session";
        if (actionType == "create") return "Créer";
        if (actionType == "quest") return "Quête";
        if (actionType == "save") return "Sauvegarder";
        if (actionType == "death") return "Consulter";
        if (actionType == "altered_data") return "Consulter";
        if (actionType == "continue") return "Continuer";
        if (actionType == "travel") return "Voyager";
        if (actionType == "talk") return "Parler";
        if (actionType == "select") return "Choisir";
        return "Ouvrir";
    }

    bool shouldCreateItemCard(const GuiMenuSnapshot& snapshot, const GuiMenuActionSnapshot& action)
    {
        if (!action.enabled || action.number < 0 || action.group != "main" || action.dangerous)
        {
            return false;
        }

        if (snapshot.specializedView == "combat" || snapshot.specializedView == "generic")
        {
            return false;
        }

        const std::string text = snapshot.screenId + " " + action.actionId + " " + action.label;

        if (snapshot.specializedView == "creation" || snapshot.specializedView == "save")
        {
            return action.hasItemMetadata || action.label.find('|') != std::string::npos;
        }

        if (containsAny(text, {".select", "select", "article", "stock", "client", "guild", "guilde", "quest", "quête", "quete"}))
        {
            return true;
        }

        if (action.label.find('|') != std::string::npos)
        {
            return true;
        }

        return snapshot.specializedView == "inventory" ||
               snapshot.specializedView == "equipment" ||
               snapshot.specializedView == "potions" ||
               snapshot.specializedView == "shop" ||
               snapshot.specializedView == "exploration" ||
               snapshot.specializedView == "quest" ||
               snapshot.specializedView == "progression" ||
               snapshot.specializedView == "catalog" ||
               snapshot.specializedView == "activity" ||
               snapshot.specializedView == "session" ||
               snapshot.specializedView == "creation" ||
               snapshot.specializedView == "post_combat" ||
               snapshot.specializedView == "save" ||
               snapshot.specializedView == "death" ||
               snapshot.specializedView == "altered_data";
    }


    bool startsWithBracketNumberChoice(const std::string& rawLine)
    {
        const std::string line = trimCopy(rawLine);
        if (line.size() < 4 || line.front() != '[')
        {
            return false;
        }

        std::size_t index = 1;
        bool hasDigit = false;
        while (index < line.size() && std::isdigit(static_cast<unsigned char>(line[index])))
        {
            hasDigit = true;
            ++index;
        }

        return hasDigit && index < line.size() && line[index] == ']';
    }

    bool isListEntryStart(const std::string& rawLine)
    {
        const std::string line = trimCopy(rawLine);
        if (line.empty() || isSeparatorLine(line))
        {
            return false;
        }

        if (line.rfind("- ", 0) == 0)
        {
            return true;
        }

        return startsWithNumberChoice(line) || startsWithBracketNumberChoice(line);
    }

    int extractEntryNumber(const std::string& rawLine)
    {
        const std::string line = trimCopy(rawLine);
        if (startsWithBracketNumberChoice(line))
        {
            const std::size_t close = line.find(']');
            try
            {
                return std::stoi(line.substr(1, close - 1));
            }
            catch (...)
            {
                return -1;
            }
        }

        std::size_t index = 0;

        while (index < line.size() && std::isdigit(static_cast<unsigned char>(line[index])))
        {
            ++index;
        }

        if (index == 0 || index >= line.size() || line[index] != ':')
        {
            return -1;
        }

        try
        {
            return std::stoi(line.substr(0, index));
        }
        catch (...)
        {
            return -1;
        }
    }

    std::string cleanListEntryName(const std::string& rawLine)
    {
        std::string line = trimCopy(rawLine);

        if (line.rfind("- ", 0) == 0)
        {
            line = trimCopy(line.substr(2));
        }
        else if (startsWithBracketNumberChoice(line))
        {
            const std::size_t close = line.find(']');
            line = trimCopy(line.substr(close + 1));
        }
        else
        {
            const std::size_t separator = line.find(':');
            if (separator != std::string::npos && separator + 1 < line.size())
            {
                line = trimCopy(line.substr(separator + 1));
            }
        }

        const std::size_t pipe = line.find('|');
        if (pipe != std::string::npos)
        {
            line = trimCopy(line.substr(0, pipe));
        }

        return line;
    }

    void addLineItemCard(
        GuiMenuSnapshot& snapshot,
        const std::vector<std::string>& block,
        int entryNumber,
        std::size_t maxCards
    )
    {
        if (snapshot.itemCards.size() >= maxCards || block.empty())
        {
            return;
        }

        std::string name = cleanListEntryName(block.front());
        if (name.empty())
        {
            return;
        }

        std::string quantity = extractQuantityFromName(name);

        std::vector<std::string> detailParts;
        std::string status = inferItemCardStatus(block.front());
        std::string price;
        std::string stock;
        std::string reward;
        std::string progress;
        std::string owner;

        const std::vector<std::string> firstParts = splitByPipe(block.front());
        for (std::size_t i = 1; i < firstParts.size(); ++i)
        {
            detailParts.push_back(firstParts[i]);
        }

        for (std::size_t i = 1; i < block.size(); ++i)
        {
            const std::string line = trimCopy(block[i]);
            if (line.empty() || isSeparatorLine(line))
            {
                continue;
            }

            detailParts.push_back(line);
            if (status.empty()) status = inferItemCardStatus(line);
        }

        price = findMetadataPart(detailParts, {"prix", "or", "gold"});
        stock = findMetadataPart(detailParts, {"stock", "quantité", "quantite", "exemplaire"});
        reward = findMetadataPart(detailParts, {"récompense", "recompense", "reward", "xp", "butin"});
        progress = findMetadataPart(detailParts, {"progression", "objectif", "état", "etat", "niveau"});
        owner = findMetadataPart(detailParts, {"client", "origine", "interlocuteur", "lieu", "famille", "race", "classe native", "type", "catégorie", "categorie"});

        GuiMenuActionSnapshot lineAction;
        lineAction.label = block.front();
        lineAction.hint = joinTextParts(detailParts, 0);
        lineAction.actionId = snapshot.screenId + ".line";

        GuiMenuItemSnapshot card;
        card.id = "line_item." + std::to_string(snapshot.itemCards.size() + 1);
        card.kind = inferItemCardKind(snapshot, lineAction);
        card.section = inferItemCardSection(snapshot, lineAction);
        card.actionType = "inspect";
        card.actionTypeLabel = labelForItemActionType(card.actionType);
        card.name = name;
        card.quantity = quantity;
        card.detail = joinTextParts(detailParts, 0);
        card.status = status;
        card.price = price;
        card.stock = stock;
        card.reward = reward;
        card.progress = progress;
        card.owner = owner;
        card.actionNumber = entryNumber;
        card.actionDisplayLabel = entryNumber >= 0 ? std::to_string(entryNumber) + " - " + name : name;
        card.available = false;
        card.important = !status.empty() || !reward.empty() || card.kind == "boss" || card.kind == "special_character" || card.kind == "class" || card.kind == "race";
        card.metadataSource = "lines";

        snapshot.itemCards.push_back(card);
    }

    void populateLineItemCards(GuiMenuSnapshot& snapshot, std::size_t maxCards)
    {
        if (!snapshot.itemCards.empty() || snapshot.lines.empty() || snapshot.specializedView == "combat" || snapshot.specializedView == "generic")
        {
            return;
        }

        std::vector<std::string> currentBlock;
        int currentNumber = -1;

        auto flushBlock = [&]()
        {
            if (!currentBlock.empty())
            {
                addLineItemCard(snapshot, currentBlock, currentNumber, maxCards);
                currentBlock.clear();
                currentNumber = -1;
            }
        };

        for (const std::string& rawLine : snapshot.lines)
        {
            if (isListEntryStart(rawLine))
            {
                flushBlock();
                currentBlock.push_back(rawLine);
                currentNumber = extractEntryNumber(rawLine);
            }
            else if (!currentBlock.empty())
            {
                currentBlock.push_back(rawLine);
            }
        }

        flushBlock();
    }

    void populateItemCards(GuiMenuSnapshot& snapshot)
    {
        if (snapshot.specializedView == "generic")
        {
            return;
        }

        constexpr std::size_t maxCards = 24;

        for (const GuiMenuActionSnapshot& action : snapshot.actions)
        {
            if (snapshot.itemCards.size() >= maxCards || !shouldCreateItemCard(snapshot, action))
            {
                continue;
            }

            const std::vector<std::string> parts = splitByPipe(action.label);
            std::string name = parts.empty() ? trimCopy(action.label) : parts.front();
            std::string quantity = extractQuantityFromName(name);
            std::string detail = parts.size() > 1 ? joinTextParts(parts, 1) : trimCopy(action.hint);
            std::string status = inferItemCardStatus(action.label + " " + action.hint);
            std::string price = findMetadataPart(parts, {"prix", "or", "gold"});
            std::string stock = findMetadataPart(parts, {"stock", "épuisé", "epuise"});
            std::string maxQuantity = findMetadataPart(parts, {"max", "quantité", "quantite", "possible"});
            std::string reward = findMetadataPart(parts, {"récompense", "recompense", "reward", "xp", "objet"});
            std::string progress = findMetadataPart(parts, {"progression", "objectif", "état", "etat"});
            std::string owner = findMetadataPart(parts, {"client", "origine", "interlocuteur", "lieu", "type", "catégorie", "categorie"});
            std::string kind = inferItemCardKind(snapshot, action);
            std::string section = inferItemCardSection(snapshot, action);
            std::string actionType = inferItemActionType(action);
            std::string metadataSource = "label";
            bool important = false;

            if (action.hasItemMetadata)
            {
                metadataSource = "structured";
                if (!action.itemName.empty()) name = action.itemName;
                if (!action.itemQuantity.empty()) quantity = action.itemQuantity;
                if (!action.itemDetail.empty()) detail = action.itemDetail;
                if (!action.itemStatus.empty()) status = action.itemStatus;
                if (!action.itemPrice.empty()) price = action.itemPrice;
                if (!action.itemStock.empty()) stock = action.itemStock;
                if (!action.itemMaxQuantity.empty()) maxQuantity = action.itemMaxQuantity;
                if (!action.itemReward.empty()) reward = action.itemReward;
                if (!action.itemProgress.empty()) progress = action.itemProgress;
                if (!action.itemOwner.empty()) owner = action.itemOwner;
                if (!action.itemKind.empty()) kind = action.itemKind;
                if (!action.itemSection.empty()) section = action.itemSection;
                if (!action.itemActionType.empty()) actionType = action.itemActionType;
                important = action.itemImportant;
            }

            if (name.empty())
            {
                name = action.actionDisplayLabel.empty() ? action.label : action.actionDisplayLabel;
            }

            GuiMenuItemSnapshot card;
            card.id = "item." + std::to_string(snapshot.itemCards.size() + 1);

            card.kind = kind;
            card.section = section;
            card.actionType = actionType;
            card.actionTypeLabel = labelForItemActionType(actionType);
            card.name = name;
            card.quantity = quantity;
            card.detail = detail;
            card.status = status;
            card.price = price;
            card.stock = stock;
            card.maxQuantity = maxQuantity;
            card.reward = reward;
            card.progress = progress;
            card.owner = owner;
            card.metadataSource = metadataSource;
            card.actionNumber = action.number;
            card.actionId = action.actionId;
            card.actionLabel = action.label;
            card.actionDisplayLabel = action.actionDisplayLabel;
            card.available = action.enabled;
            card.important = important || !status.empty() || !reward.empty() || !progress.empty() || card.kind == "weapon" || card.kind == "armor" || card.kind == "quest";

            snapshot.itemCards.push_back(card);
        }

        populateLineItemCards(snapshot, maxCards);
    }

    void populateFocusCards(GuiMenuSnapshot& snapshot)
    {
        if (snapshot.specializedView == "inventory")
        {
            addFocusCard(snapshot, "inventory.all", "Voir tout", "Liste complète ou catégorie générale.", {"voir tout", "inventory.all", "inventaire.tout", "all"});
            addFocusCard(snapshot, "inventory.weapons", "Armes", "Armes, raretés et équipement offensif.", {"arme", "weapon"});
            addFocusCard(snapshot, "inventory.armor", "Armures", "Tenues, armures et protections.", {"armure", "armor", "tenue"});
            addFocusCard(snapshot, "inventory.materials", "Matériaux", "Ressources, composants et récoltes.", {"materiau", "matériau", "material"});
        }
        else if (snapshot.specializedView == "equipment")
        {
            addFocusCard(snapshot, "equipment.view", "Voir équipement", "Résumé rapide ou détaillé selon l'écran.", {"voir", "equipment.view", "equipement.view", "équipement"});
            addFocusCard(snapshot, "equipment.weapon", "Changer arme", "Changer l'arme utilisée.", {"arme", "weapon"});
            addFocusCard(snapshot, "equipment.armor", "Changer tenue", "Changer l'armure ou la tenue.", {"armure", "armor", "tenue"});
        }
        else if (snapshot.specializedView == "potions")
        {
            addFocusCard(snapshot, "potions.heal", "Soin", "Potions curatives et soin rapide.", {"heal", "soin", "curative", "quick_heal", "potion de soin"});
            addFocusCard(snapshot, "potions.defense", "Défense", "Protection, résistance et survie.", {"defensive", "défensive", "protection", "resistance", "résistance"});
            addFocusCard(snapshot, "potions.offense", "Offensif", "Potions lancées ou agressives.", {"offensive", "dégâts", "degats", "debuff"});
            addFocusCard(snapshot, "potions.buff", "Buff", "Renforts temporaires.", {"buff", "renforcer"});
        }
        else if (snapshot.specializedView == "shop")
        {
            addFocusCard(snapshot, "shop.stock", "Stock", "Voir les offres disponibles.", {"stock", "voir", "shop.stock"});
            addFocusCard(snapshot, "shop.buy", "Acheter", "Acheter avec de l'or si possible.", {"acheter", "buy", "shop.item.buy"});
            addFocusCard(snapshot, "shop.sell", "Vendre", "Vendre des objets ou ressources.", {"vendre", "sell", "vente"});
            addFocusCard(snapshot, "shop.buyback", "Rachat", "Récupérer une vente récente avant le prochain combat.", {"racheter", "rachat", "buyback"});
            addFocusCard(snapshot, "shop.barter", "Troc", "Échanges contre objets ou matériaux.", {"troquer", "troc", "barter"});
        }
        else if (snapshot.specializedView == "exploration")
        {
            addFocusCard(snapshot, "exploration.biomes", "Biomes", "Choisir le terrain à fouiller.", {"biome", "biomes", "exploration.biome", "plaine", "route", "forêt", "foret", "marais", "ruines"});
            addFocusCard(snapshot, "exploration.intensity", "Intensité", "Choisir le niveau de risque de la sortie.", {"intensité", "intensite", "sortie prudente", "sortie normale", "sortie audacieuse", "exploration.intensity"});
            addFocusCard(snapshot, "exploration.quest", "Objectifs", "Repérer les zones liées aux quêtes actives.", {"objectif", "quête probable", "quete probable", "quest"});
            addFocusCard(snapshot, "exploration.resources", "Ressources", "Matériaux, plantes, coffres et traces possibles.", {"matériau", "materiau", "ressource", "coffre", "plante", "traces"});
        }
        else if (snapshot.specializedView == "quest")
        {
            addFocusCard(snapshot, "quest.journal", "Journal", "Consulter les quêtes actives et terminées.", {"journal", "quest.hub.journal", "quest.guild.journal"});
            addFocusCard(snapshot, "quest.guild", "Guilde", "Panneau, rangs et missions de guilde.", {"guilde", "guild"});
            addFocusCard(snapshot, "quest.turn_in", "Rendre", "Rendre une quête terminée.", {"rendre", "turn_in", "terminée", "terminee"});
            addFocusCard(snapshot, "quest.locations", "Lieux", "Accéder aux lieux visitables.", {"lieux", "locations", "exploration"});
        }
        else if (snapshot.specializedView == "progression")
        {
            addFocusCard(snapshot, "progression.bestiary", "Bestiaire", "Connaissance des entités rencontrées.", {"bestiaire", "bestiary"});
            addFocusCard(snapshot, "progression.materials", "Matériaux", "Connaissance et expérimentation des matériaux.", {"materiau", "matériau", "material"});
            addFocusCard(snapshot, "progression.statistics", "Statistiques", "Progression du personnage ou du compte.", {"statistique", "statistics"});
            addFocusCard(snapshot, "progression.knowledge", "Connaissances", "Informations débloquées sans spoiler caché.", {"knowledge", "connaissance"});
        }
        else if (snapshot.specializedView == "catalog")
        {
            addFocusCard(snapshot, "catalog.races", "Races", "Identités jouables et bonus de départ.", {"race", "races"});
            addFocusCard(snapshot, "catalog.classes", "Classes", "Familles, rôles et statistiques de départ.", {"classe", "classes", "class"});
            addFocusCard(snapshot, "catalog.boss", "Boss", "Registre des boss débloqués ou brouillés.", {"boss"});
            addFocusCard(snapshot, "catalog.specials", "Spéciaux", "Personnages et groupes spéciaux.", {"spécial", "special", "personnage", "groupe"});
        }
        else if (snapshot.specializedView == "activity")
        {
            addFocusCard(snapshot, "activity.story", "Histoire", "Route principale visible, encore scellée par les archives.", {"histoire", "story", "activity.story"});
            addFocusCard(snapshot, "activity.combat", "Combats", "PvE, boss, groupes et JcJ local.", {"combat", "combats", "pvp", "pve", "boss"});
            addFocusCard(snapshot, "activity.exploration", "Exploration", "Biomes, ressources, coffres, pièges et rencontres.", {"exploration", "biome", "biomes"});
            addFocusCard(snapshot, "activity.management", "Gestion", "Après-combat, inventaire, sauvegarde, quêtes et lieux.", {"gestion", "après-combat", "apres-combat", "boutiques", "lieux", "quêtes", "quetes", "sauvegarder"});
        }
        else if (snapshot.specializedView == "session")
        {
            addFocusCard(snapshot, "session.solo", "Solo", "Un seul personnage actif.", {"solo", "session.solo"});
            addFocusCard(snapshot, "session.coop2", "Coop 2", "Un allié joueur local intervient surtout en combat.", {"2 joueurs", "coop.2", "multi local - 2"});
            addFocusCard(snapshot, "session.coop3", "Coop 3", "Deux alliés joueurs locaux avec récompenses séparées.", {"3 joueurs", "coop.3", "multi local - 3"});
            addFocusCard(snapshot, "session.group", "Groupe", "Résumé du groupe ou choix du personnage secondaire.", {"groupe", "joueur 2", "joueur 3", "personnage joueur"});
        }
        else if (snapshot.specializedView == "creation")
        {
            addFocusCard(snapshot, "creation.difficulty", "Difficulté", "Impacte mort, ressources, respawn et récompenses.", {"difficulté", "difficulte", "difficulty"});
            addFocusCard(snapshot, "creation.race", "Race", "Origine, affinités et bonus de départ.", {"race", "races"});
            addFocusCard(snapshot, "creation.class", "Classe", "Style de combat, rôle et kit initial.", {"classe", "class", "famille"});
            addFocusCard(snapshot, "creation.identity", "Identité", "Nom, identité protégée et validation finale.", {"nom", "identité", "identite", "personnage"});
        }
        else if (snapshot.specializedView == "post_combat")
        {
            addFocusCard(snapshot, "post.shop", "Boutiques", "Acheter, vendre ou consulter les offres.", {"boutique", "shop"});
            addFocusCard(snapshot, "post.inventory", "Inventaire", "Gérer objets, équipement et potions.", {"inventaire", "inventory", "équipement", "equipement", "potions"});
            addFocusCard(snapshot, "post.quests", "Quêtes", "Consulter ou rendre des quêtes.", {"quête", "quete", "quest"});
            addFocusCard(snapshot, "post.save", "Sauvegarde", "Sauvegarder la progression.", {"sauvegarde", "save"});
        }
        else if (snapshot.specializedView == "save")
        {
            const bool isAccountList = containsAny(snapshot.screenId, {"save.accounts.list"});
            const bool isAccountActions = containsAny(snapshot.screenId, {"save.accounts.actions"});
            const bool isCharacterList = containsAny(snapshot.screenId, {"save.characters.list"});
            const bool isCharacterActions = containsAny(snapshot.screenId, {"save.characters.actions"});

            if (isCharacterActions)
            {
                addFocusCard(snapshot, "save.play", "Incarner", "Charger le personnage sélectionné.", {"incarner", "play", "charger"});
                addFocusCard(snapshot, "save.transfer", "Transfert", "Exporter, cloner ou transférer une maîtrise.", {"transfer", "transfert", "extraire", "clone"});
                addFocusCard(snapshot, "save.delete", "Suppression", "Action irréversible à confirmer.", {"supprimer", "delete", "irréversible", "irreversible"});
            }
            else if (isCharacterList)
            {
                addFocusCard(snapshot, "save.character_select", "Sélectionner", "Choisir un personnage existant avant de proposer Incarner.", {"save.characters.select", "personnage"});
                addFocusCard(snapshot, "save.create", "Créer", "Créer ou préparer un nouveau personnage.", {"créer", "creer", "nouveau", "create", "new"});
            }
            else if (isAccountActions)
            {
                addFocusCard(snapshot, "save.account_login", "Se connecter", "Ouvrir le compte sélectionné.", {"save.accounts.login", "connecter", "connexion"});
                addFocusCard(snapshot, "save.transfer", "Transfert", "Exporter ou importer les données du compte.", {"transfer", "transfert", "extraire", "importer", "exporter"});
                addFocusCard(snapshot, "save.delete", "Suppression", "Action irréversible à confirmer.", {"supprimer", "delete", "irréversible", "irreversible"});
            }
            else if (isAccountList)
            {
                addFocusCard(snapshot, "save.account_select", "Compte existant", "Sélectionner un compte avant les actions de compte.", {"save.accounts.select", "compte"});
                addFocusCard(snapshot, "save.account_create", "Créer compte", "Créer un nouveau compte.", {"save.accounts.create", "créer", "creer", "nouveau", "create", "new"});
                addFocusCard(snapshot, "save.account_import", "Importer", "Restaurer un compte exporté.", {"save.accounts.import", "importer"});
            }
            else
            {
                addFocusCard(snapshot, "save.create", "Créer", "Créer ou préparer une sauvegarde.", {"créer", "creer", "nouveau", "create", "new"});
                addFocusCard(snapshot, "save.transfer", "Transfert", "Exporter, importer ou cloner une maîtrise.", {"transfer", "transfert", "extraire", "clone", "importer", "exporter"});
                addFocusCard(snapshot, "save.delete", "Suppression", "Action irréversible à confirmer.", {"supprimer", "delete", "irréversible", "irreversible"});
            }
        }
        else if (snapshot.specializedView == "death")
        {
            addFocusCard(snapshot, "death.losses", "Pertes", "Or, expérience, consommables et durabilité perdus.", {"perdu", "perdue", "perdus", "or", "expérience", "experience", "durabilité", "durabilite"});
            addFocusCard(snapshot, "death.equipment", "Équipement", "Arme ou armure brisée, volée ou protégée.", {"arme", "armure", "équipement", "equipement", "brisée", "brisee", "volée", "volee"});
            addFocusCard(snapshot, "death.survival", "Survie", "Anomalie, protection ou retour du personnage.", {"survie", "anomalie", "survit", "encore"});
            addFocusCard(snapshot, "death.lethal", "Léthal", "Mort définitive ou registre corrompu.", {"lethal", "létal", "définitive", "definitive", "corrompue"});
        }
        else if (snapshot.specializedView == "altered_data")
        {
            addFocusCard(snapshot, "altered.known", "Altérations connues", "Liste les codes déjà révélés sans les mélanger au menu normal.", {"altération", "alteration", "altéré", "altere", "données", "donnees", "code"});
            addFocusCard(snapshot, "altered.toggle", "Activer / désactiver", "Permet de basculer une altération connue quand le menu le propose.", {"activer", "désactiver", "desactiver", "toggle"});
            addFocusCard(snapshot, "altered.enter", "Entrer un code", "Saisie discrète d'un code déjà assumé par le joueur.", {"entrer", "code", "saisir"});
            addFocusCard(snapshot, "altered.back", "Retour", "Quitter les données altérées sans changer l'état du personnage.", {"retour", "quitter", "back"});
        }
        else if (snapshot.specializedView == "combat")
        {
            addFocusCard(snapshot, "combat.heal", "Soin", "Signalé seulement dans les cas pertinents.", {"heal", "soin", "quick_heal", "potion de soin", "recommend_heal"});
            addFocusCard(snapshot, "combat.potions", "Potions", "Ouvrir les consommables de combat.", {"potion", "potions"});
            addFocusCard(snapshot, "combat.equipment", "Équipement", "Changer ou consulter l'équipement.", {"équipement", "equipement", "equipment"});
            addFocusCard(snapshot, "combat.escape", "Fuite", "Disponible seulement si le mode le permet.", {"fuir", "fuite", "escape"});
        }
    }

}

MenuScreen::MenuScreen(const std::string& title, const std::string& screenId)
    : screenId(screenId),
      title(title)
{
}

void MenuScreen::setTitle(const std::string& value)
{
    title = value;
}

void MenuScreen::setScreenId(const std::string& value)
{
    screenId = value;
}

void MenuScreen::setChoiceInput(const std::string& hint)
{
    inputMode = "choice";
    inputPlaceholder.clear();
    inputHint = hint;
    confirmationKeyword.clear();
    hasNumericRange = false;
    numericMin = 0;
    numericMax = 0;
    acceptsEmptyInput = false;
    hasTextLengthRange = false;
    textMinLength = 0;
    textMaxLength = 0;
}

void MenuScreen::setTextInput(
    const std::string& placeholder,
    const std::string& hint,
    bool allowEmpty,
    int minLength,
    int maxLength
)
{
    inputMode = "text";
    inputPlaceholder = placeholder;
    inputHint = hint;
    confirmationKeyword.clear();
    hasNumericRange = false;
    numericMin = 0;
    numericMax = 0;
    acceptsEmptyInput = allowEmpty;
    textMinLength = std::max(0, minLength);
    textMaxLength = std::max(0, maxLength);
    hasTextLengthRange = textMinLength > 0 || textMaxLength > 0;
}

void MenuScreen::setQuantityInput(int minValue, int maxValue, const std::string& hint)
{
    inputMode = "quantity";
    numericMin = minValue;
    numericMax = maxValue;
    hasNumericRange = true;
    inputPlaceholder = std::to_string(minValue) + "-" + std::to_string(maxValue);
    inputHint = hint.empty()
        ? "Entre une quantité comprise entre " + std::to_string(minValue) + " et " + std::to_string(maxValue) + "."
        : hint;
    confirmationKeyword.clear();
    acceptsEmptyInput = false;
    hasTextLengthRange = false;
    textMinLength = 0;
    textMaxLength = 0;
}

void MenuScreen::setConfirmationInput(const std::string& keyword, const std::string& hint)
{
    inputMode = "confirmation";
    inputPlaceholder = keyword;
    inputHint = hint.empty() ? "Tape le mot demandé pour confirmer." : hint;
    confirmationKeyword = keyword;
    hasNumericRange = false;
    numericMin = 0;
    numericMax = 0;
    acceptsEmptyInput = false;
    hasTextLengthRange = false;
    textMinLength = 0;
    textMaxLength = 0;
}

void MenuScreen::setContinueInput(const std::string& hint)
{
    inputMode = "continue";
    inputPlaceholder.clear();
    inputHint = hint;
    confirmationKeyword.clear();
    hasNumericRange = false;
    numericMin = 0;
    numericMax = 0;
    acceptsEmptyInput = true;
    hasTextLengthRange = false;
    textMinLength = 0;
    textMaxLength = 0;
}

void MenuScreen::setDisplayOnlyInput(const std::string& hint)
{
    inputMode = "display";
    inputPlaceholder.clear();
    inputHint = hint;
    confirmationKeyword.clear();
    hasNumericRange = false;
    numericMin = 0;
    numericMax = 0;
    acceptsEmptyInput = false;
    hasTextLengthRange = false;
    textMinLength = 0;
    textMaxLength = 0;
}

void MenuScreen::setPagination(std::size_t pageIndexValue, std::size_t totalPagesValue)
{
    hasPagination = true;
    totalPages = std::max<std::size_t>(1, totalPagesValue);
    pageIndex = std::min(pageIndexValue, totalPages - 1);
}

void MenuScreen::clearPagination()
{
    hasPagination = false;
    pageIndex = 0;
    totalPages = 1;
}

void MenuScreen::addSubtitle(const std::string& value)
{
    subtitles.push_back(value);
}

void MenuScreen::addLine(const std::string& value)
{
    lines.push_back(value);
}

void MenuScreen::addFooterLine(const std::string& value)
{
    footerLines.push_back(value);
}

void MenuScreen::addOption(
    int number,
    const std::string& label,
    const std::string& hint,
    bool enabled,
    const std::string& actionId,
    const MenuOptionItemData& itemData
)
{
    options.emplace_back(number, label, hint, enabled, actionId, itemData);
}

void MenuScreen::addBackOption(const std::string& label, const std::string& actionId)
{
    addOption(0, label, "", true, actionId);
}

const std::string& MenuScreen::getScreenId() const
{
    return screenId;
}

const std::string& MenuScreen::getTitle() const
{
    return title;
}

const std::string& MenuScreen::getInputMode() const
{
    return inputMode;
}

const std::string& MenuScreen::getInputPlaceholder() const
{
    return inputPlaceholder;
}

const std::string& MenuScreen::getInputHint() const
{
    return inputHint;
}

const std::string& MenuScreen::getConfirmationKeyword() const
{
    return confirmationKeyword;
}

bool MenuScreen::getHasNumericRange() const
{
    return hasNumericRange;
}

int MenuScreen::getNumericMin() const
{
    return numericMin;
}

int MenuScreen::getNumericMax() const
{
    return numericMax;
}

bool MenuScreen::getAcceptsEmptyInput() const
{
    return acceptsEmptyInput;
}

bool MenuScreen::getHasTextLengthRange() const
{
    return hasTextLengthRange;
}

int MenuScreen::getTextMinLength() const
{
    return textMinLength;
}

int MenuScreen::getTextMaxLength() const
{
    return textMaxLength;
}

bool MenuScreen::getHasPagination() const
{
    return hasPagination;
}

std::size_t MenuScreen::getPageIndex() const
{
    return pageIndex;
}

std::size_t MenuScreen::getTotalPages() const
{
    return totalPages;
}

const std::vector<std::string>& MenuScreen::getSubtitles() const
{
    return subtitles;
}

const std::vector<std::string>& MenuScreen::getLines() const
{
    return lines;
}

const std::vector<MenuOption>& MenuScreen::getOptions() const
{
    return options;
}

const std::vector<std::string>& MenuScreen::getFooterLines() const
{
    return footerLines;
}

int MenuScreen::getHighestOptionNumber() const
{
    int highest = 0;

    for (const MenuOption& option : options)
    {
        highest = std::max(highest, option.getNumber());
    }

    return highest;
}


GuiMenuSnapshot MenuScreen::toGuiSnapshot() const
{
    GuiMenuSnapshot snapshot;
    snapshot.screenId = screenId;
    snapshot.title = title;
    snapshot.screenCategory = inferScreenCategory(screenId, title);
    snapshot.specializedView = inferSpecializedView(snapshot.screenCategory);
    snapshot.specializedTitle = inferSpecializedTitle(snapshot.specializedView);
    snapshot.specializedHint = inferSpecializedHint(snapshot.specializedView);
    snapshot.inputMode = inputMode;
    snapshot.inputIntent = inferInputIntent(inputMode);
    snapshot.inputPlaceholder = inputPlaceholder;
    snapshot.inputHint = inputHint;
    snapshot.confirmationKeyword = confirmationKeyword;
    snapshot.expectedInputPattern = inferExpectedInputPattern(inputMode);
    snapshot.primarySubmitLabel = inferPrimarySubmitLabel(inputMode);
    snapshot.inputRequired = inferInputRequired(inputMode);
    snapshot.acceptsEmptyInput = acceptsEmptyInput;
    snapshot.hasNumericRange = hasNumericRange;
    snapshot.hasTextLengthRange = hasTextLengthRange;
    snapshot.textMinLength = textMinLength;
    snapshot.textMaxLength = textMaxLength;
    snapshot.numericMin = numericMin;
    snapshot.numericMax = numericMax;
    snapshot.hasPagination = hasPagination;
    snapshot.pageIndex = static_cast<int>(pageIndex);
    snapshot.pageNumber = static_cast<int>(pageIndex + 1);
    snapshot.totalPages = static_cast<int>(totalPages);
    snapshot.hasPreviousPage = hasPagination && pageIndex > 0;
    snapshot.hasNextPage = hasPagination && pageIndex + 1 < totalPages;
    snapshot.subtitles = subtitles;
    snapshot.lines = lines;
    snapshot.footerLines = footerLines;

    addUniqueTag(snapshot.contextTags, snapshot.screenCategory);
    addUniqueTag(snapshot.contextTags, "saisie:" + snapshot.inputIntent);

    if (inputMode == "confirmation")
    {
        addUniqueTag(snapshot.contextTags, "confirmation");
    }
    else if (inputMode == "continue")
    {
        addUniqueTag(snapshot.contextTags, "continuer");
    }
    else if (inputMode == "text")
    {
        addUniqueTag(snapshot.contextTags, "texte libre");
        if (!acceptsEmptyInput)
        {
            addUniqueTag(snapshot.contextTags, "texte requis");
        }
        if (hasTextLengthRange)
        {
            if (textMinLength > 0)
            {
                addUniqueTag(snapshot.contextTags, "texte min:" + std::to_string(textMinLength));
            }
            if (textMaxLength > 0)
            {
                addUniqueTag(snapshot.contextTags, "texte max:" + std::to_string(textMaxLength));
            }
        }
    }
    else if (inputMode == "quantity")
    {
        addUniqueTag(snapshot.contextTags, "quantite");
        if (hasNumericRange)
        {
            addUniqueTag(snapshot.contextTags, "min:" + std::to_string(numericMin));
            addUniqueTag(snapshot.contextTags, "max:" + std::to_string(numericMax));
        }
    }

    if (hasPagination)
    {
        addUniqueTag(snapshot.contextTags, "pagination");
        addUniqueTag(snapshot.contextTags, "page:" + std::to_string(snapshot.pageNumber) + "/" + std::to_string(snapshot.totalPages));
    }

    int recommendationCandidateIndex = -1;
    int recommendationCandidateCount = 0;

    for (const MenuOption& option : options)
    {
        GuiMenuActionSnapshot action;
        action.number = option.getNumber();
        action.label = option.getLabel();
        action.actionDisplayLabel = std::to_string(action.number) + " - " + action.label;
        action.hint = option.getHint();
        action.enabled = option.isEnabled();
        action.actionId = option.getActionId();
        action.role = inferActionRole(option);
        action.style = inferActionStyle(action.role);
        action.group = inferActionGroup(action.role);
        action.dangerous = action.role == "danger";
        action.knowledgeSensitive = isKnowledgeSensitiveAction(option);

        if (option.hasStructuredItemData())
        {
            const MenuOptionItemData& itemData = option.getItemData();
            action.hasItemMetadata = true;
            action.itemKind = itemData.kind;
            action.itemSection = itemData.section;
            action.itemActionType = itemData.actionType;
            action.itemName = itemData.name;
            action.itemQuantity = itemData.quantity;
            action.itemDetail = itemData.detail;
            action.itemStatus = itemData.status;
            action.itemPrice = itemData.price;
            action.itemStock = itemData.stock;
            action.itemMaxQuantity = itemData.maxQuantity;
            action.itemReward = itemData.reward;
            action.itemProgress = itemData.progress;
            action.itemOwner = itemData.owner;
            action.itemImportant = itemData.important;
        }

        snapshot.actionCount++;
        if (action.enabled)
        {
            snapshot.enabledActionCount++;
        }
        else
        {
            snapshot.disabledActionCount++;
        }

        if (action.group == "main")
        {
            snapshot.mainActionCount++;
        }
        else if (action.group == "navigation")
        {
            snapshot.navigationActionCount++;
        }
        else if (action.group == "danger")
        {
            snapshot.dangerActionCount++;
        }
        else if (action.group == "locked")
        {
            snapshot.lockedActionCount++;
        }

        if (!action.enabled)
        {
            snapshot.hasDisabledAction = true;
        }
        if (action.dangerous)
        {
            snapshot.hasDangerAction = true;
            addUniqueTag(snapshot.contextTags, "action dangereuse");
        }
        if (action.knowledgeSensitive)
        {
            snapshot.hasKnowledgeSensitiveAction = true;
            addUniqueTag(snapshot.contextTags, "connaissance prudente");
        }
        if (action.role == "pagination")
        {
            addUniqueTag(snapshot.contextTags, "pagination");
        }
        if (action.role == "back")
        {
            addUniqueTag(snapshot.contextTags, "retour");
        }

        if (action.enabled)
        {
            snapshot.validChoices.push_back(action.number);

            const bool combatHealRecommendation =
                snapshot.screenCategory == "combat" &&
                isCombatHealRecommendation(option) &&
                !action.dangerous &&
                !action.knowledgeSensitive;

            const bool canBeDiscreetRecommendation =
                action.group == "main" &&
                !action.dangerous &&
                !action.knowledgeSensitive &&
                !isCombatAttackAction(option);

            if (combatHealRecommendation)
            {
                recommendationCandidateIndex = static_cast<int>(snapshot.actions.size());
                recommendationCandidateCount = 1;
                snapshot.recommendationMode = "combat_heal_low_hp";
            }
            else if (canBeDiscreetRecommendation && snapshot.recommendationMode != "combat_heal_low_hp")
            {
                recommendationCandidateIndex = static_cast<int>(snapshot.actions.size());
                recommendationCandidateCount++;
            }
        }

        snapshot.actions.push_back(action);
    }

    if (recommendationCandidateCount == 1 &&
        recommendationCandidateIndex >= 0 &&
        recommendationCandidateIndex < static_cast<int>(snapshot.actions.size()))
    {
        GuiMenuActionSnapshot& recommendedAction = snapshot.actions[static_cast<std::size_t>(recommendationCandidateIndex)];
        recommendedAction.recommended = true;
        recommendedAction.recommendationReason = buildRecommendationReason(recommendedAction);
        snapshot.recommendedChoice = recommendedAction.number;
        snapshot.recommendedChoiceLabel = recommendedAction.label;
        if (snapshot.recommendationMode != "combat_heal_low_hp")
        {
            snapshot.recommendationMode = "discreet_single_safe_action";
        }
        snapshot.recommendationReason = recommendedAction.recommendationReason;
        addUniqueTag(snapshot.contextTags, "mise en avant discrete");
    }
    else if (recommendationCandidateCount > 1)
    {
        snapshot.recommendationMode = "none_free_choice";
        snapshot.recommendationReason = "Plusieurs actions possibles : aucune mise en avant pour garder le choix libre.";
        addUniqueTag(snapshot.contextTags, "choix libre");
    }

    if (snapshot.specializedView != "generic")
    {
        addUniqueTag(snapshot.contextTags, "vue:" + snapshot.specializedView);
        populateInfoCards(snapshot);
        populateNoticeCards(snapshot);
        populateItemCards(snapshot);
        populateFocusCards(snapshot);
        if (!snapshot.infoCards.empty())
        {
            addUniqueTag(snapshot.contextTags, "infos structurees:" + std::to_string(snapshot.infoCards.size()));
        }
        if (!snapshot.itemCards.empty())
        {
            addUniqueTag(snapshot.contextTags, "cartes metier:" + std::to_string(snapshot.itemCards.size()));
        }
        if (!snapshot.noticeCards.empty())
        {
            addUniqueTag(snapshot.contextTags, "notes:" + std::to_string(snapshot.noticeCards.size()));
        }
    }

    if (snapshot.hasDisabledAction)
    {
        addUniqueTag(snapshot.contextTags, "options verrouillees");
    }

    if (snapshot.hasKnowledgeSensitiveAction)
    {
        addUniqueTag(snapshot.contextTags, "aucune faiblesse cachee revelee");
    }

    return snapshot;
}
