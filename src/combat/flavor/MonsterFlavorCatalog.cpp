#include "combat/flavor/MonsterFlavorCatalog.hpp"
#include "entity/Monster.hpp"
#include <algorithm>
#include <cctype>
#include <functional>
#include <vector>

namespace
{
    std::string lower(std::string value)
    {
        std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c){ return static_cast<char>(std::tolower(c)); });
        return value;
    }
    bool has(const std::string& text, const std::vector<std::string>& tokens)
    {
        for (const auto& token : tokens) if (text.find(token) != std::string::npos) return true;
        return false;
    }
    std::size_t variant(const Monster& monster, std::size_t count, int salt)
    {
        const std::string key = monster.getName() + "|" + monster.getType() + "|" + std::to_string(monster.getLevel()) + "|" + std::to_string(salt);
        return count == 0 ? 0 : std::hash<std::string>{}(key) % count;
    }
    std::string choose(const Monster& monster, const std::vector<std::string>& options, int salt)
    {
        return options.empty() ? "" : options[variant(monster, options.size(), salt)];
    }
}

std::string MonsterFlavorCatalog::buildAppearanceLine(const Monster& monster)
{
    const std::string text = lower(monster.getName() + " " + monster.getType());
    std::string line;
    if (monster.getRace() == Race::Slime)
        line = choose(monster, {"sa membrane tremble en couches irrégulières", "des bulles lentes remontent sous sa surface", "son corps se reforme sans jamais garder exactement la même silhouette"}, 1);
    else if (monster.getRace() == Race::Insectoide)
        line = choose(monster, {"ses segments bougent avec une précision sèche", "ses pattes testent le sol avant le reste du corps", "ses pièces de carapace frottent comme de petites lames"}, 1);
    else if (monster.getRace() == Race::Plante)
        line = choose(monster, {"ses fibres se contractent comme des muscles verts", "des racines fines cherchent déjà les fissures du terrain", "ses feuilles ne suivent pas le vent : elles suivent les mouvements"}, 1);
    else if (monster.getRace() == Race::MortVivant)
        line = choose(monster, {"ses gestes ont un retard presque imperceptible sur son intention", "la poussière quitte ses articulations à chaque mouvement", "rien dans sa posture ne ressemble à un besoin de respirer"}, 1);
    else if (monster.getRace() == Race::Construction)
        line = choose(monster, {"ses pièces s'alignent avec un claquement mécanique", "des marques d'usure tracent les mêmes gestes répétés des centaines de fois", "son centre de masse se verrouille avant chaque déplacement"}, 1);
    else if (monster.getRace() == Race::Dragon || monster.getRace() == Race::Draconide || monster.getRace() == Race::SemiDragon)
        line = choose(monster, {"les écailles de son cou se soulèvent au rythme de sa respiration", "sa queue corrige silencieusement chaque déplacement", "la chaleur de son souffle précède légèrement ses paroles"}, 1);
    else if (monster.getRace() == Race::Esprit)
        line = choose(monster, {"ses contours arrivent une fraction de seconde après ses mouvements", "sa silhouette garde des détails qui ressemblent à des souvenirs", "la lumière traverse certaines parties de son corps sans logique stable"}, 1);
    else
        line = choose(monster, {"sa posture porte des habitudes que l'équipement seul n'explique pas", "ses yeux vérifient les issues avant de revenir sur toi", "les marques sur sa tenue montrent qu'il a déjà survécu à des combats mal propres"}, 1);

    if (has(text,{"borgne","balafr","fissur","caboss","brisé","brise","fendu"})) line += ", et une ancienne blessure change clairement sa façon de se tenir";
    if (monster.isElite()) line += "; l'assurance d'une élite se voit avant même le premier coup";
    if (monster.isEvolved()) line += "; son évolution a laissé des signes physiques qui ne ressemblent plus à la forme ordinaire de son espèce";
    return line + ".";
}

