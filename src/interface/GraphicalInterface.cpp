// EN: GraphicalInterface.cpp keeps future GUI data extraction close to the current terminal models.
// FR: GraphicalInterface.cpp rapproche l'extraction de données IG des modèles terminal actuels.
// English: This file is part of Dinotofu. Code identifiers are written in English, while player-facing text can stay in French.
// Français : Ce fichier fait partie de Dinotofu. Les identifiants du code sont en anglais, tandis que les textes affichés au joueur peuvent rester en français.

#include "interface/GraphicalInterface.hpp"

#include "core/VersionInfo.hpp"
#include "interface/model/GuiSchemaVersion.hpp"

#include <sstream>

GuiMenuSnapshot GraphicalInterface::snapshotScreen(const MenuScreen& screen)
{
    return screen.toGuiSnapshot();
}

GuiCombatStateSnapshot GraphicalInterface::snapshotCombatState(const GuiCombatStateSnapshot& snapshot)
{
    return snapshot;
}

std::vector<std::string> GraphicalInterface::collectActionIds(const MenuScreen& screen)
{
    std::vector<std::string> actionIds;

    for (const MenuOption& option : screen.getOptions())
    {
        if (!option.getActionId().empty())
        {
            actionIds.push_back(option.getActionId());
        }
    }

    return actionIds;
}

std::string GraphicalInterface::describeScreenContract(const MenuScreen& screen)
{
    std::ostringstream output;
    const GuiMenuSnapshot snapshot = screen.toGuiSnapshot();
    output << "screen=" << screen.getScreenId()
           << "; title=" << screen.getTitle()
           << "; category=" << snapshot.screenCategory
           << "; input=" << snapshot.inputIntent
           << "; options=" << screen.getOptions().size();

    if (snapshot.hasDangerAction)
    {
        output << "; danger_action=yes";
    }

    return output.str();
}

std::string GraphicalInterface::describeCombatContract(const GuiCombatStateSnapshot& snapshot)
{
    std::ostringstream output;
    output << "combat=" << snapshot.screenId
           << "; title=" << snapshot.title
           << "; player_units=" << snapshot.playerUnits.size()
           << "; enemy_units=" << snapshot.enemyUnits.size()
           << "; neutral_units=" << snapshot.neutralUnits.size()
           << "; summary_lines=" << snapshot.summaryLines.size()
           << "; turn=" << snapshot.turnNumber;

    if (!snapshot.phase.empty())
    {
        output << "; phase=" << snapshot.phase;
    }

    return output.str();
}

