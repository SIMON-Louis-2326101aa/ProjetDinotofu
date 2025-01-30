int main(){
    Personnage hazak, fail;
    //Création de 2 objets de type Personnage : hazak et fail
    int poHFaible = 20;
    fail.attaquer(hazak); //fail attaque hazak
    hazak.boirePotionDeVie(poHFaible); //hazak récupère 20 de vie en buvant une potion
    fail.attaquer(hazak); //fail attaque hazak
    hazak.attaquer(fail); //hazak contre-attaque... 
    
    fail.changerArme("Double hache tranchante veneneuse de la mort", 40);
    fail.attaquer(hazak);
    return 0;
}