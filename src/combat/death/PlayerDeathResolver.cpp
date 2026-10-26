#include "combat/death/PlayerDeathResolver.hpp"

#include "entity/Player.hpp"
#include "story/HestiaArcSystem.hpp"

PlayerDeathImmediateResolution PlayerDeathResolver::resolveImmediate(Player& player, const std::string& context)
{
    PlayerDeathImmediateResolution out;
    if (!player.isDead()) return out;

    const bool hestiaActuallyPresent = player.getHestiaStoryState().residence == "active_party";
    if (!HestiaArcSystem::consumeSacrificeForPlayer(player, hestiaActuallyPresent))
        return out;

    // Hestia extrait le joueur du combat. Elle sauve sa vie, mais ne transforme pas la défaite en victoire.
    player.reviveWithHealthPercentage(20);
    player.recordCanonicalEvent(
        "hestia_arc",
        "sacrifice_context:" + context,
        "Hestia a extrait le joueur d'une mort certaine et sa matérialisation a disparu",
        1);

    out.handled = true;
    out.hestiaSacrifice = true;
    out.lines = {
        "Au moment où tout s'arrête, Hestia agit avant même de comprendre ce qu'elle fait.",
        "Une lumière immense engloutit le combat et t'arrache au danger.",
        "Tu reprends conscience en sécurité avec une partie de tes forces.",
        "Hestia, elle, n'est plus matérialisée : elle a donné sa présence actuelle pour sauver ta vie.",
        "Cette intervention n'est possible que lorsqu'elle voyage réellement avec toi."
    };
    return out;
}
