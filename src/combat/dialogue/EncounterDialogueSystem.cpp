#include "combat/dialogue/EncounterDialogueSystem.hpp"
#include "combat/EnemyCombatQueue.hpp"
#include "combat/profile/MonsterBehaviorProfile.hpp"
#include "progression/language/LanguageSystem.hpp"
#include "character/SpecialCharacterDialogueCatalog.hpp"
#include "entity/Monster.hpp"
#include "entity/Player.hpp"
#include "core/Random.hpp"
#include "interface/menu/common/MessageScreen.hpp"

#include <algorithm>
#include <cctype>
#include <vector>

namespace
{
    std::string lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
        return value;
    }

    bool containsAny(const std::string& text, const std::vector<std::string>& values)
    {
        for (const std::string& value : values) if (text.find(value) != std::string::npos) return true;
        return false;
    }

    bool isMindless(const Monster& monster)
    {
        const std::string text = lower(monster.getName() + " " + monster.getType());
        if (monster.getRace() == Race::Construction || monster.getRace() == Race::MortVivant)
        {
            return !containsAny(text, {"roi", "chevalier", "oracle", "seigneur", "mage", "conscient", "parlant"});
        }
        return containsAny(text, {"zombie", "squelette", "ossement", "cadavre", "carcasse", "tourelle", "automate sans âme"});
    }

    bool raceCanSpeak(Race race)
    {
        switch (race)
        {
            case Race::Humain: case Race::SemiHumain: case Race::Elfe: case Race::ElfeNoir:
            case Race::Nain: case Race::Gnome: case Race::Halfelin: case Race::Tieffelin:
            case Race::Aasimar: case Race::Kitsune: case Race::Fee: case Race::SemiDragon:
            case Race::Gobelin: case Race::Hobgobelin: case Race::Orc: case Race::Demon:
            case Race::Ange: case Race::Dragon: case Race::Draconide: case Race::Esprit:
            case Race::Aberration: case Race::AnomalieArcanique:
                return true;
            default:
                return false;
        }
    }

    const Monster* chooseSpeaker(const EnemyCombatQueue& wave, Random& random)
    {
        std::vector<const Monster*> special;
        std::vector<const Monster*> elite;
        std::vector<const Monster*> ordinary;
        auto add = [&](const Monster& monster) {
            if (!raceCanSpeak(monster.getRace()) || isMindless(monster)) return;
            if (SpecialCharacterDialogueCatalog::hasDialogueFor(monster.getName())) special.push_back(&monster);
            else if (monster.isElite() || monster.isEvolved()) elite.push_back(&monster);
            else ordinary.push_back(&monster);
        };
        for (int i = 0; i < wave.getActiveEnemyCount(); ++i) add(wave.getActiveEnemy(i));
        for (int i = 0; i < wave.getWaitingEnemyCount(); ++i) add(wave.getWaitingEnemy(i));

        auto pick = [&](const std::vector<const Monster*>& pool) -> const Monster* {
            if (pool.empty()) return nullptr;
            return pool[static_cast<std::size_t>(random.between(0, static_cast<int>(pool.size()) - 1))];
        };
        if (!special.empty()) return pick(special);
        if (!elite.empty() && random.between(1, 100) <= 72) return pick(elite);
        if (!ordinary.empty() && random.between(1, 100) <= 42) return pick(ordinary);
        return nullptr;
    }

    std::string foreignLine(const std::string& languageId, Random& random)
    {
        if (languageId == "gobelin") return random.between(1,2)==1 ? "« Grik tak narok ! Skree val ! »" : "« Rakka-til, nosh griba ! »";
        if (languageId == "orc") return random.between(1,2)==1 ? "« Urg drah kor. Mak'thar ! »" : "« Gor ash muk ! »";
        if (languageId == "infernal") return random.between(1,2)==1 ? "« Vel'khara noss tiren... »" : "« Shaal ven dorakh. »";
        if (languageId == "draconique") return random.between(1,2)==1 ? "« Tharun vek siira. »" : "« Kraav nor elthar. »";
        if (languageId == "elfique") return random.between(1,2)==1 ? "« Leth aen silva, mori. »" : "« Elyn thar vae. »";
        if (languageId == "elfique_noir") return random.between(1,2)==1 ? "« Vhaer neth il'raen. »" : "« Ssil ven drae. »";
        if (languageId == "feerique") return random.between(1,2)==1 ? "« Aeli ril, thim vael. »" : "« Nimae, nimae... sor. »";
        if (languageId == "kitsune") return random.between(1,2)==1 ? "« Hoshi no kage, yoru wa miru. »" : "« Kitsu rei, michi wa uso. »";
        if (languageId == "celeste") return random.between(1,2)==1 ? "« Aurel na venia. »" : "« Lum aster, solenne. »";
        if (languageId == "nain") return random.between(1,2)==1 ? "« Khar dum, barak ven ! »" : "« Dorn akh, sten var. »";
        if (languageId == "gnome") return random.between(1,2)==1 ? "« Tik-ven, rota glim ! »" : "« Mekka tri, zinn ! »";
        if (languageId == "halfelin") return random.between(1,2)==1 ? "« Brindle hay, mora fen. »" : "« Tella moss, rin. »";
        if (languageId == "vampirique") return random.between(1,2)==1 ? "« Vespera sanguis, nael. »" : "« Mor ven, vitae thren. »";
        if (languageId == "spirituel") return random.between(1,2)==1 ? "« [souvenir : pluie] [nom effacé] [reste] »" : "« [tu étais ici avant d'arriver] »";
        if (languageId == "anormal") return random.between(1,2)==1 ? "« // voix non conforme // sujet observé // »" : "« La phrase se plie avant d'atteindre tes oreilles. »";
        return "« ... »";
    }

    std::string translatedLine(const Monster& monster, Random& random)
    {
        const std::string text = lower(monster.getName() + " " + monster.getType());
        if (containsAny(text, {"shaman", "chamane", "oracle"}))
            return random.between(1,2)==1 ? "« Restez près de moi. Les blessures passent avant la gloire. »" : "« Ne rompez pas la ligne, je peux encore vous tenir debout. »";
        if (containsAny(text, {"archer", "frondeur", "tireur"}))
            return random.between(1,2)==1 ? "« Un pas de côté. Je préfère viser ceux qui croient être à couvert. »" : "« Bouge encore. J'ai besoin de savoir où mettre le prochain trait. »";
        if (containsAny(text, {"voleur", "bandit", "brigand", "pillard", "taxeur"}))
            return random.between(1,2)==1 ? "« Pose ta bourse et tu garderas peut-être le reste. »" : "« On peut appeler ça un péage si ça t'aide à moins pleurer. »";
        if (containsAny(text, {"garde", "chevalier", "capitaine"}))
            return random.between(1,2)==1 ? "« Tu franchis cette ligne, tu assumes ce qu'il y a derrière. »" : "« Formation serrée. On ne lui donne aucun passage propre. »";
        if (monster.getRace() == Race::Gobelin || monster.getRace() == Race::Hobgobelin)
            return random.between(1,3)==1 ? "« Tu as l'air d'avoir une bourse et peu d'amis. Mauvaise combinaison. »" : (random.between(1,2)==1 ? "« On prend les vivants, les sacs, puis on discute du reste. »" : "« Pas besoin d'être grand pour compter ton or mieux que toi. »");
        if (monster.getRace() == Race::Orc)
            return random.between(1,2)==1 ? "« Tiens ta ligne. Si tu recules, je le verrai. »" : "« Un bon combat vaut mieux qu'une longue excuse. »";
        if (monster.getRace() == Race::Demon || monster.getRace() == Race::Tieffelin)
            return random.between(1,2)==1 ? "« Ta peur fait plus de bruit que ton arme. »" : "« Je ne promets rien. C'est déjà plus franc que la plupart des contrats. »";
        if (monster.getRace() == Race::Dragon || monster.getRace() == Race::Draconide || monster.getRace() == Race::SemiDragon)
            return random.between(1,2)==1 ? "« Je respecte le courage. Je punis l'arrogance. »" : "« Chaque pas de plus sera gravé dans tes os. »";
        if (monster.getRace() == Race::Elfe || monster.getRace() == Race::ElfeNoir || monster.getRace() == Race::Fee || monster.getRace() == Race::Kitsune)
            return random.between(1,2)==1 ? "« La route t'a laissé entrer. Elle ne t'a pas promis la sortie. »" : "« Joli pas. Mauvais silence. »";
        if (monster.getRace() == Race::Ange || monster.getRace() == Race::Aasimar)
            return random.between(1,2)==1 ? "« La lumière n'excuse pas tout. Elle révèle surtout ce que tu fais maintenant. »" : "« Même la grâce garde une lame pour les intrus. »";
        if (monster.getRace() == Race::Esprit || monster.getRace() == Race::Aberration || monster.getRace() == Race::AnomalieArcanique)
            return random.between(1,2)==1 ? "« Ton nom tremble dans la marge du monde. »" : "« Je parle depuis un endroit où tes règles arrivent en retard. »";
        return random.between(1,2)==1 ? "« Pas un pas de plus. Les problèmes commencent toujours comme ça. »" : "« Je ne te connais pas. Ça rendra ce combat plus simple. »";
    }

    std::string postureLine(const Monster& monster)
    {
        const MonsterBehaviorProfile profile = MonsterBehaviorProfileCatalog::build(monster);
        std::string line = monster.getName() + " prend l'initiative de parler";
        if (!profile.archetype.empty()) line += " avec la retenue d'un profil " + profile.archetype;
        line += ".";
        if (monster.isElite()) line += " Les autres lui laissent naturellement de l'espace.";
        if (monster.isEvolved()) line += " Quelque chose dans sa voix paraît plus net, comme si son évolution avait aussi renforcé sa volonté.";
        return line;
    }
}