std::string MonsterFlavorCatalog::buildIdleLine(const Monster& monster)
{
    const std::string text = lower(monster.getName() + " " + monster.getType());
    if (has(text,{"archer","frondeur","tireur"})) return choose(monster,{"Il vérifie sans cesse l'espace entre lui et sa cible.","Son arme reste basse, mais son regard mesure déjà les trajectoires.","Il déplace un pied dès qu'une ligne de tir devient mauvaise."},2);
    if (has(text,{"bouclier","garde","gardien","chevalier"})) return choose(monster,{"Il présente d'abord la partie la plus solide de sa garde.","Il économise ses gestes et attend qu'une attaque vienne à lui.","Son poids reste centré : le pousser sera plus dur que le blesser."},2);
    if (has(text,{"shaman","chamane","oracle","mage"})) return choose(monster,{"Ses doigts répètent un motif comme s'il entretenait un sort déjà commencé.","Il surveille autant ses alliés que l'adversaire.","Son regard revient régulièrement vers les blessures et les zones chargées de magie."},2);
    if (monster.getRace()==Race::Bete) return choose(monster,{"Elle renifle les blessures, la peur et les changements d'appui.","Ses oreilles et son poids bougent avant ses pattes.","Elle ne fixe pas seulement la cible : elle surveille aussi l'endroit où elle pourrait fuir."},2);
    return choose(monster,{"Il ne reste jamais complètement immobile.","Un petit tic de posture trahit la manière dont il préfère attaquer.","Il teste la distance sans offrir encore un vrai engagement."},2);
}

std::string MonsterFlavorCatalog::buildAttackMotionLine(const Monster& monster)
{
    const std::string text = lower(monster.getName() + " " + monster.getType());
    if (monster.getRace()==Race::Slime) return choose(monster,{"la masse gélatineuse se tasse puis repart d'un seul bloc","une partie du corps s'étire avant de rappeler tout le reste","la surface se creuse puis projette le poids vers l'avant"},3);
    if (monster.getRace()==Race::Insectoide) return choose(monster,{"les pattes se replient puis claquent presque ensemble","le thorax pivote avant une détente sèche","la carapace se baisse et la pointe cherche une couture"},3);
    if (has(text,{"archer","frondeur","tireur"})) return choose(monster,{"le tir part après une correction minuscule du poignet","la corde claque au moment où la cible change d'appui","le projectile quitte l'arme depuis un angle volontairement sale"},3);
    if (has(text,{"hache","brute","berserk"})) return choose(monster,{"tout le poids du torse accompagne l'arme","le coup démarre large puis se resserre brutalement","l'épaule descend avant que l'arme ne remonte dans la trajectoire"},3);
    if (has(text,{"dague","couteau","assassin","voleur"})) return choose(monster,{"la main d'arme disparaît derrière le corps avant de revenir très court","le pas cherche l'extérieur de la garde plutôt que sa force","la lame reste près du corps jusqu'à la dernière fraction de seconde"},3);
    if (monster.getRace()==Race::Dragon || monster.getRace()==Race::Draconide || monster.getRace()==Race::SemiDragon) return choose(monster,{"les écailles du cou se contractent avant l'attaque","les griffes prennent le sol pendant que le torse accumule l'élan","la queue contrebalance le corps comme une seconde arme"},3);
    return choose(monster,{"le geste part sans trajectoire parfaitement scolaire","son attaque suit l'habitude de son espèce plus que celle d'un manuel","il transforme un petit déplacement en vraie tentative de rupture"},3);
}

std::string MonsterFlavorCatalog::buildImpactTextureLine(const Monster& monster, bool critical, bool boosted)
{
    std::string base;
    switch(monster.getRace())
    {
        case Race::Slime: base="L'impact est mou au premier contact puis étonnamment lourd quand toute la masse suit."; break;
        case Race::Construction: base="Le choc arrive sans hésitation musculaire, comme une pièce mécanique qui termine sa course."; break;
        case Race::Bete: base="Le choc est bref, animal et immédiatement suivi d'un repositionnement."; break;
        case Race::MortVivant: base="Le coup manque de prudence corporelle : rien ne semble protéger l'attaquant de son propre élan."; break;
        case Race::Elementaire: base="Le contact laisse une sensation qui dépasse la simple force physique."; break;
        default: base="Le contact porte la manière de combattre propre à cet adversaire, pas seulement une valeur de dégâts."; break;
    }
    if (critical) base += " Cette fois, l'angle tombe exactement là où la défense cède.";
    else if (boosted) base += " Une puissance inhabituelle pousse le mouvement au-delà de son rythme normal.";
    return base;
}


