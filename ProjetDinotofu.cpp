#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <random>
#include <math.h>
using namespace std;

random_device rd;
mt19937 gen(rd());

uniform_int_distribution<> alTour(1, 2);     // alTour(gen)
uniform_int_distribution<> alOptions(0, 5);  // alOptions(gen)
uniform_int_distribution<> alDesDe20(1, 20); // alDesDe20(gen)
uniform_int_distribution<> alClassj2(1, 3);  // alClassj2(gen)
// 3 autres unitform on ete place avant chaque combats
// alDegat1(gen)
// alDegat2(gen)
// alTestIA(gen)

// #########################################################################################################################

void option0j1(string joueur1, int pv1, int pv1max, int dgt1max, int dgt1min, int dgt1crit, int poH1, int poD1, string choi1)
{
    cout << joueur1 << endl
         << "Classe : " << choi1 << endl
         << pv1 << "/" << pv1max << " Pv restants" << endl
         << dgt1max << " Dégats max" << endl
         << dgt1min << " Dégats min" << endl
         << dgt1crit << " Dégats crit" << endl
         << poH1 << " Potions de soin restantes" << endl
         << poD1 << " Potion de dégats restantes" << endl
         << endl;
}

// #########################################################################################################################

void option0j2(string joueur2, int pv2, int pv2max, int dgt2max, int dgt2min, int dgt2crit, int poH2, int poD2, string choi2)
{
    cout << joueur2 << endl
         << "Classe : " << choi2 << endl
         << pv2 << "/" << pv2max << " Pv restants" << endl
         << dgt2max << " Dégats max" << endl
         << dgt2min << " Dégats min" << endl
         << dgt2crit << " Dégats crit" << endl
         << poH2 << " Potions de soin restantes" << endl
         << poD2 << " Potion de dégats restantes" << endl
         << endl;
}

// #########################################################################################################################

void choixJ2()
{
    cout << "1: chevalier : " << endl
         << "pv =200" << endl
         << "Dégats 5 - 20;  crit 30" << endl
         << "4 potions de soin" << endl
         << "3 potions de dégats" << endl
         << endl;
    sleep(1);
    cout << "2: léger : " << endl
         << "pv =150" << endl
         << "Dégats 10 - 30;  crit 45" << endl
         << "5 potions de soin" << endl
         << "2 potions de dégats" << endl
         << endl;
    sleep(1);
    cout << "3: lourd : " << endl
         << "pv =400" << endl
         << "Dégats 2 - 8;  crit 15" << endl
         << "2 potions de soin" << endl
         << "5 potions de dégats" << endl
         << endl;
    cout << "Veuillez entrer uniquement le chiffre correspondant :" << endl;
    sleep(1);
}

// #########################################################################################################################
// #########################################################################################################################
// #########################################################################################################################
// #########################################################################################################################