std::string GraphicalInterface::menuSnapshotToJson(const GuiMenuSnapshot& snapshot)
{
    std::ostringstream json;
    json << "{\n";
    json << indent(1) << "\"type\": \"menu\",\n";
    json << indent(1) << "\"schemaVersion\": " << GuiSchemaVersion::Menu << ",\n";
    json << indent(1) << "\"gameVersion\": \"" << escapeJson(VersionInfo::currentVersion()) << "\",\n";
    json << indent(1) << "\"screenId\": \"" << escapeJson(snapshot.screenId) << "\",\n";
    json << indent(1) << "\"title\": \"" << escapeJson(snapshot.title) << "\",\n";
    json << indent(1) << "\"screenCategory\": \"" << escapeJson(snapshot.screenCategory) << "\",\n";
    json << indent(1) << "\"inputMode\": \"" << escapeJson(snapshot.inputMode) << "\",\n";
    json << indent(1) << "\"inputIntent\": \"" << escapeJson(snapshot.inputIntent) << "\",\n";
    json << indent(1) << "\"inputPlaceholder\": \"" << escapeJson(snapshot.inputPlaceholder) << "\",\n";
    json << indent(1) << "\"inputHint\": \"" << escapeJson(snapshot.inputHint) << "\",\n";
    json << indent(1) << "\"confirmationKeyword\": \"" << escapeJson(snapshot.confirmationKeyword) << "\",\n";
    json << indent(1) << "\"expectedInputPattern\": \"" << escapeJson(snapshot.expectedInputPattern) << "\",\n";
    json << indent(1) << "\"primarySubmitLabel\": \"" << escapeJson(snapshot.primarySubmitLabel) << "\",\n";
    json << indent(1) << "\"inputRequired\": " << (snapshot.inputRequired ? "true" : "false") << ",\n";
    json << indent(1) << "\"acceptsEmptyInput\": " << (snapshot.acceptsEmptyInput ? "true" : "false") << ",\n";
    json << indent(1) << "\"hasNumericRange\": " << (snapshot.hasNumericRange ? "true" : "false") << ",\n";
    json << indent(1) << "\"numericMin\": " << snapshot.numericMin << ",\n";
    json << indent(1) << "\"numericMax\": " << snapshot.numericMax << ",\n";
    json << indent(1) << "\"hasTextLengthRange\": " << (snapshot.hasTextLengthRange ? "true" : "false") << ",\n";
    json << indent(1) << "\"textMinLength\": " << snapshot.textMinLength << ",\n";
    json << indent(1) << "\"textMaxLength\": " << snapshot.textMaxLength << ",\n";
    json << indent(1) << "\"hasDangerAction\": " << (snapshot.hasDangerAction ? "true" : "false") << ",\n";
    json << indent(1) << "\"hasDisabledAction\": " << (snapshot.hasDisabledAction ? "true" : "false") << ",\n";
    json << indent(1) << "\"hasKnowledgeSensitiveAction\": " << (snapshot.hasKnowledgeSensitiveAction ? "true" : "false") << ",\n";
    json << indent(1) << "\"actionCount\": " << snapshot.actionCount << ",\n";
    json << indent(1) << "\"enabledActionCount\": " << snapshot.enabledActionCount << ",\n";
    json << indent(1) << "\"disabledActionCount\": " << snapshot.disabledActionCount << ",\n";
    json << indent(1) << "\"mainActionCount\": " << snapshot.mainActionCount << ",\n";
    json << indent(1) << "\"navigationActionCount\": " << snapshot.navigationActionCount << ",\n";
    json << indent(1) << "\"dangerActionCount\": " << snapshot.dangerActionCount << ",\n";
    json << indent(1) << "\"lockedActionCount\": " << snapshot.lockedActionCount << ",\n";
    json << indent(1) << "\"hasPagination\": " << (snapshot.hasPagination ? "true" : "false") << ",\n";
    json << indent(1) << "\"pageIndex\": " << snapshot.pageIndex << ",\n";
    json << indent(1) << "\"pageNumber\": " << snapshot.pageNumber << ",\n";
    json << indent(1) << "\"totalPages\": " << snapshot.totalPages << ",\n";
    json << indent(1) << "\"hasPreviousPage\": " << (snapshot.hasPreviousPage ? "true" : "false") << ",\n";
    json << indent(1) << "\"hasNextPage\": " << (snapshot.hasNextPage ? "true" : "false") << ",\n";
    json << indent(1) << "\"contextTags\": " << stringArrayToJson(snapshot.contextTags, 1) << ",\n";
    json << indent(1) << "\"validChoices\": " << intArrayToJson(snapshot.validChoices, 1) << ",\n";
    json << indent(1) << "\"recommendedChoice\": " << snapshot.recommendedChoice << ",\n";
    json << indent(1) << "\"recommendedChoiceLabel\": \"" << escapeJson(snapshot.recommendedChoiceLabel) << "\",\n";
    json << indent(1) << "\"recommendationMode\": \"" << escapeJson(snapshot.recommendationMode) << "\",\n";
    json << indent(1) << "\"recommendationSafety\": \"" << escapeJson(snapshot.recommendationSafety) << "\",\n";
    json << indent(1) << "\"visualAssetPolicy\": \"supplementary_only_no_hidden_information\",\n";
    json << indent(1) << "\"terminalVisualAssets\": \"disabled_and_not_toggleable\",\n";
    json << indent(1) << "\"recommendationReason\": \"" << escapeJson(snapshot.recommendationReason) << "\",\n";
    json << indent(1) << "\"specializedView\": \"" << escapeJson(snapshot.specializedView) << "\",\n";
    json << indent(1) << "\"specializedTitle\": \"" << escapeJson(snapshot.specializedTitle) << "\",\n";
    json << indent(1) << "\"specializedHint\": \"" << escapeJson(snapshot.specializedHint) << "\",\n";
    json << indent(1) << "\"focusCards\": [";

    if (!snapshot.focusCards.empty())
    {
        json << "\n";
    }

    for (std::size_t i = 0; i < snapshot.focusCards.size(); ++i)
    {
        const GuiMenuFocusSnapshot& card = snapshot.focusCards[i];
        json << indent(2) << "{\n";
        json << indent(3) << "\"id\": \"" << escapeJson(card.id) << "\",\n";
        json << indent(3) << "\"label\": \"" << escapeJson(card.label) << "\",\n";
        json << indent(3) << "\"detail\": \"" << escapeJson(card.detail) << "\",\n";
        json << indent(3) << "\"actionNumber\": " << card.actionNumber << ",\n";
        json << indent(3) << "\"actionId\": \"" << escapeJson(card.actionId) << "\",\n";
        json << indent(3) << "\"actionLabel\": \"" << escapeJson(card.actionLabel) << "\",\n";
        json << indent(3) << "\"actionDisplayLabel\": \"" << escapeJson(card.actionDisplayLabel) << "\",\n";
        json << indent(3) << "\"available\": " << (card.available ? "true" : "false") << "\n";
        json << indent(2) << "}";

        if (i + 1 < snapshot.focusCards.size())
        {
            json << ",";
        }
        json << "\n";
    }

    if (!snapshot.focusCards.empty())
    {
        json << indent(1);
    }

    json << "],\n";
    json << indent(1) << "\"infoCards\": [";

    if (!snapshot.infoCards.empty())
    {
        json << "\n";
    }

    for (std::size_t i = 0; i < snapshot.infoCards.size(); ++i)
    {
        const GuiMenuInfoSnapshot& card = snapshot.infoCards[i];
        json << indent(2) << "{\n";
        json << indent(3) << "\"id\": \"" << escapeJson(card.id) << "\",\n";
        json << indent(3) << "\"label\": \"" << escapeJson(card.label) << "\",\n";
        json << indent(3) << "\"value\": \"" << escapeJson(card.value) << "\",\n";
        json << indent(3) << "\"source\": \"" << escapeJson(card.source) << "\",\n";
        json << indent(3) << "\"kind\": \"" << escapeJson(card.kind) << "\",\n";
        json << indent(3) << "\"important\": " << (card.important ? "true" : "false") << "\n";
        json << indent(2) << "}";

        if (i + 1 < snapshot.infoCards.size())
        {
            json << ",";
        }
        json << "\n";
    }

    if (!snapshot.infoCards.empty())
    {
        json << indent(1);
    }

    json << "],\n";
    json << indent(1) << "\"itemCards\": [";

    if (!snapshot.itemCards.empty())
    {
        json << "\n";
    }

    for (std::size_t i = 0; i < snapshot.itemCards.size(); ++i)
    {
        const GuiMenuItemSnapshot& card = snapshot.itemCards[i];
        json << indent(2) << "{\n";
        json << indent(3) << "\"id\": \"" << escapeJson(card.id) << "\",\n";
        json << indent(3) << "\"kind\": \"" << escapeJson(card.kind) << "\",\n";
        json << indent(3) << "\"section\": \"" << escapeJson(card.section) << "\",\n";
        json << indent(3) << "\"actionType\": \"" << escapeJson(card.actionType) << "\",\n";
        json << indent(3) << "\"actionTypeLabel\": \"" << escapeJson(card.actionTypeLabel) << "\",\n";
        json << indent(3) << "\"name\": \"" << escapeJson(card.name) << "\",\n";
        json << indent(3) << "\"quantity\": \"" << escapeJson(card.quantity) << "\",\n";
        json << indent(3) << "\"detail\": \"" << escapeJson(card.detail) << "\",\n";
        json << indent(3) << "\"status\": \"" << escapeJson(card.status) << "\",\n";
        json << indent(3) << "\"price\": \"" << escapeJson(card.price) << "\",\n";
        json << indent(3) << "\"stock\": \"" << escapeJson(card.stock) << "\",\n";
        json << indent(3) << "\"maxQuantity\": \"" << escapeJson(card.maxQuantity) << "\",\n";
        json << indent(3) << "\"reward\": \"" << escapeJson(card.reward) << "\",\n";
        json << indent(3) << "\"progress\": \"" << escapeJson(card.progress) << "\",\n";
        json << indent(3) << "\"owner\": \"" << escapeJson(card.owner) << "\",\n";
        json << indent(3) << "\"actionNumber\": " << card.actionNumber << ",\n";
        json << indent(3) << "\"actionId\": \"" << escapeJson(card.actionId) << "\",\n";
        json << indent(3) << "\"actionLabel\": \"" << escapeJson(card.actionLabel) << "\",\n";
        json << indent(3) << "\"actionDisplayLabel\": \"" << escapeJson(card.actionDisplayLabel) << "\",\n";
        json << indent(3) << "\"available\": " << (card.available ? "true" : "false") << ",\n";
        json << indent(3) << "\"important\": " << (card.important ? "true" : "false") << ",\n";
        json << indent(3) << "\"metadataSource\": \"" << escapeJson(card.metadataSource) << "\"\n";
        json << indent(2) << "}";

        if (i + 1 < snapshot.itemCards.size())
        {
            json << ",";
        }
        json << "\n";
    }

    if (!snapshot.itemCards.empty())
    {
        json << indent(1);
    }

    json << "],\n";
    json << indent(1) << "\"noticeCards\": [";

    if (!snapshot.noticeCards.empty())
    {
        json << "\n";
    }

    for (std::size_t i = 0; i < snapshot.noticeCards.size(); ++i)
    {
        const GuiMenuNoticeSnapshot& card = snapshot.noticeCards[i];
        json << indent(2) << "{\n";
        json << indent(3) << "\"id\": \"" << escapeJson(card.id) << "\",\n";
        json << indent(3) << "\"text\": \"" << escapeJson(card.text) << "\",\n";
        json << indent(3) << "\"source\": \"" << escapeJson(card.source) << "\",\n";
        json << indent(3) << "\"kind\": \"" << escapeJson(card.kind) << "\",\n";
        json << indent(3) << "\"important\": " << (card.important ? "true" : "false") << "\n";
        json << indent(2) << "}";

        if (i + 1 < snapshot.noticeCards.size())
        {
            json << ",";
        }
        json << "\n";
    }

    if (!snapshot.noticeCards.empty())
    {
        json << indent(1);
    }

    json << "],\n";
    json << indent(1) << "\"subtitles\": " << stringArrayToJson(snapshot.subtitles, 1) << ",\n";
    json << indent(1) << "\"lines\": " << stringArrayToJson(snapshot.lines, 1) << ",\n";
    json << indent(1) << "\"footerLines\": " << stringArrayToJson(snapshot.footerLines, 1) << ",\n";
    json << indent(1) << "\"actions\": [";

    if (!snapshot.actions.empty())
    {
        json << "\n";
    }

    for (std::size_t i = 0; i < snapshot.actions.size(); ++i)
    {
        const GuiMenuActionSnapshot& action = snapshot.actions[i];
        json << indent(2) << "{\n";
        json << indent(3) << "\"number\": " << action.number << ",\n";
        json << indent(3) << "\"label\": \"" << escapeJson(action.label) << "\",\n";
        json << indent(3) << "\"actionDisplayLabel\": \"" << escapeJson(action.actionDisplayLabel) << "\",\n";
        json << indent(3) << "\"hint\": \"" << escapeJson(action.hint) << "\",\n";
        json << indent(3) << "\"enabled\": " << (action.enabled ? "true" : "false") << ",\n";
        json << indent(3) << "\"actionId\": \"" << escapeJson(action.actionId) << "\",\n";
        json << indent(3) << "\"role\": \"" << escapeJson(action.role) << "\",\n";
        json << indent(3) << "\"style\": \"" << escapeJson(action.style) << "\",\n";
        json << indent(3) << "\"group\": \"" << escapeJson(action.group) << "\",\n";
        json << indent(3) << "\"dangerous\": " << (action.dangerous ? "true" : "false") << ",\n";
        json << indent(3) << "\"knowledgeSensitive\": " << (action.knowledgeSensitive ? "true" : "false") << ",\n";
        json << indent(3) << "\"recommended\": " << (action.recommended ? "true" : "false") << ",\n";
        json << indent(3) << "\"recommendationReason\": \"" << escapeJson(action.recommendationReason) << "\",\n";
        json << indent(3) << "\"hasItemMetadata\": " << (action.hasItemMetadata ? "true" : "false") << ",\n";
        json << indent(3) << "\"itemKind\": \"" << escapeJson(action.itemKind) << "\",\n";
        json << indent(3) << "\"itemSection\": \"" << escapeJson(action.itemSection) << "\",\n";
        json << indent(3) << "\"itemActionType\": \"" << escapeJson(action.itemActionType) << "\",\n";
        json << indent(3) << "\"itemName\": \"" << escapeJson(action.itemName) << "\",\n";
        json << indent(3) << "\"itemQuantity\": \"" << escapeJson(action.itemQuantity) << "\",\n";
        json << indent(3) << "\"itemDetail\": \"" << escapeJson(action.itemDetail) << "\",\n";
        json << indent(3) << "\"itemStatus\": \"" << escapeJson(action.itemStatus) << "\",\n";
        json << indent(3) << "\"itemPrice\": \"" << escapeJson(action.itemPrice) << "\",\n";
        json << indent(3) << "\"itemStock\": \"" << escapeJson(action.itemStock) << "\",\n";
        json << indent(3) << "\"itemMaxQuantity\": \"" << escapeJson(action.itemMaxQuantity) << "\",\n";
        json << indent(3) << "\"itemReward\": \"" << escapeJson(action.itemReward) << "\",\n";
        json << indent(3) << "\"itemProgress\": \"" << escapeJson(action.itemProgress) << "\",\n";
        json << indent(3) << "\"itemOwner\": \"" << escapeJson(action.itemOwner) << "\",\n";
        json << indent(3) << "\"itemImportant\": " << (action.itemImportant ? "true" : "false") << "\n";
        json << indent(2) << "}";

        if (i + 1 < snapshot.actions.size())
        {
            json << ",";
        }
        json << "\n";
    }

    if (!snapshot.actions.empty())
    {
        json << indent(1);
    }

    json << "]\n";
    json << "}";
    return json.str();
}