bool EncounterDialogueSystem::display(
    const Player& player,
    const EnemyCombatQueue& wave,
    Random& random,
    const std::string& screenIdPrefix
)
{
    const Monster* speaker = chooseSpeaker(wave, random);
    if (speaker == nullptr) return false;

    const std::string languageId = LanguageSystem::languageForMonsterRace(speaker->getRace());
    const int knowledge = LanguageSystem::getKnowledgeLevel(player, languageId);
    const std::string foreign = foreignLine(languageId, random);
    const std::string translated = translatedLine(*speaker, random);

    std::vector<std::string> lines;
    lines.push_back("Interlocuteur : " + speaker->getName());
    lines.push_back("Langue : " + LanguageSystem::displayName(languageId));
    lines.push_back("Compréhension : " + LanguageSystem::knowledgeLabel(knowledge));
    lines.push_back("");
    lines.push_back(postureLine(*speaker));
    lines.push_back(LanguageSystem::renderSpeechForKnowledge(player, languageId, foreign, translated));

    if (languageId != "commun" && knowledge == 0)
        lines.push_back("Les mots t'échappent. Le ton, les gestes et la situation restent les seuls indices fiables.");
    else if (knowledge == 1)
        lines.push_back("Tu récupères des fragments utiles, mais une demande complexe ou une nuance peut encore t'échapper.");
    else
        lines.push_back("Tu comprends la phrase sans devoir deviner son sens général.");

    if (SpecialCharacterDialogueCatalog::hasDialogueFor(speaker->getName()))
        lines.push_back("Cette personne a une identité assez forte pour réagir différemment d'une rencontre ordinaire.");

    MessageScreen::show("DIALOGUE D'INTRODUCTION", screenIdPrefix + ".enemy_dialogue", lines, false);
    return true;
}