void deuxjoueur(string joueur1, int pv1, int pv1max, int degats1, int dgt1max, int dgt1min, int dgt1crit, int poH1, int poD1, string choi1)
{
    cout << endl
         << endl
         << "A votre tour Joueur 2, quel est ton nom ?" << endl;
    string joueur2;
    cin >> joueur2;
    cout << endl;

    cout << "Tres bien " << joueur2 << endl;
    sleep(2);
    cout << "Choisis ta classe et entre dans l'arène parmi ces 3 classes: " << endl
         << endl;
    sleep(2);
    choixJ2();

    int pv2;
    int pv2max;
    int degats2;
    int dgt2max;
    int dgt2min;
    int dgt2crit;
    int poH2;
    int poD2;
    int classj2;
    string choi2;
    while (true)
    {
        cin >> classj2;
        if (classj2 >= 1 && classj2 <= 3)
            break;
        else
            cout << "Veuillez entrer un chiffre valide (1, 2 ou 3) :" << endl;
    }

    if (classj2 == 1)
    {
        pv2 = 200;
        pv2max = 200;
        dgt2max = 20;
        dgt2min = 5;
        dgt2crit = 30;
        poH2 = 4;
        poD2 = 3;
        choi2 = "Chevalier";
    }
    else if (classj2 == 2)
    {
        pv2 = 150;
        pv2max = 150;
        dgt2max = 30;
        dgt2min = 10;
        dgt2crit = 45;
        poH2 = 5;
        poD2 = 2;
        choi2 = "Classe Légère";
    }
    else if (classj2 == 3)
    {
        pv2 = 400;
        pv2max = 400;
        dgt2max = 12;
        dgt2min = 5;
        dgt2crit = 20;
        poH2 = 2;
        poD2 = 5;
        choi2 = "Classe Lourde";
    }

    system("clear");
    cout << joueur2 << ", vous avez choisis la classe: " << choi2 << "; et avez acqueris avec succès ses statistiques." << endl;
    sleep(5);

    // #########################################################################################################################

    uniform_int_distribution<> alDegat1(dgt1min, dgt1max);
    uniform_int_distribution<> alDegat2(dgt2min, dgt2max);

    int options;
    int tour = 0;
    int desDe20;
    cout << endl
         << "Preparez vous, le combat va bientot commencer..." << endl;
    sleep(5);
    system("clear");
    tour = alTour(gen);
    while (true)
    {
        if (pv1 <= 0)
        {
            sleep(5);
            system("clear");
            cout << joueur1 << " a été mis a terre..." << endl
                 << joueur2 << " remporte donc la partie ainsi que mon respect." << endl
                 << endl;
            break;
        }
        else if (pv2 <= 0)
        {
            sleep(5);
            system("clear");
            cout << joueur2 << " a été mis a terre..." << endl
                 << joueur1 << " remporte donc la partie ainsi que mon respect." << endl
                 << endl;
            break;
        }

        else
        {
            sleep(2);
            if (tour == 1)
            {
                cout << endl
                     << endl
                     << endl
                     << endl
                     << endl
                     << endl;
                cout << "Tour de " << joueur1 << endl
                     << endl;
                cout << "Choisis une option parmi :" << endl
                     << "0: Stats                 1: Attack" << endl
                     << "2: Potion de soin        3: Potion de dégat" << endl
                     << "4: Manuel de potions     5: Passer son tour" << endl
                     << endl
                     << ":";

                cin >> options;
                system("clear");
                if (options == 0)
                {
                    option0j1(joueur1, pv1, pv1max, dgt1max, dgt1min, dgt1crit, poH1, poD1, choi1);
                }
                else if (options == 1)
                {
                    desDe20 = alDesDe20(gen);
                    if (desDe20 <= 3)
                    {
                        degats1 = 0;
                        cout << "Vous avez attaqué mais l'énnemi a réussit a esquiver." << endl
                             << endl;
                        tour = 2;
                    }
                    else if (desDe20 <= 16)
                    {
                        degats1 = alDegat1(gen);
                        cout << "Vous avez attaqué et infligé: " << degats1 << " points de dégats." << endl
                             << endl;
                        pv2 = pv2 - degats1;
                        tour = 2;
                    }
                    else if (desDe20 <= 20)
                    {
                        degats1 = dgt1crit;
                        cout << "Vous avez attaqué et infligé de lourds dégats: " << degats1 << endl
                             << endl;
                        pv2 = pv2 - degats1;
                        tour = 2;
                    }
                }
                else if (options == 2)
                {
                    if (poH1 > 0)
                    {
                        pv1 = pv1 + 55;
                        poH1 = poH1 - 1;
                        if (pv1 > pv1max)
                        {
                            pv1 = pv1max;
                        }
                        cout << "Vous sentez vos blessures se refermer et voyez votre vitalité augmenter" << endl
                             << endl;
                        tour = 2;
                    }
                    else
                    {
                        cout << "Vous n'avez plus de potions en votre possession, vous pouvez toujours tenter autre chose:" << endl
                             << endl;
                    }
                }
                else if (options == 3)
                {
                    if (poD1 > 0)
                    {
                        cout << "vous sentez vos forces monter en puissance et une rage s'empare de vous..." << endl;
                        sleep(2);
                        desDe20 = alDesDe20(gen);
                        if (desDe20 <= 3)
                        {
                            degats1 = 0;
                            cout << "Vous avez attaqué mais votre puissance vous a paralisé pendant un court instant." << endl
                                 << endl;
                            poD1 = poD1 - 1;
                            tour = 2;
                        }
                        else if (desDe20 <= 16)
                        {
                            degats1 = alDegat1(gen);
                            degats1 = degats1 + 25;
                            poD1 = poD1 - 1;
                            cout << "Vous avez attaqué et infliger: " << degats1 << " points de dégats avec une puissance dévastatrice." << endl
                                 << endl;
                            pv2 = pv2 - degats1;
                            tour = 2;
                        }
                        else if (desDe20 <= 20)
                        {
                            degats1 = dgt1crit + 25;
                            poD1 = poD1 - 1;
                            cout << "Votre rage étais si forte que vous avez attaqué et infligé des dégats monstrueux: " << degats1 << endl
                                 << endl;
                            pv2 = pv2 - degats1;
                            tour = 2;
                        }
                    }
                    else
                    {
                        cout << "Votre rage est épuisé, vous pouvez toujours tenter autre chose:" << endl
                             << endl;
                    }
                }
                else if (options == 4)
                {
                    cout << "Vous avez demandé a un expert de vous expliquer le contenue de votre équipement:" << endl;
                    sleep(1);
                    cout << "Vos potions de soin rapportent 55 mais passent votre tour;" << endl;
                    cout << "Vos potions de dégats augmente de 25 votre puissance, forcent à attaquer et se perdent même si l'attaque échoue." << endl
                         << endl;
                }
                else if (options == 5)
                {
                    cout << "Vous avez oublié quoi faire et avez mis fin a votre tour..." << endl
                         << endl;
                    tour = 2;
                }
                else
                {
                    sleep(1);
                    cout << "Option invalide, veulliez entrer une option valide, le temps n'est pas compté." << endl
                         << endl;
                    sleep(1);
                }
            }

            // #########################################################################################################################

            else if (tour == 2)
            {
                cout << endl
                     << endl
                     << endl
                     << endl
                     << endl
                     << endl;
                cout << "Tour de " << joueur2 << endl
                     << endl;
                cout << "Choisis une option parmi :" << endl
                     << "0: Stats                 1: Attack" << endl
                     << "2: Potion de soin        3: Potion de dégat" << endl
                     << "4: Manuel de potions     5: Passer son tour" << endl
                     << endl
                     << ":";

                cin >> options;
                system("clear");
                if (options == 0)
                {
                    option0j2(joueur2, pv2, pv2max, dgt2max, dgt2min, dgt2crit, poH2, poD2, choi2);
                }
                else if (options == 1)
                {
                    desDe20 = alDesDe20(gen);
                    if (desDe20 <= 3)
                    {
                        degats2 = 0;
                        cout << "Vous avez attaqué mais l'énnemi a réussit a esquiver." << endl
                             << endl;
                        tour = 1;
                    }
                    else if (desDe20 <= 16)
                    {
                        degats2 = alDegat2(gen);
                        cout << "Vous avez attaqué et infligé: " << degats2 << " points de dégats." << endl
                             << endl;
                        pv1 = pv1 - degats2;
                        tour = 1;
                    }
                    else if (desDe20 <= 20)
                    {
                        degats2 = dgt2crit;
                        cout << "Vous avez attaqué et infligé de lourds dégats: " << degats2 << endl
                             << endl;
                        pv1 = pv1 - degats2;
                        tour = 1;
                    }
                }
                else if (options == 2)
                {
                    if (poH2 > 0)
                    {
                        pv2 = pv2 + 55;
                        poH2 = poH2 - 1;
                        if (pv2 > pv2max)
                        {
                            pv2 = pv2max;
                        }
                        cout << "Vous sentez vos blessures se refermer et voyez votre vitalité augmenter" << endl
                             << endl;
                        tour = 1;
                    }
                    else
                    {
                        cout << "Vous n'avez plus de potions en votre possession, vous pouvez toujours tenter autre chose:" << endl
                             << endl;
                    }
                }
                else if (options == 3)
                {
                    if (poD2 > 0)
                    {
                        cout << "vous sentez vos forces monter en puissance et une rage s'empare de vous..." << endl;
                        sleep(2);
                        desDe20 = alDesDe20(gen);
                        if (desDe20 <= 3)
                        {
                            degats2 = 0;
                            cout << "Vous avez attaqué mais votre puissance vous a paralisé pendant un court instant." << endl
                                 << endl;
                            poD2 = poD2 - 1;
                            tour = 1;
                        }
                        else if (desDe20 <= 16)
                        {
                            degats2 = alDegat2(gen);
                            degats2 = degats2 + 25;
                            poD2 = poD2 - 1;
                            cout << "Vous avez attaqué et infliger: " << degats2 << " points de dégats avec une puissance dévastatrice." << endl
                                 << endl;
                            pv1 = pv1 - degats2;
                            tour = 1;
                        }
                        else if (desDe20 <= 20)
                        {
                            degats2 = dgt2crit + 25;
                            poD2 = poD2 - 1;
                            cout << "Votre rage étais si forte que vous avez attaqué et infligé des dégats monstrueux: " << degats2 << endl
                                 << endl;
                            pv1 = pv1 - degats2;
                            tour = 1;
                        }
                    }
                    else
                    {
                        cout << "Votre rage est épuisé, vous pouvez toujours tenter autre chose:" << endl
                             << endl;
                    }
                }
                else if (options == 4)
                {
                    cout << "Vous avez demandé a un expert de vous expliquer le contenue de votre équipement:" << endl;
                    sleep(1);
                    cout << "Vos potions de soin rapportent 55 mais passent votre tour;" << endl;
                    cout << "Vos potions de dégats augmente de 25 votre puissance, forcent à attaquer et se perdent même si l'attaque échoue." << endl
                         << endl;
                }
                else if (options == 5)
                {
                    cout << "Vous avez oublié quoi faire et avez mis fin a votre tour..." << endl
                         << endl;
                    tour = 1;
                }
                else
                {
                    sleep(1);
                    cout << "Option invalide, veulliez entrer une option valide, le temps n'est pas compté." << endl
                         << endl;
                    sleep(1);
                }
            }
        }
    }
}

// #####################################################################################################################################################
// #####################################################################################################################################################
// #####################################################################################################################################################