std::string GraphicalInterface::combatSnapshotToJson(const GuiCombatStateSnapshot& snapshot)
{
    auto writeUnitArray = [](std::ostringstream& json, const std::string& name, const std::vector<GuiCombatUnitSnapshot>& units, int indentLevel)
    {
        auto writeStringArrayInline = [](std::ostringstream& stream, const std::vector<std::string>& values)
        {
            stream << "[";
            for (std::size_t i = 0; i < values.size(); ++i)
            {
                if (i > 0) stream << ", ";
                stream << "\"" << GraphicalInterface::escapeJson(values[i]) << "\"";
            }
            stream << "]";
        };

        json << GraphicalInterface::indent(indentLevel) << "\"" << name << "\": [";
        if (!units.empty()) json << "\n";

        for (std::size_t i = 0; i < units.size(); ++i)
        {
            const GuiCombatUnitSnapshot& unit = units[i];
            json << GraphicalInterface::indent(indentLevel + 1) << "{\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"unitId\": \"" << GraphicalInterface::escapeJson(unit.unitId) << "\",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"targetActionId\": \"" << GraphicalInterface::escapeJson(unit.targetActionId) << "\",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"targetChoice\": " << unit.targetChoice << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"name\": \"" << GraphicalInterface::escapeJson(unit.name) << "\",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"type\": \"" << GraphicalInterface::escapeJson(unit.type) << "\",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"side\": \"" << GraphicalInterface::escapeJson(unit.side) << "\",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"kind\": \"" << GraphicalInterface::escapeJson(unit.kind) << "\",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"level\": " << unit.level << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"hp\": " << unit.hp << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"maxHp\": " << unit.maxHp << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"minDamage\": " << unit.minDamage << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"maxDamage\": " << unit.maxDamage << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"criticalDamage\": " << unit.criticalDamage << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"healingPotionCount\": " << unit.healingPotionCount << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"damagePotionCount\": " << unit.damagePotionCount << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"dead\": " << (unit.dead ? "true" : "false") << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"targetable\": " << (unit.targetable ? "true" : "false") << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"healingThreatMarked\": " << (unit.healingThreatMarked ? "true" : "false") << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"provoking\": " << (unit.provoking ? "true" : "false") << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"defensePostureActive\": " << (unit.defensePostureActive ? "true" : "false") << ",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"defensePostureLabel\": \"" << GraphicalInterface::escapeJson(unit.defensePostureLabel) << "\",\n";
            json << GraphicalInterface::indent(indentLevel + 2) << "\"statusLabels\": ";
            writeStringArrayInline(json, unit.statusLabels);
            json << "\n";
            json << GraphicalInterface::indent(indentLevel + 1) << "}";

            if (i + 1 < units.size()) json << ",";
            json << "\n";
        }

        if (!units.empty()) json << GraphicalInterface::indent(indentLevel);
        json << "]";
    };

    std::ostringstream json;
    json << "{\n";
    json << indent(1) << "\"type\": \"combat\",\n";
    json << indent(1) << "\"schemaVersion\": " << GuiSchemaVersion::Combat << ",\n";
    json << indent(1) << "\"gameVersion\": \"" << escapeJson(VersionInfo::currentVersion()) << "\",\n";
    json << indent(1) << "\"screenId\": \"" << escapeJson(snapshot.screenId) << "\",\n";
    json << indent(1) << "\"title\": \"" << escapeJson(snapshot.title) << "\",\n";
    json << indent(1) << "\"phase\": \"" << escapeJson(snapshot.phase) << "\",\n";
    json << indent(1) << "\"activeUnitId\": \"" << escapeJson(snapshot.activeUnitId) << "\",\n";
    json << indent(1) << "\"currentActorName\": \"" << escapeJson(snapshot.currentActorName) << "\",\n";
    json << indent(1) << "\"currentTargetName\": \"" << escapeJson(snapshot.currentTargetName) << "\",\n";
    json << indent(1) << "\"escapeAvailable\": " << (snapshot.escapeAvailable ? "true" : "false") << ",\n";
    json << indent(1) << "\"turnNumber\": " << snapshot.turnNumber << ",\n";
    json << indent(1) << "\"summaryLines\": " << stringArrayToJson(snapshot.summaryLines, 1) << ",\n";
    writeUnitArray(json, "playerUnits", snapshot.playerUnits, 1);
    json << ",\n";
    writeUnitArray(json, "enemyUnits", snapshot.enemyUnits, 1);
    json << ",\n";
    writeUnitArray(json, "neutralUnits", snapshot.neutralUnits, 1);
    json << ",\n";
    json << indent(1) << "\"logLines\": " << stringArrayToJson(snapshot.logLines, 1) << "\n";
    json << "}";
    return json.str();
}

