#include <cassert>

#include "class_system/ClassCatalog.hpp"
#include "core/Random.hpp"
#include "entity/Player.hpp"
#include "item/weapon/WeaponCatalog.hpp"
#include "story/HestiaSoulArtifactSystem.hpp"

int main()
{
    Player player("ArtefactCombat", ClassCatalog::createClassByName("Guerrier"));
    assert(HestiaSoulArtifactSystem::grant(player, "Épée"));
    assert(!HestiaSoulArtifactSystem::isEquipped(player));
    assert(HestiaSoulArtifactSystem::equip(player));
    assert(HestiaSoulArtifactSystem::isEquipped(player));
    assert(player.hasEquippedWeapon());

    Weapon artifact = player.getEquippedWeapon();
    assert(artifact.getPersistentId() == "hestia_soul_artifact");
    assert(artifact.isIndestructible());
    assert(artifact.isSoulBound());
    assert(artifact.isUniqueRecoveryItem());
    assert(artifact.isPersonalRecoveryItem());

    Random random(1313);
    bool sawArtifactHit = false;
    for (int i = 0; i < 40 && !sawArtifactHit; ++i)
    {
        bool dodged = false;
        bool critical = false;
        const int dealt = player.attack(random, dodged, critical, 0);
        if (!dodged)
        {
            assert(dealt == 3 || dealt == 4); // niveau 1 : 3 dégâts de base, critique léger possible
            sawArtifactHit = true;
        }
    }
    assert(sawArtifactHit);

    const auto directKill = HestiaSoulArtifactSystem::recordKillingBlow(player, "race:Gobelin", true);
    assert(directKill.counted && directKill.levelUp);
    assert(player.getHestiaArtifactState().level == 2);
    assert(HestiaSoulArtifactSystem::damage(player) == 6);

    player.getInventory().addWeapon(WeaponCatalog::createRustySword());
    assert(player.equipWeapon(0));
    assert(!HestiaSoulArtifactSystem::isEquipped(player));
    assert(player.getEquippedWeapon().getPersistentId() != "hestia_soul_artifact");

    assert(HestiaSoulArtifactSystem::equip(player));
    assert(HestiaSoulArtifactSystem::unequip(player));
    assert(!HestiaSoulArtifactSystem::isEquipped(player));
    return 0;
}