std::string MonsterFlavorCatalog::buildDeathLine(const Monster& monster)
{
    const std::string text = lower(monster.getName() + " " + monster.getType());
    if (monster.doesSplitOnDeath()) return monster.getName() + " ne meurt pas proprement : sa masse perd sa cohésion et prépare déjà une division.";
    if (monster.getRace()==Race::Slime) return choose(monster,{"La membrane s'affaisse autour du noyau et cesse enfin de se reformer.","La masse gélatineuse se vide de son mouvement et s'étale autour de son dernier point dense.","Les vibrations internes s'arrêtent les unes après les autres jusqu'à ne laisser qu'une gelée inerte."},21);
    if (monster.getRace()==Race::Construction) return choose(monster,{"Le mécanisme tente encore un cycle, se bloque, puis toutes les pièces retombent sous leur propre poids.","Une articulation continue de chercher son ordre précédent avant de s'immobiliser définitivement.","Le centre mécanique s'éteint ; le reste du corps devient soudain un simple assemblage de matière."},21);
    if (monster.getRace()==Race::MortVivant) return choose(monster,{"Ce qui animait le corps lâche prise sans respiration finale.","La posture s'effondre d'un coup, comme si une tension invisible venait d'être coupée.","Le cadavre redevient enfin un cadavre ; rien dans sa chute ne cherche à se protéger."},21);
    if (monster.getRace()==Race::Insectoide) return choose(monster,{"Les pattes se contractent vers le thorax avant de cesser de répondre.","La carapace heurte le sol en plusieurs temps tandis que les segments perdent leur coordination.","Les derniers mouvements deviennent mécaniques, sans plus former une attaque cohérente."},21);
    if (monster.getRace()==Race::Plante) return choose(monster,{"Les fibres se relâchent et les racines cessent de chercher de nouvelles prises.","Les feuilles tombent d'abord, puis la tension quitte les tiges comme une corde coupée.","La sève ralentit visiblement dans les blessures jusqu'à ne plus alimenter aucun mouvement."},21);
    if (monster.getRace()==Race::Esprit) return choose(monster,{"Les contours se décollent les uns des autres puis disparaissent sans laisser un corps complet.","La silhouette perd d'abord ses détails, puis son volume, comme un souvenir qu'on cesse de tenir.","La lumière traverse soudain toute la forme avant qu'elle ne se disperse."},21);
    if (monster.getRace()==Race::Dragon || monster.getRace()==Race::Draconide || monster.getRace()==Race::SemiDragon) return choose(monster,{"Les griffes labourent une dernière fois le sol avant que le poids des écailles n'emporte le corps.","Le souffle s'interrompt dans un grondement court et les plaques du cou retombent une à une.","La queue tente encore de corriger l'équilibre, puis le corps draconique cesse de répondre."},21);
    if (has(text,{"bandit","garde","mercenaire","soldat","humain","orc","gobelin","kobold"})) return choose(monster,{"Il cherche encore un appui par réflexe avant que ses jambes ne cèdent.","La garde disparaît avant la conscience : l'arme descend, puis le corps suit.","Son dernier mouvement ressemble moins à une attaque qu'à une tentative de rester debout."},21);
    return choose(monster,{"La manière dont il tombe reste cohérente avec son corps et ses blessures, pas avec une animation générique.","Le dernier mouvement reprend une habitude déjà visible pendant le combat avant de s'interrompre.","La tension quitte progressivement la posture jusqu'à rendre toute nouvelle attaque impossible."},21);
}

