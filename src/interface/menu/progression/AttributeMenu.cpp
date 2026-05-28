// EN: AttributeMenu.cpp briefly defines this Dinotofu module and its responsibilities.
// FR: AttributeMenu.cpp résume brièvement ce module de Dinotofu et ses responsabilités.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.
// Description: Lets the player spend DND-inspired attribute points after leveling up.

#include "interface/menu/progression/AttributeMenu.hpp"

#include "core/Console.hpp"
#include "interface/menu/common/MessageScreen.hpp"
#include "interface/TerminalInterface.hpp"
#include "interface/model/MenuScreen.hpp"
#include "progression/DndAttributes.hpp"

#include <string>

namespace
{
    std::string attributeCurrentValue(const Player& player, int choice)
    {
        const DndAttributes& attributes = player.getAttributes();

        if (choice == 1) return std::to_string(attributes.getStrength());
        if (choice == 2) return std::to_string(attributes.getDexterity());
        if (choice == 3) return std::to_string(attributes.getConstitution());
        if (choice == 4) return std::to_string(attributes.getIntelligence());
        if (choice == 5) return std::to_string(attributes.getWisdom());
        if (choice == 6) return std::to_string(attributes.getCharisma());

        return "?";
    }

    MenuScreen buildAttributeScreen(const Player& player)
    {
        MenuScreen screen("ATTRIBUTS", "attributes.spend");
        screen.addSubtitle("Points disponibles : " + std::to_string(player.getUnspentAttributePoints()));
        screen.addLine("Choisis l'attribut à renforcer. Les effets profonds restent liés aux futurs systèmes de progression.");
        screen.addBackOption("Retour", "attributes.back");

        for (int choice = 1; choice <= DndAttributes::getChoiceCount(); choice++)
        {
            MenuOptionItemData itemData;
            itemData.structured = true;
            itemData.kind = "attribute";
            itemData.section = "Attributs";
            itemData.actionType = "upgrade";
            itemData.name = DndAttributes::getChoiceName(choice);
            itemData.detail = DndAttributes::getChoiceDescription(choice);
            itemData.status = "Valeur actuelle : " + attributeCurrentValue(player, choice);
            itemData.progress = "Points disponibles : " + std::to_string(player.getUnspentAttributePoints());
            itemData.important = player.getUnspentAttributePoints() > 0;

            screen.addOption(
                choice,
                DndAttributes::getChoiceName(choice) + " | actuel : " + attributeCurrentValue(player, choice),
                DndAttributes::getChoiceDescription(choice),
                true,
                "attributes.upgrade." + std::to_string(choice),
                itemData
            );
        }

        return screen;
    }
}

void AttributeMenu::open(Player& player)
{
    while (true)
    {
        int choice = TerminalInterface::askMenuChoiceFromOptions(
            buildAttributeScreen(player),
            "Veuillez choisir un attribut affiché."
        );

        Console::clear();

        if (choice == 0)
        {
            return;
        }

        if (player.getUnspentAttributePoints() <= 0)
        {
            MessageScreen::show(
                "ATTRIBUTS",
                "attributes.no_points",
                {
                    "Tu n'as aucun point d'attribut à dépenser pour le moment.",
                    "Le registre gardera cette voie ouverte quand ton potentiel grandira."
                }
            );
            continue;
        }

        if (player.spendAttributePoint(choice))
        {
            MessageScreen::show(
                "ATTRIBUT AMÉLIORÉ",
                "attributes.upgrade.success",
                {
                    DndAttributes::getChoiceName(choice) + " augmente.",
                    "Nouvelle valeur : " + attributeCurrentValue(player, choice),
                    "Le personnage se rapproche doucement de sa vraie voie."
                }
            );
        }
        else
        {
            MessageScreen::show(
                "ATTRIBUT REFUSÉ",
                "attributes.upgrade.failed",
                {
                    "Impossible d'améliorer cet attribut.",
                    "Le registre refuse ce choix pour le moment."
                }
            );
        }
    }
}

void AttributeMenu::displayLockedDevelopmentMessage()
{
    MessageScreen::show(
        "ATTRIBUTS SCELLÉS",
        "attributes.locked_development",
        {
            "Tu sens que ton potentiel pourrait encore grandir...",
            "Mais les registres refusent encore de graver ce choix définitivement.",
            "[les runes restent muettes]"
        },
        false
    );
}