void pvpIA(string joueur1, int pv1, int pv1max, int degats1, int dgt1max, int dgt1min, int dgt1crit, int poH1, int poD1, string choi1)
{
    cout << endl
         << endl
         << "Preparation de l'IA..." << endl
         << endl;
    sleep(1);
    string joueur2;
    joueur2 = "Matt";

    cout << joueur2 << " est entré dans l'arène" << endl
         << endl;
    sleep(2);

    int classj2;
    cout << joueur1 << " choisis le type d'option voullu pour le choix de la class de l'ennemi: 1 aleatoire, 2 choisis :" << endl;
    int testIA = 0;
    while (true)
    {
        cin >> testIA;
        system("clear");
        if (testIA >= 1 && testIA <= 2)
            break;
        else
            cout << "Veuillez entrer un chiffre valide (1 ou 2) :" << endl;
    }
    if (testIA == 2)
    {
        cout << endl
             << "Choisis la classe que Matt choisiras: " << endl
             << endl;
        sleep(2);
        choixJ2();
        while (true)
        {
            cin >> classj2;
            if (classj2 >= 1 && classj2 <= 3)
                break;
            else
                cout << "Veuillez entrer un chiffre valide (1, 2 ou 3) :" << endl;
        }
    }
    else
    {
        classj2 = alClassj2(gen);
    }
    sleep(1);
    int pv2;
    int pv2max;
    int degats2;
    int dgt2max;
    int dgt2min;
    int dgt2crit;
    int poH2;
    int poD2;
    string choi2;

    cout << endl;

    if (classj2 == 1)
    {
        pv2 = 200;
        pv2max = 200;
        dgt2max = 20;
        dgt2min = 5;
        dgt2crit = 30;
        poH2 = 4;
        poD2 = 3;
        choi2 = "Chevalier";
    }
    else if (classj2 == 2)
    {
        pv2 = 150;
        pv2max = 150;
        dgt2max = 30;
        dgt2min = 10;
        dgt2crit = 45;
        poH2 = 5;
        poD2 = 2;
        choi2 = "Classe Légère";
    }
    else if (classj2 == 3)
    {
        pv2 = 400;
        pv2max = 400;
        dgt2max = 12;
        dgt2min = 5;
        dgt2crit = 20;
        poH2 = 2;
        poD2 = 5;
        choi2 = "Classe Lourde";
    }

    system("clear");
    cout << joueur2 << ", a choisis la classe: " << choi2 << "; et a acqueris avec succès ses statistiques." << endl;
    sleep(5);

    // #########################################################################################################################

    uniform_int_distribution<> alDegat1(dgt1min, dgt1max);
    uniform_int_distribution<> alDegat2(dgt2min, dgt2max);

    int options;
    int tour = 0;
    int desDe20;
    cout << endl
         << "Preparez vous, le combat va bientot commencer..." << endl;
    sleep(5);
    system("clear");
    tour = alTour(gen);
    while (true)
    {
        if (pv1 <= 0)
        {
            sleep(5);
            system("clear");
            cout << joueur1 << " a été mis a terre..." << endl
                 << joueur2 << " remporte donc la partie ainsi que mon respect." << endl
                 << endl;
            break;
        }
        else if (pv2 <= 0)
        {
            sleep(5);
            system("clear");
            cout << joueur2 << " a été mis a terre..." << endl
                 << joueur1 << " remporte donc la partie ainsi que mon respect." << endl
                 << endl;
            break;
        }

        else
        {
            sleep(2);
            if (tour == 1)
            {
                cout << endl
                     << endl
                     << endl
                     << endl
                     << endl
                     << endl;
                cout << "Tour de " << joueur1 << endl
                     << endl;
                cout << "Choisis une option parmi :" << endl
                     << "0: Stats                 1: Attack" << endl
                     << "2: Potion de soin        3: Potion de dégat" << endl
                     << "4: Manuel de potions     5: Passer son tour" << endl
                     << endl
                     << ":";

                cin >> options;
                system("clear");
                if (options == 0)
                {
                    option0j1(joueur1, pv1, pv1max, dgt1max, dgt1min, dgt1crit, poH1, poD1, choi1);
                }
                else if (options == 1)
                {
                    desDe20 = alDesDe20(gen);
                    if (desDe20 <= 3)
                    {
                        degats1 = 0;
                        cout << "Vous avez attaqué mais l'énnemi a réussit a esquiver." << endl
                             << endl;
                        tour = 2;
                    }
                    else if (desDe20 <= 16)
                    {
                        degats1 = alDegat1(gen);
                        cout << "Vous avez attaqué et infligé: " << degats1 << " points de dégats." << endl
                             << endl;
                        pv2 = pv2 - degats1;
                        tour = 2;
                    }
                    else if (desDe20 <= 20)
                    {
                        degats1 = dgt1crit;
                        cout << "Vous avez attaqué et infligé de lourds dégats: " << degats1 << endl
                             << endl;
                        pv2 = pv2 - degats1;
                        tour = 2;
                    }
                }
                else if (options == 2)
                {
                    if (poH1 > 0)
                    {
                        pv1 = pv1 + 55;
                        poH1 = poH1 - 1;
                        if (pv1 > pv1max)
                        {
                            pv1 = pv1max;
                        }
                        cout << "Vous sentez vos blessures se refermer et voyez votre vitalité augmenter" << endl
                             << endl;
                        tour = 2;
                    }
                    else
                    {
                        cout << "Vous n'avez plus de potions en votre possession, vous pouvez toujours tenter autre chose:" << endl
                             << endl;
                    }
                }
                else if (options == 3)
                {
                    if (poD1 > 0)
                    {
                        cout << "vous sentez vos forces monter en puissance et une rage s'empare de vous..." << endl;
                        sleep(2);
                        desDe20 = alDesDe20(gen);
                        if (desDe20 <= 3)
                        {
                            degats1 = 0;
                            cout << "Vous avez attaqué mais votre puissance vous a paralisé pendant un court instant." << endl
                                 << endl;
                            poD1 = poD1 - 1;
                            tour = 2;
                        }
                        else if (desDe20 <= 16)
                        {
                            degats1 = alDegat1(gen);
                            degats1 = degats1 + 25;
                            poD1 = poD1 - 1;
                            cout << "Vous avez attaqué et infliger: " << degats1 << " points de dégats avec une puissance dévastatrice." << endl
                                 << endl;
                            pv2 = pv2 - degats1;
                            tour = 2;
                        }
                        else if (desDe20 <= 20)
                        {
                            degats1 = dgt1crit + 25;
                            poD1 = poD1 - 1;
                            cout << "Votre rage étais si forte que vous avez attaqué et infligé des dégats monstrueux: " << degats1 << endl
                                 << endl;
                            pv2 = pv2 - degats1;
                            tour = 2;
                        }
                    }
                    else
                    {
                        cout << "Votre rage est épuisé, vous pouvez toujours tenter autre chose:" << endl
                             << endl;
                    }
                }
                else if (options == 4)
                {
                    cout << "Vous avez demandé a un expert de vous expliquer le contenue de votre équipement:" << endl;
                    sleep(1);
                    cout << "Vos potions de soin rapportent 55 mais passent votre tour;" << endl;
                    cout << "Vos potions de dégats augmente de 25 votre puissance, forcent à attaquer et se perdent même si l'attaque échoue." << endl
                         << endl;
                }
                else if (options == 5)
                {
                    cout << "Vous avez oublié quoi faire et avez mis fin a votre tour..." << endl
                         << endl;
                    tour = 2;
                }
                else
                {

                    sleep(1);
                    cout << "Option invalide, veulliez entrer une option valide, le temps n'est pas compté." << endl
                         << endl;
                    sleep(1);
                }
            }

            // #########################################################################################################################

            else if (tour == 2)
            {
                cout << endl
                     << endl
                     << endl
                     << endl
                     << endl
                     << endl;
                cout << "Tour de " << joueur2 << endl
                     << endl;

                options = alOptions(gen);
                if (options == 5)
                {
                    options = alOptions(gen);
                }
                if (options == 2)
                {
                    options = 1;
                }
                else if (options == 4)
                {
                    options = 1;
                }
                else if (options == 3)
                {
                    if (poD2 == 0)
                    {
                        options = 1;
                    }
                }

                if (pv2 <= 60)
                {
                    if (poH2 > 0)
                    {
                        options = 2;
                    }
                }
                cout << options << endl;

                if (options == 0)
                {
                    option0j2(joueur2, pv2, pv2max, dgt2max, dgt2min, dgt2crit, poH2, poD2, choi2);
                }
                else if (options == 1)
                {
                    desDe20 = alDesDe20(gen);
                    if (desDe20 <= 3)
                    {
                        degats2 = 0;
                        cout << joueur2 << " a attaqué mais son énnemi a réussit a esquiver." << endl
                             << endl;
                        tour = 1;
                    }
                    else if (desDe20 <= 16)
                    {
                        degats2 = alDegat2(gen);
                        cout << joueur2 << " a attaqué et infligé: " << degats2 << " points de dégats." << endl
                             << endl;
                        pv1 = pv1 - degats2;
                        tour = 1;
                    }
                    else if (desDe20 <= 20)
                    {
                        degats2 = dgt2crit;
                        cout << joueur2 << " a attaqué et infligé de lourds dégats: " << degats2 << endl
                             << endl;
                        pv1 = pv1 - degats2;
                        tour = 1;
                    }
                }
                else if (options == 2)
                {
                    if (poH2 > 0)
                    {
                        pv2 = pv2 + 55;
                        poH2 = poH2 - 1;
                        if (pv2 > pv2max)
                        {
                            pv2 = pv2max;
                        }
                        cout << joueur2 << " sent ses blessures se refermer et vois sa vitalité augmenter" << endl
                             << endl;
                        tour = 1;
                    }
                }
                else if (options == 3)
                {
                    if (poD2 > 0)
                    {
                        cout << joueur2 << " sens ses forces monter en puissance et une rage s'empare de lui..." << endl;
                        sleep(2);
                        desDe20 = alDesDe20(gen);
                        if (desDe20 <= 3)
                        {
                            degats2 = 0;
                            cout << joueur2 << " a attaqué mais sa puissance l'a paralisé pendant un court instant." << endl
                                 << endl;
                            poD2 = poD2 - 1;
                            tour = 1;
                        }
                        else if (desDe20 <= 16)
                        {
                            degats2 = alDegat2(gen);
                            degats2 = degats2 + 25;
                            poD2 = poD2 - 1;
                            cout << joueur2 << " a attaqué et infliger: " << degats2 << " points de dégats avec une puissance dévastatrice." << endl
                                 << endl;
                            pv1 = pv1 - degats2;
                            tour = 1;
                        }
                        else if (desDe20 <= 20)
                        {
                            degats2 = dgt2crit + 25;
                            poD2 = poD2 - 1;
                            cout << "La rage de " << joueur2 << " étais si forte qu'elle a infligé des dégats monstrueux: " << degats2 << endl
                                 << endl;
                            pv1 = pv1 - degats2;
                            tour = 1;
                        }
                    }
                }
                else if (options == 5)
                {
                    cout << "Matt a oublié quoi faire et a mis fin a son tour..." << endl
                         << endl;
                    tour = 1;
                }
            }
        }
    }
}