std::string MonsterFlavorCatalog::buildFleeLine(const Monster& monster)
{
    const std::string text = lower(monster.getName() + " " + monster.getType());
    if (has(text,{"archer","tireur","frondeur"})) return choose(monster,{"Il recule en gardant son arme vers toi, utilisant chaque pas pour conserver une ligne de tir possible.","La retraite se fait par angles courts : il refuse de tourner complètement le dos tant qu'il reste à portée.","Il abandonne du terrain avant d'abandonner sa garde, puis disparaît dès qu'un couvert coupe la ligne de vue."},22);
    if (has(text,{"bandit","voleur","assassin","kobold","gobelin"})) return choose(monster,{"Il jette un dernier regard aux issues repérées plus tôt et prend la moins mauvaise.","La fuite n'est pas noble : il profite du premier obstacle pour casser la poursuite.","Il sacrifie sa position, pas sa survie, et disparaît par l'angle qu'il surveillait depuis le début."},22);
    if (monster.getRace()==Race::Bete) return choose(monster,{"L'animal rompt le contact d'un bond, puis garde assez de distance pour sentir si tu le poursuis.","La peur gagne les appuis avant le reste du corps ; il fuit sans chercher une sortie humaine.","Il cesse de menacer, tourne par instinct et cherche immédiatement le terrain où sa morphologie l'avantage."},22);
    if (monster.getRace()==Race::Dragon || monster.getRace()==Race::Draconide || monster.getRace()==Race::SemiDragon) return choose(monster,{"La retraite reste lourde et contrôlée : il protège son flanc blessé avant de gagner de la distance.","Il quitte la mêlée en utilisant queue, ailes ou masse comme écran plutôt qu'en courant droit.","Même en fuite, la posture draconique refuse d'offrir complètement le dos."},22);
    return choose(monster,{"Sa fuite reprend les mêmes habitudes de mouvement que son combat, mais toute l'énergie sert maintenant à survivre.","Il rompt la distance dès qu'il comprend qu'aucun échange supplémentaire ne lui est favorable.","La peur ne le téléporte pas : il cherche réellement une sortie et y engage tout son mouvement."},22);
}

std::string MonsterFlavorCatalog::buildSurrenderLine(const Monster& monster)
{
    const std::string text = lower(monster.getName() + " " + monster.getType());
    if (has(text,{"garde","soldat","mercenaire","chevalier"})) return choose(monster,{"L'arme est posée assez loin pour ne plus être une menace, mais la posture reste militaire.","Il ouvre la main d'arme, recule d'un pas et attend clairement que la violence cesse.","La reddition est propre : genou au sol, arme abandonnée, regard encore fixé sur les issues."},23);
    if (has(text,{"bandit","voleur","gobelin","kobold"})) return choose(monster,{"Il lâche ce qu'il tient plus vite qu'il ne l'aurait admis quelques secondes plus tôt.","Les mains montent, le corps reste prêt à bondir si quelqu'un transforme la reddition en exécution.","Il se rend sans dignité particulière, surtout avec l'attention très vive de quelqu'un qui veut rester vivant."},23);
    if (monster.getRace()==Race::Orc) return choose(monster,{"Il plante ou jette son arme au sol et expose volontairement ses mains vides.","La mâchoire reste serrée, mais la posture de combat disparaît : il reconnaît la défaite sans s'effondrer.","Il cesse d'avancer, écarte son arme et attend la réponse avec une hostilité désormais contenue."},23);
    return choose(monster,{"La menace quitte sa posture avant que la peur ne quitte son visage.","Il montre clairement qu'il ne cherche plus à combattre, sans devenir pour autant un allié ou un cadavre.","L'arme baisse, les appuis se ferment et tout son langage corporel demande l'arrêt du combat."},23);
}

std::string MonsterFlavorCatalog::buildBestiaryFlavor(const Monster& monster)
{
    return " Aspect observé : " + buildAppearanceLine(monster) + " Habitude visible : " + buildIdleLine(monster);
}