std::string GraphicalInterface::screenToJson(const MenuScreen& screen)
{
    return menuSnapshotToJson(snapshotScreen(screen));
}

std::string GraphicalInterface::escapeJson(const std::string& value)
{
    std::ostringstream escaped;

    for (unsigned char character : value)
    {
        switch (character)
        {
            case '"': escaped << "\\\""; break;
            case '\\': escaped << "\\\\"; break;
            case '\b': escaped << "\\b"; break;
            case '\f': escaped << "\\f"; break;
            case '\n': escaped << "\\n"; break;
            case '\r': escaped << "\\r"; break;
            case '\t': escaped << "\\t"; break;
            default:
                if (character < 0x20)
                {
                    escaped << "\\u";
                    const char* hex = "0123456789ABCDEF";
                    escaped << '0' << '0' << hex[(character >> 4) & 0x0F] << hex[character & 0x0F];
                }
                else
                {
                    escaped << character;
                }
                break;
        }
    }

    return escaped.str();
}

std::string GraphicalInterface::stringArrayToJson(const std::vector<std::string>& values, int indentLevel)
{
    if (values.empty())
    {
        return "[]";
    }

    std::ostringstream json;
    json << "[\n";

    for (std::size_t i = 0; i < values.size(); ++i)
    {
        json << indent(indentLevel + 1) << "\"" << escapeJson(values[i]) << "\"";
        if (i + 1 < values.size())
        {
            json << ",";
        }
        json << "\n";
    }

    json << indent(indentLevel) << "]";
    return json.str();
}


std::string GraphicalInterface::intArrayToJson(const std::vector<int>& values, int indentLevel)
{
    if (values.empty())
    {
        return "[]";
    }

    std::ostringstream json;
    json << "[";

    for (std::size_t i = 0; i < values.size(); ++i)
    {
        if (i > 0)
        {
            json << ", ";
        }
        json << values[i];
    }

    json << "]";
    (void)indentLevel;
    return json.str();
}

std::string GraphicalInterface::indent(int indentLevel)
{
    return std::string(static_cast<std::size_t>(indentLevel) * 2, ' ');
}