// #####################################################################################################################################################
// #####################################################################################################################################################
// #####################################################################################################################################################

void pvBoss(string joueur1, int pv1, int pv1max, int degats1, int dgt1max, int dgt1min, int dgt1crit, int poH1, int poD1, int classj1, string choi1)
{
    cout << "Vous sentez une aura maléfique autour de vous." << endl;
    sleep(2);
    cout << "Par chance, votre ame est encore pur, et vous sentez vos forces evoluer a mesure que vous avancez en direction de cet aura." << endl
         << endl;
    sleep(2);

    if (classj1 == 1)
    {
        pv1 = 500;
        pv1max = 500;
        dgt1max = 20;
        dgt1min = 5;
        dgt1crit = 30;
        poH1 = 6;
        poD1 = 5;
        choi1 = "Paladin";
    }
    else if (classj1 == 2)
    {
        pv1 = 400;
        pv1max = 400;
        dgt1max = 30;
        dgt1min = 10;
        dgt1crit = 45;
        poH1 = 7;
        poD1 = 4;
        choi1 = "Assassin";
    }
    else if (classj1 == 3)
    {
        pv1 = 900;
        pv1max = 900;
        dgt1max = 12;
        dgt1min = 5;
        dgt1crit = 20;
        poH1 = 4;
        poD1 = 7;
        choi1 = "Colosse";
    }
    cout << "Votre classe evolue en classe " << choi1 << endl
         << "Evolution de pv réussit : passage a " << pv1max << "pv" << endl
         << "Evolution de Dégats échoué : blocage a " << dgt1min << " - " << dgt1max << ";  crit " << dgt1crit << endl
         << "Don d'objet réussit : passage a " << poH1 << " potions de soin" << endl
         << "Don d'objet réussit : passage a " << poD1 << " potions de dégâts" << endl
         << endl;
    sleep(4);
    cout << joueur1 << ", vous avez evolué en classe: " << choi1 << "; et avez acqueris avec succès ses statistiques." << endl
         << endl;
    sleep(1);
    int classj2;
    cout << joueur1 << " choisis le type d'option voullu pour le choix du boss : 1 aleatoire, 2 choisis :" << endl;
    int testIA = 0;
    while (true)
    {
        cin >> testIA;
        system("clear");
        if (testIA >= 1 && testIA <= 2)
            break;
        else
            cout << "Veuillez entrer un chiffre valide (1 ou 2) :" << endl;
    }
    if (testIA == 2)
    {
        cout << "selectionnez l'entité que vous voulez exterminer :" << endl;
        sleep(1);
        cout << "1: Ange     2: Demon     3: Protecteur universel déchu" << endl;
        cout << "Leurs statistiques resterons inconnue pour le moment, vous aurez donc besoin d'un bon instinct..." << endl;
        cout << "Veuillez entrer uniquement le chiffre correspondant :" << endl;
        while (true)
        {
            cin >> classj2;
            if (classj2 >= 1 && classj2 <= 3)
                break;
            else
                cout << "Veuillez entrer un chiffre valide (1, 2 ou 3) :" << endl;
        }
    }
    else
    {
        classj2 = alClassj2(gen);
    }

    sleep(1);
    int pv2;
    int pv2max;
    int degats2;
    int dgt2max;
    int dgt2min;
    int dgt2crit;
    int poH2;
    int poD2;
    int ulti2 = 0;
    int delai2 = 0;
    int ulti2max;
    int delai2max;
    string choi2;
    string joueur2;
    int cryptage = 1;
    int typej2 = 0;
    uniform_int_distribution<> alTypej2(1, 3);

    cout << endl;

    if (classj2 == 1)
    {
        pv2 = 800;
        pv2max = 800;
        dgt2max = 10;
        dgt2min = 3;
        dgt2crit = 15;
        poH2 = 3;
        poD2 = 5;
        choi2 = "Ange";
        joueur2 = "Fitoria";
        ulti2max = 3;
        delai2max = 14;
    }
    else if (classj2 == 2)
    {
        pv2 = 600;
        pv2max = 600;
        dgt2max = 15;
        dgt2min = 5;
        dgt2crit = 22;
        poH2 = 5;
        poD2 = 4;
        choi2 = "Demon";
        joueur2 = "Zelef";
        ulti2max = 4;
        delai2max = 10;
    }
    else if (classj2 == 3)
    {
        pv2 = 1200;
        pv2max = 1200;
        dgt2max = 5;
        dgt2min = 2;
        dgt2crit = 8;
        poH2 = 2;
        poD2 = 5;
        choi2 = "Protecteur universel déchu";
        joueur2 = "Atlas";
        ulti2max = 3;
        delai2max = 12;
    }

    delai2 = delai2max;
    ulti2 = 0;

    cout << endl
         << endl
         << "Preparation du Boss..." << endl
         << endl;
    sleep(3);
    cout << joueur2 << " est entré dans l'arène" << endl
         << endl;
    sleep(3);
    system("clear");
    cout << joueur2 << ", est de type " << choi2 << "; faites attention: ce combat pourrais vous être fatal..." << endl;
    sleep(5);

    // #########################################################################################################################

    uniform_int_distribution<> alDegat1(dgt1min, dgt1max);
    uniform_int_distribution<> alDegat2(dgt2min, dgt2max);

    int options;
    int tour = 0;
    int desDe20;
    cout << endl
         << "Preparez vous, le combat va bientot commencer..." << endl;
    sleep(5);
    system("clear");
    tour = alTour(gen);
    while (true)
    {
        if (pv1 <= 0)
        {
            sleep(5);
            system("clear");
            cout << joueur1 << " a été mis a terre..." << endl
                 << joueur2 << " as donc retrouver assez de force et de rage en vous battant pour exterminer l'humanité." << endl
                 << endl;
            break;
        }
        else if (pv2 <= 0)
        {
            sleep(5);
            system("clear");
            cout << joueur2 << " a été mis a terre..." << endl
                 << joueur1 << " remporte donc le combat et sauve le monde." << endl
                 << endl;
            break;
        }

        else
        {
            sleep(2);
            if (tour == 1)
            {
                cout << endl
                     << endl
                     << endl
                     << endl
                     << endl
                     << endl;
                cout << "Tour de " << joueur1 << endl
                     << endl;
                cout << "Choisis une option parmi :" << endl
                     << "0: Stats                 1: Attack" << endl
                     << "2: Potion de soin        3: Potion de dégat" << endl
                     << "4: Manuel de potions     5: Passer son tour" << endl
                     << endl
                     << ":";

                cin >> options;
                system("clear");
                if (options == 0)
                {
                    cout << joueur1 << endl
                         << "Classe : " << choi1 << endl
                         << pv1 << "/" << pv1max << " Pv restants" << endl
                         << dgt1max << " Dégats max" << endl
                         << dgt1min << " Dégats min" << endl
                         << dgt1crit << " Dégats crit" << endl
                         << poH1 << " Potions de soin restantes" << endl
                         << poD1 << " Potion de dégats restantes" << endl
                         << endl;
                }
                else if (options == 1)
                {

                    desDe20 = alDesDe20(gen);
                    if (desDe20 <= 3)
                    {
                        degats1 = 0;
                        cout << "Vous avez attaqué mais l'énnemi a réussit a esquiver." << endl
                             << endl;
                        tour = 2;
                    }
                    else if (desDe20 <= 16)
                    {
                        degats1 = alDegat1(gen);
                        if (classj2 == 3)
                        {
                            if (ulti2 > 0)
                            {
                                pv1 = pv1 - (degats1 / 3);
                                cout << "Vous avez attaqué mais malgres votre puissance vous n'avez tout de même pas reussit à endomager l'armure de votre ennemie" << endl
                                     << endl;
                                sleep(1);
                                cout << joueur2 << " a tout de même subbit l'attaque qu'il vous a en partie renvoyé." << endl
                                     << endl;
                                sleep(1);
                                degats1 = 0;
                            }
                        }
                        cout << "Vous avez attaqué et infligé: " << degats1 << " points de dégats." << endl
                             << endl;
                        pv2 = pv2 - degats1;
                        tour = 2;
                    }
                    else if (desDe20 <= 20)
                    {
                        degats1 = dgt1crit;
                        if (classj2 == 3)
                        {
                            if (ulti2 > 0)
                            {
                                pv1 = pv1 - (degats1 / 3);
                                cout << "Vous avez attaqué mais malgres votre puissance vous n'avez tout de même pas reussit à endomager l'armure de votre ennemie" << endl
                                     << endl;
                                sleep(1);
                                cout << joueur2 << " a tout de même subbit l'attaque qu'il vous a en partie renvoyé." << endl
                                     << endl;
                                sleep(1);
                                degats1 = 0;
                            }
                        }
                        cout << "Vous avez attaqué et infligé de lourds dégats: " << degats1 << endl
                             << endl;
                        pv2 = pv2 - degats1;
                        tour = 2;
                    }
                }
                else if (options == 2)
                {
                    if (poH1 > 0)
                    {
                        pv1 = pv1 + 75;
                        poH1 = poH1 - 1;
                        if (pv1 > pv1max)
                        {
                            pv1 = pv1max;
                        }
                        cout << "Vous sentez vos blessures se refermer et voyez votre vitalité augmenter" << endl
                             << endl;
                        tour = 2;
                    }
                    else
                    {
                        cout << "Vous n'avez plus de potions en votre possession, vous pouvez toujours tenter autre chose:" << endl
                             << endl;
                    }
                }
                else if (options == 3)
                {
                    if (poD1 > 0)
                    {
                        cout << "vous sentez vos forces monter en puissance et une rage s'empare de vous..." << endl;
                        sleep(2);

                        desDe20 = alDesDe20(gen);
                        if (desDe20 <= 3)
                        {
                            degats1 = 0;
                            cout << "Vous avez attaqué mais votre puissance vous a paralisé pendant un court instant." << endl
                                 << endl;
                            poD1 = poD1 - 1;
                            tour = 2;
                        }
                        else if (desDe20 <= 16)
                        {
                            degats1 = alDegat1(gen);
                            degats1 = degats1 + 45;
                            poD1 = poD1 - 1;
                            if (classj2 == 3)
                            {
                                if (ulti2 > 0)
                                {
                                    pv1 = pv1 - (degats1 / 3);
                                    cout << "Vous avez attaqué mais malgres votre puissance vous n'avez tout de même pas reussit à endomager l'armure de votre ennemie" << endl
                                         << endl;
                                    sleep(1);
                                    cout << joueur2 << " a tout de même subbit l'attaque qu'il vous a en partie renvoyé." << endl
                                         << endl;
                                    sleep(1);
                                    degats1 = 0;
                                }
                            }
                            cout << "Vous avez attaqué et infliger: " << degats1 << " points de dégats avec une puissance dévastatrice." << endl
                                 << endl;
                            pv2 = pv2 - degats1;
                            tour = 2;
                        }
                        else if (desDe20 <= 20)
                        {
                            degats1 = dgt1crit + 45;
                            poD1 = poD1 - 1;
                            if (classj2 == 3)
                            {
                                if (ulti2 > 0)
                                {
                                    pv1 = pv1 - (degats1 / 3);
                                    cout << "Vous avez attaqué mais malgres votre puissance vous n'avez tout de même pas reussit à endomager l'armure de votre ennemie" << endl
                                         << endl;
                                    sleep(1);
                                    cout << joueur2 << " a tout de même subbit l'attaque qu'il vous a en partie renvoyé." << endl
                                         << endl;
                                    sleep(1);
                                    degats1 = 0;
                                }
                            }
                            cout << "Votre rage étais si forte que vous avez attaqué et infligé des dégats monstrueux: " << degats1 << endl
                                 << endl;
                            pv2 = pv2 - degats1;
                            tour = 2;
                        }
                        if (ulti2 > 0)
                        {
                            if (classj2 == 3)
                            {
                                if (degats1 > 0)
                                {
                                }
                            }
                        }
                    }
                    else
                    {
                        cout << "Votre rage est épuisé, vous pouvez toujours tenter autre chose:" << endl
                             << endl;
                    }
                }
                else if (options == 4)
                {
                    cout << "Vous avez demandé a un expert de vous expliquer le contenue de votre équipement:" << endl;
                    sleep(1);
                    cout << "Vos potions de soin rapportent 75 mais passent votre tour;" << endl;
                    cout << "Vos potions de dégats augmente de 45 votre puissance, forcent à attaquer et se perdent même si l'attaque échoue." << endl
                         << endl;
                }
                else if (options == 5)
                {
                    cout << "Vous avez oublié quoi faire et avez mis fin a votre tour..." << endl
                         << endl;
                    tour = 2;
                }
                else
                {
                    sleep(1);
                    cout << "Option invalide, veulliez entrer une option valide, le temps n'est pas compté." << endl
                         << endl;
                    sleep(1);
                }
                if (tour == 2)
                {
                    if (classj2 == 3)
                    {
                        if (ulti2 > 0)
                        {
                            ulti2 = ulti2 - 1;

                            if (ulti2 == 0)
                            {
                                cout << "Il semblerais que l'armure d'" << joueur2 << " se soit affaiblie, ses materiaux semblent êtres revenue à leurs états de base." << endl
                                     << endl;
                                delai2 = delai2max;
                            }
                        }
                    }
                    if (cryptage == 1)
                    {
                        if (pv2 < (pv2max / 2))
                        {
                            cout << "Tres bien humain, je vois que je t'ai sous estimé, je me nomme " << joueur2 << " et j'appartiens a la classe : " << choi2 << endl;
                            sleep(5);
                            cout << "Tu a fais une grave erreur en venant m'affronter ici, en ces lieux." << endl;
                            sleep(3);
                            cout << "Mais pour féliciter les efforts que tu a mis pour en arriver là, je t'autorise à acceder a mes statistiques." << endl
                                 << endl;
                            sleep(4);
                            cout << "Décryptage en cours ..." << endl
                                 << endl;
                            sleep(4);
                            cout << "Contemple l'erreur que tu viens de faire en venant m'affronter !" << endl
                                 << endl;
                            sleep(3);
                            cout << "Décriptage terminé, affichage des statistiques ennemi..." << endl
                                 << endl
                                 << endl;
                            sleep(4);
                            cout << joueur2 << endl
                                 << "Type d'antité : " << choi2 << endl
                                 << "Les Pv max de cette entité sont de : " << pv2max << endl
                                 << pv2 << " Pv restants" << endl
                                 << dgt2max << " Dégats max" << endl
                                 << dgt2min << " Dégats min" << endl
                                 << dgt2crit << " Dégats crit" << endl
                                 << poH2 << " Potions de soin restantes" << endl
                                 << poD2 << " Potion de dégats restantes" << endl
                                 << endl
                                 << endl;
                            sleep(3);
                            cout << "Maintenant que tu sais tout ca, je ne vais plus être si clément avec toi, tu va connaitre toute ma puissance !" << endl
                                 << endl;
                            sleep(5);
                            cryptage = 0;
                        }
                    }
                }
            }

            // #########################################################################################################################

            else if (tour == 2)
            {
                cout << endl
                     << endl
                     << endl
                     << endl
                     << endl
                     << endl;
                cout << "Tour de " << joueur2 << endl
                     << endl;

                options = alOptions(gen);
                if (options == 5)
                {
                    options = alOptions(gen);
                }
                if (options == 2)
                {
                    options = 1;
                }
                else if (options == 4) // ulti
                {
                    if (ulti2 > 0)
                    {
                        options = 1;
                    }
                    if (delai2 > 0)
                    {
                        options = 1;
                    }
                    if (pv2 > (pv2max / 2))
                    {
                        options = 1;
                    }
                }
                else if (options == 3)
                {
                    if (poD2 == 0)
                    {
                        options = 1;
                    }
                }

                if (pv2 <= (pv2max / 4))
                {
                    if (poH2 > 0)
                    {
                        options = 2;
                    }
                }
                cout << options << endl;

                if (options == 0)
                {
                    if (cryptage == 0)
                    {
                        cout << joueur2 << endl
                             << "Type d'antité : " << choi2 << endl
                             << "Les Pv max de cette entité sont de : " << pv2max << endl
                             << pv2 << "/" << pv2max << " Pv restants" << endl
                             << dgt2max << " Dégats max" << endl
                             << dgt2min << " Dégats min" << endl
                             << dgt2crit << " Dégats crit" << endl
                             << poH2 << " Potions de soin restantes" << endl
                             << poD2 << " Potion de dégats restantes" << endl
                             << endl;
                    }
                    else
                    {
                        cout << "Tentative de décryptage des statistiques de l'entitée échoué, vous n'avez pas encore atteint toute les conditions." << endl
                             << endl;
                    }
                }
                else if (options == 1)
                {
                    desDe20 = alDesDe20(gen);
                    if (desDe20 <= 3)
                    {
                        degats2 = 0;
                        cout << joueur2 << " a attaqué mais son énnemi a réussit a esquiver." << endl
                             << endl;
                        tour = 1;
                    }
                    else if (desDe20 <= 16)
                    {
                        degats2 = alDegat2(gen);
                        cout << joueur2 << " a attaqué et infligé: " << degats2 << " points de dégats." << endl
                             << endl;
                        pv1 = pv1 - degats2;
                        tour = 1;
                    }
                    else if (desDe20 <= 20)
                    {
                        degats2 = dgt2crit;
                        cout << joueur2 << " a attaqué et infligé de lourds dégats: " << degats2 << endl
                             << endl;
                        pv1 = pv1 - degats2;
                        tour = 1;
                    }
                    if (ulti2 > 0)
                    {
                        if (classj2 == 2)
                        {
                            if (typej2 == 2)
                            {
                                pv2 = pv2 + (degats2 * 50 / 100);
                                if (degats2 < 0)
                                {
                                    cout << joueur2 << " s'est régénéré." << endl
                                         << endl;
                                }
                            }

                            if (typej2 == 3)
                            {
                                pv2 = pv2 + (degats2 * 50 / 100);
                                if (degats2 < 0)
                                {
                                    cout << joueur2 << " s'est régénéré." << endl
                                         << endl;
                                }
                            }
                        }
                    }
                }
                else if (options == 2)
                {
                    if (poH2 > 0)
                    {
                        pv2 = pv2 + (pv2max * 10 / 100);
                        poH2 = poH2 - 1;
                        if (pv2 > pv2max)
                        {
                            pv2 = pv2max;
                        }
                        if (classj2 == 1)
                        {
                            cout << joueur2 << " sentoure de lumière, ses blessures se referment et sa vitalité augmente" << endl;
                        }
                        if (classj2 == 2)
                        {
                            cout << joueur2 << " fais jair des miasmes de ses blessures, sa vitalité augmente a mesure qu'il se reconstitue" << endl;
                        }
                        if (classj2 == 3)
                        {
                            cout << joueur2 << " vois son armure se réparer, son corp se solidifie et brille a nouveau" << endl;
                        }
                        cout << endl;
                        tour = 1;
                    }
                }
                else if (options == 3)
                {
                    if (poD2 > 0)
                    {
                        if (classj2 == 1)
                        {
                            cout << joueur2 << " sens ses forces monter en puissance et une lumière éblouissante illumine toute la pièce..." << endl;
                        }
                        if (classj2 == 2)
                        {
                            cout << joueur2 << " sens ses forces monter en puissance et une aura ténébreuse s'empare de lui..." << endl;
                        }
                        if (classj2 == 3)
                        {
                            cout << joueur2 << " sens ses forces monter en puissance et ses armes se solidifient..." << endl;
                        }

                        sleep(2);
                        desDe20 = alDesDe20(gen);
                        if (desDe20 <= 3)
                        {
                            degats2 = 0;
                            cout << joueur2 << " a attaqué mais sa puissance l'a paralisé pendant un court instant." << endl
                                 << endl;
                            poD2 = poD2 - 1;
                            tour = 1;
                        }
                        else if (desDe20 <= 16)
                        {
                            degats2 = alDegat2(gen);
                            degats2 = degats2 + 50;
                            poD2 = poD2 - 1;
                            cout << joueur2 << " a attaqué et infliger: " << degats2 << " points de dégats avec une puissance dévastatrice." << endl
                                 << endl;
                            pv1 = pv1 - degats2;
                            tour = 1;
                        }
                        else if (desDe20 <= 20)
                        {
                            degats2 = dgt2crit + 50;
                            poD2 = poD2 - 1;
                            cout << "La puissance de " << joueur2 << " étais si forte qu'elle a infligé des dégats monstrueux: " << degats2 << endl
                                 << endl;
                            pv1 = pv1 - degats2;
                            tour = 1;
                        }
                        if (ulti2 > 0)
                        {
                            if (classj2 == 2)
                            {
                                if (typej2 == 2)
                                {
                                    pv2 = pv2 + (degats2 * 50 / 100);
                                    if (degats2 < 0)
                                    {
                                        cout << joueur2 << " s'est régénéré." << endl
                                             << endl;
                                    }
                                }

                                if (typej2 == 3)
                                {
                                    pv2 = pv2 + (degats2 * 50 / 100);
                                    if (degats2 < 0)
                                    {
                                        cout << joueur2 << " s'est régénéré." << endl
                                             << endl;
                                    }
                                }
                            }
                        }
                    }
                }
                else if (options == 4)
                {
                    ulti2 = ulti2max;
                    if (classj2 == 1)
                    {
                        cout << joueur2 << " déploie de grandes ailes dans son dos, et des chaines s'emparent de votre corp, vous immobilisant." << endl;
                    }
                    if (classj2 == 2)
                    {
                        typej2 = alTypej2(gen);
                        if (typej2 == 1)
                        {
                            cout << joueur2 << " s'enble s'être entouré d'une aura sombre, des miasmes vous entourent réduisant peu a peu votre vitalitée." << endl;
                            sleep(1);
                            cout << "L'effet de Corrosion a été affécté à " << joueur1 << endl;
                            sleep(1);
                            cout << "Cela réduit votre seuil de vie maximale au fils du temps, et vous inflige 10 dégats chaques tours." << endl
                                 << endl;
                        }
                        if (typej2 == 2)
                        {
                            cout << "L'arme de " << joueur2 << " s'enble se teinter d'une couleure rouge sang, se faire toucher par ces attaques pourrais ếtre désaventageux." << endl;
                            sleep(1);
                            cout << "L'effet de Saignement a été affécté à " << joueur1 << endl;
                            sleep(1);
                            cout << "Cela convertis 50% des degats infligé par l'ennemi, en vitalité, le régénérant a chaque coups subits." << endl
                                 << endl;
                        }
                        if (typej2 == 3)
                        {
                            cout << joueur2 << " s'enble s'être entouré d'une aura sombre, des miasmes vous entourent réduisant peu a peu votre vitalitée." << endl;
                            sleep(1);
                            cout << "De plus, l'arme de " << joueur2 << " s'enble se teinter d'une couleure rouge sang, se faire toucher par ces attaques pourrais ếtre désaventageux." << endl;
                            sleep(1);
                            cout << "L'effet de Corrosion et de Saignement ont été affécté à " << joueur1 << endl;
                            sleep(1);
                            cout << "Cela convertis 50% des degats infligé par l'ennevi, en vitalité, le régénérant a chaque coups subits." << endl;
                            sleep(1);
                            cout << "Cela réduit aussi votre seuil de vie maximale au fils du temps, et vous inflige 10 dégats chaques tours." << endl
                                 << endl;
                        }
                    }
                    if (classj2 == 3)
                    {
                        cout << joueur2 << " se met en position de défence, son armure change de couleure et de materiaux." << endl;
                        sleep(1);
                        cout << "Sa résistance semble ếtre impénétrable désormais." << endl
                             << endl;
                    }
                }
                else if (options == 5)
                {
                    cout << joueur2 << " a eu un moment de folie et a mis fin a son tour..." << endl
                         << endl;
                    tour = 1;
                }
                if (ulti2 == 0)
                {
                    if (tour == 1)
                    {
                        delai2 = delai2 - 1;
                    }
                }
                if (ulti2 > 0)
                {
                    if (tour == 1)
                    {
                        if (classj2 == 1)
                        {
                            tour = 2;
                            ulti2 = ulti2 - 1;
                            pv2 = pv2 + (pv2max * 5 / 100);
                            cout << joueur2 << " semble s'etre régénérée grace a des esprits lumineux." << endl
                                 << endl;
                            sleep(2);

                            if (ulti2 == 0)
                            {
                                cout << "Les ailes de " << joueur2 << " semblent s'être rétractées, et ses chaines semblent avoir subitement disparues." << endl
                                     << endl;
                                delai2 = delai2max;
                                tour = 1;
                            }
                        }
                        if (classj2 == 2)
                        {
                            ulti2 = ulti2 - 1;
                            if (typej2 == 1)
                            {
                                pv1 = pv1 - 10;
                                pv1 = pv1 - (pv2max * 2 / 100);
                                pv1max = pv1max - (pv2max * 2 / 100);
                                cout << joueur1 << " a recu des degats de Corrosion." << endl
                                     << endl;
                            }
                            if (typej2 == 3)
                            {
                                pv1 = pv1 - 10;
                                pv1 = pv1 - (pv2max * 2 / 100);
                                pv1max = pv1max - (pv2max * 2 / 100);
                                cout << joueur1 << " a recu des degats de Corrosion." << endl
                                     << endl;
                            }
                            if (ulti2 == 0)
                            {
                                delai2 = delai2max;
                                if (typej2 == 1)
                                {
                                    cout << "Il semblerais que l'aura sombre de " << joueur2 << " se soit affaiblie, sa rage semble s'être calmée et les miasmes sur vous on disparus." << endl
                                         << endl;
                                }
                                if (typej2 == 2)
                                {
                                    cout << "La lame de " << joueur2 << " s'enble avoir repris ses couleures initiales." << endl
                                         << endl;
                                }
                                if (typej2 == 3)
                                {
                                    cout << "Il semblerais que l'aura sombre de " << joueur2 << " se soit affaiblie, sa rage semble s'être calmée et les miasmes sur vous on disparus."
                                         << endl;
                                    sleep(1);
                                    cout << "De plus, la lame de " << joueur2 << " s'enble avoir repris ses couleures initiales." << endl
                                         << endl;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// #####################################################################################################################################################
// #####################################################################################################################################################
// #####################################################################################################################################################

void pve(string joueur1, int pv1, int pv1max, int degats1, int dgt1max, int dgt1min, int dgt1crit, int poH1, int poD1, string choi1)
{
    cout << endl
         << endl
         << "Preparation de la vague d'ennemis..." << endl
         << endl;
    sleep(3);

    cout << endl
         << endl
         << "Ce mode n'est pas finis, vous avez donc gagné car tout le monde a fuis face à ces bugs." << endl
         << endl;
    sleep(1);
}

// #####################################################################################################################################################
// #####################################################################################################################################################
// #####################################################################################################################################################
// #####################################################################################################################################################
// #####################################################################################################################################################
void modeDeJeux(string joueur1, int pv1, int pv1max, int degats1, int dgt1max, int dgt1min, int dgt1crit, int poH1, int poD1, int classj1, string choi1)
{
    cout
        << "Selectionnez le mode de jeux que vous voulez: " << endl
        << endl;
    sleep(1);
    cout << "1:PvP IA,     2:PvP 2 Joueurs,     3:PvE,     4:PvE Boss" << endl
         << endl;
    sleep(1);
    cout << "Veuillez entrer uniquement le chiffre correspondant :" << endl;
    unsigned modeDeJeu;
    while (true)
    {
        cin >> modeDeJeu;
        if (modeDeJeu >= 1 && modeDeJeu <= 4)
            break;
        else
            cout << "Veuillez entrer un chiffre valide (1, 2, 3 ou 4) :" << endl;
    }
    system("clear");

    if (modeDeJeu == 1)
    {
        cout << "Votre mode de jeux a bien été definis sur: PvP IA" << endl
             << endl;
        sleep(5);
        pvpIA(joueur1, pv1, pv1max, degats1, dgt1max, dgt1min, dgt1crit, poH1, poD1, choi1);
    }
    else if (modeDeJeu == 2)
    {
        cout << "Votre mode de jeux a bien été definis sur: PvP 2 Joueurs" << endl
             << endl;
        sleep(5);
        deuxjoueur(joueur1, pv1, pv1max, degats1, dgt1max, dgt1min, dgt1crit, poH1, poD1, choi1);
    }
    else if (modeDeJeu == 3)
    {
        cout << "Votre mode de jeux a bien été definis sur: PvE" << endl
             << endl;
        sleep(5);
        pve(joueur1, pv1, pv1max, degats1, dgt1max, dgt1min, dgt1crit, poH1, poD1, choi1);
    }
    else if (modeDeJeu == 4)
    {
        cout << "Votre mode de jeux a bien été definis sur: PvE Boss" << endl
             << endl;
        sleep(5);
        pvBoss(joueur1, pv1, pv1max, degats1, dgt1max, dgt1min, dgt1crit, poH1, poD1, classj1, choi1);
    }
    else
    {
        cout << "Ce n'est pas chronométré, selectionnez un chiffre valide." << endl
             << endl;
        sleep(3);
        system("clear");
        modeDeJeux(joueur1, pv1, pv1max, degats1, dgt1max, dgt1min, dgt1crit, poH1, poD1, classj1, choi1);
    }
}

// #####################################################################################################################################################

int main()
{
    system("clear");
    sleep(2);
    cout << "Bonjour voyageur, et bienvenue dans ce nouveau monde appeler Dinotofu;" << endl;
    sleep(2);
    cout << "monde de fantaisie ou règnent arène et baston." << endl
         << endl;
    sleep(2);

    cout << "Quel est ton nom ?" << endl;
    string joueur1;
    cin >> joueur1;
    cout << endl;
    cout << "Tres bien " << joueur1 << endl;
    sleep(2);

    cout << "Choisis ta classe et entre dans l'arène parmi ces 3 classes: " << endl
         << endl;
    sleep(2);
    choixJ2();

    int pv1;
    int pv1max;
    int degats1 = 0;
    int dgt1max;
    int dgt1min;
    int dgt1crit;
    int poH1;
    int poD1;
    int classj1;
    string choi1;

    while (true)
    {
        cin >> classj1;
        if (classj1 >= 1 && classj1 <= 3)
            break;
        else
            cout << "Veuillez entrer un chiffre valide (1, 2 ou 3) :" << endl;
    }

    if (classj1 == 1)
    {
        pv1 = 200;
        pv1max = 200;
        dgt1max = 20;
        dgt1min = 5;
        dgt1crit = 30;
        poH1 = 4;
        poD1 = 3;
        choi1 = "Chevalier";
    }
    else if (classj1 == 2)
    {
        pv1 = 150;
        pv1max = 150;
        dgt1max = 30;
        dgt1min = 10;
        dgt1crit = 45;
        poH1 = 5;
        poD1 = 2;
        choi1 = "Classe Léger";
    }
    else if (classj1 == 3)
    {
        pv1 = 400;
        pv1max = 400;
        dgt1max = 12;
        dgt1min = 5;
        dgt1crit = 20;
        poH1 = 2;
        poD1 = 5;
        choi1 = "Classe Lourde";
    }
    system("clear");
    cout << joueur1 << ", vous avez choisis la classe: " << choi1 << "; et avez acqueris avec succès ses statistiques." << endl
         << endl;

    modeDeJeux(joueur1, pv1, pv1max, degats1, dgt1max, dgt1min, dgt1crit, poH1, poD1, classj1, choi1);

    return 0;
}