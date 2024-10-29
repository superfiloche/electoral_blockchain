#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "headers/rsa.h"
#include "headers/key.h"
#include "headers/signature.h"
#include "headers/cellkey.h"
#include "headers/cellprotected.h"
#include "headers/hash.h"
#include "headers/block.h"
#include "headers/tree.h"
#include "headers/vote.h"

int is_in_tab(int *tab, int taille, int number)
{
    for (int i = 0; i < taille; i++)
    {
        if (tab[i] == number)
            return 1;
    }
    return 0;
}

void affiche_tab(int *tab, int taille)
{
    for (int i = 0; i < taille; i++)
    {
        printf("%d ", tab[i]);
    }
    puts("");
}

void affiche_str(char *str)
{
    for (int i = 0; i < (int) strlen(str); i++)
    {
        printf("%c", str[i]);
    }
    puts("");
}

void print_long_vector(long *result, int size)
{
    printf("Vector : [");
    for (int i = 0; i < size; i++)
    {
        printf("%lx \t", result[i]);
    }
    printf("]\n");
}

static int intCompare(const void *p1, const void *p2) // intCompare pour le qsort d' "stdlib.h"
{
    int int_a = *((int *)p1);
    int int_b = *((int *)p2);

    if (int_a == int_b)
        return 0;
    else if (int_a < int_b)
        return -1;
    else
        return 1;
}

void generate_random_data(int nv, int nc)
{
    FILE *fk = fopen("keys.txt", "w+");
    FILE *fc = fopen("candidates.txt", "w+");
    FILE *fv = fopen("declarations.txt", "w+");

    // fichier "keys.txt"
    Key *pkey = (Key *)malloc(sizeof(Key));
    Key *skey = (Key *)malloc(sizeof(Key));
    if (nv < nc)
    {
        puts("Nombre de candidat supérieur au nombre de citoyens.");
        return;
    }
    for (int i = 0; i < nv; i++)
    {
        init_pair_keys(pkey, skey, 3, 7); // on réutilise pkey skey qu'on a malloc précédemment
        fprintf(fk, "(%lx,%lx) (%lx,%lx)\n", pkey->val, pkey->n, skey->val, skey->n);
    } // un fprintf pas très coding style mais adapté pour la fonction str_to_key
    free(pkey);
    free(skey);

    // fichier "candidates.txt"
    char ligne[256] = {0};
    char *buff = ligne;
    size_t len = 256;
    int candidates[nc]; // tableau contenant les indices de ligne des candidats

    int i = 0; // compteur de candidat
    int alea;
    while (i < nc)
    {
        alea = (int)((rand() % nv) + 1);         // génération d'un random indice de ligne
        if (is_in_tab(candidates, i, alea) == 0) // ajout de ce random dans notre tableau contenant
        {                                        // les indices ligne de tous les candidats dans keys.txt
            candidates[i] = alea;
            i++;
        }
    }

    // affiche_tab(candidates, nc); pour tester qsort
    qsort(candidates, nc, sizeof(int), intCompare); // qsort de la bibliothèque "stdlib.h"
    // affiche_tab(candidates, nc);

    int n = 0;
    rewind(fk); // on revient au début du fichier "keys.txt"
    printf("Tableau des candidats : [");
    for (i = 0; i < nc; i++)
    {
        while (n != candidates[i])
        {
            fgets(buff, 256, fk); // pas possible de fseek une ligne spécifique, donc on lit ligne par ligne
            n++;                  // jusqu'à la bonne
        }
        printf("%d ", candidates[i]); // affichage pour tester
        // affiche_str(buff); pour tester
        fputs(buff, fc); // parcours du tableau candidates pour y retrouver les indices de ligne
    }
    printf("]");
    mem_reset(buff, len); // reset mémoire du buffer
    puts("");

    rewind(fk); // rewind pour recommencer le parcours du fichier pour le vote de chaque électeur
    char public[256];
    char secret[256];
    n = 0;

    // declaration des variable à free
    Key *kpublic;
    Key *ksecret;
    Signature *s;
    char *mess;

    // buffer pour le second fichier au cas où il est possible de perdre la progression sur le premier fichier
    char line[256] = {0};
    char *buffer = line;

    // boucle qui va parcourir chaque ligne du fichier des clés
    for (i = 1; i < nv + 1; i++)
    {
        fgets(buff, 256, fk);
        sscanf(buff, "%s %s", public, secret);
        kpublic = str_to_key(public);
        ksecret = str_to_key(secret);

        if (is_in_tab(candidates, nc, i) == 0) // si la ligne actuelle ne représente pas les clés publiques
        {                                      // et secrètes d'un candidat, on prend en compte son vote
            // génération random de la ligne du candidat
            alea = (int)((rand() % nc) + 1);
            while (n != alea)
            {
                fgets(buffer, 256, fc); // on parcourt le fichier des candidats et on s'arrête quand
                n++;                    // on tombe sur la ligne alea
            }

            sscanf(buffer, "%s %s", public, secret);
        }
        s = sign(public, ksecret); // on effectue la déclaration de vote en cryptant la clé publique du candidat (le str public)

        Protected *pr = init_protected(kpublic, public, s);
        mess = protected_to_str(pr);
        fprintf(fv, "%s", mess); // avant de tout print dans le fichier declarations.txt

        // on free à chaque boucle pour éviter les leaks
        free(ksecret);
        free(mess);
        freeprotected(pr);

        rewind(fc); // rewind du fichier candidat pour recommencer le parcours à 0
        n = 0;      // pour le prochain électeur
        mem_reset(buff, len);
        mem_reset(buffer, len);
    }

    fclose(fk);
    fclose(fc);
    fclose(fv);
}

void removedir(char* nomdoss)   //fonction qui supprime un répertoire entièrement (le contenu + le repértoire lui-même)
{
    //char location[256];
    char temp[256];
    size_t len = 256;
    DIR* rep = opendir(nomdoss);       //on crée un variable DIR* qui va nous permettre d'explorer le répertoire
    if (rep)
    {
        //printf("1 : %s\n", getcwd(location, 100));
        chdir(nomdoss);                //on change de répertoire et on se déplace dans le répertoire qu'on souhaite supprimer
        //printf("2 : %s\n", getcwd(location, 100));
        struct dirent* dir;
        while ((dir = readdir(rep)))   //et on fait une boucle while qui vérifie s'il existe des éléments
        {   
            if (strcmp(dir->d_name,".") != 0 && strcmp(dir->d_name, "..") != 0)     //si les éléments ne sont ni le répertoire "." ni ".." alors
            {
                strcat(temp, dir->d_name);                                          //alors on récupère le nom du fichier (qui contient aussi son format .txt)
                remove(temp);                                                       //et on le supprime
                mem_reset(temp, len);                                               //on réinitialise notre tampon
            }
        }
        chdir("..");                   //on retourne sur notre répertoire de travail
        //printf("3 : %s\n", getcwd(location, 100));
    }
    closedir(rep);                     //on closedir à la manière d'un fclose
    remove(nomdoss);                   //puis on remove le répertoire vide
}

int main(void)
{
    srand(time(NULL));
    Key *sKey = (Key *)malloc(sizeof(Key));
    Key *pKey = (Key *)malloc(sizeof(Key));
    init_pair_keys(pKey, sKey, 3, 7);
    printf("pkey: %lx, %lx \n", pKey->val, pKey->n);
    printf("pkey: %lx, %lx \n", sKey->val, pKey->n);

    // Testing Key Serialization
    char *chaine = key_to_str(pKey);
    printf("key_to_str: %s \n", chaine);
    Key *k = str_to_key(chaine);
    printf("str_to_key: %lx, %lx \n", k->val, k->n);

    // ajout
    free(chaine);

    // Testing signature
    // Candidate keys
    Key *pKeyC = malloc(sizeof(Key));
    Key *sKeyC = malloc(sizeof(Key));
    init_pair_keys(pKeyC, sKeyC, 3, 7);
    // Declaration :
    char *mess = key_to_str(pKeyC);

    // ajout
    char *temp = key_to_str(pKey);

    printf("%s vote pour %s\n", temp, mess);

    // ajout
    free(temp);

    Signature *sgn = sign(mess, sKey);
    printf("signature: ");
    print_long_vector(sgn->content, sgn->size);
    chaine = signature_to_str(sgn);
    printf("signature_to_str: %s \n", chaine);

    // ajout
    free(sgn->content);
    free(sgn);

    sgn = str_to_signature(chaine);
    printf("str_to_signature: ");
    print_long_vector(sgn->content, sgn->size);

    // ajout
    free(chaine);

    // Testing protected
    Protected *pr = init_protected(pKey, mess, sgn);
    // Verification
    if (verify(pr))
    {
        printf("Signature valide\n");
    }
    else
    {
        printf("Signature non valide\n");
    }
    chaine = protected_to_str(pr);
    printf("protected_to_str: %s\n", chaine);

    // ajout
    freeprotected(pr);

    pr = str_to_protected(chaine);

    // ajout
    temp = signature_to_str(pr->sgn);
    char *temp2 = key_to_str(pr->pubk);

    printf("str_to_protected: %s %s %s\n", temp2, pr->mess, temp);
    puts(""); // rajout d'un \n pour plus de lisibilité
    // free(pKey); on enlève car il a déjà été free dans le freeprotected(pr) précédent
    free(sKey);
    free(pKeyC);
    free(sKeyC);

    // ajout
    free(k);
    freeprotected(pr);
    free(chaine);
    free(temp);
    free(temp2);
    free(mess);

    /* Pour le test de la durée d'exécution de generate_random_data
    clock_t temps_initial ;
    clock_t temps_final ;
    double temps_cpu;
    */

    int nbcitoyens = 1000;
    int nbcandidats = 5;

    puts("__________________JEUX DE TESTS_________________\n");
    puts("Generate_random_data va s'exécuter");
    // temps_initial = clock();
    generate_random_data(nbcitoyens, nbcandidats);
    puts("Generate_random_data a été executé");
    
    /*
    temps_final = clock();
    temps_cpu = ((double) (temps_final - temps_initial)) / CLOCKS_PER_SEC;
    printf("Temps mis par generate_random_data pour 50 citoyens dont 10 candidats : %.5f\n", temps_cpu);
    puts("");
    FILE* tempsgenerate = fopen("temps.txt", "a");
    fprintf(tempsgenerate, "Nb citoyens : %d, nb candidats : %d, temps : %.3f s\n", nbcitoyens, nbcandidats, temps_cpu);
    fclose(tempsgenerate);
    */
    // marche bien 0 leaks et fichiers bien agencés

    /*
    puts("________________Test de CellKey________________");
    CellKey *c = read_public_key("keys.txt");
    print_list_keys(c);
    delete_list_ck(c);
    puts("______________________________________________\n");
    */
    // marche bien 0 leaks

    // Test de create_cell_protected, insert_cell_protected et print_list_protected => normalement ça marche
    /*
    puts("_________Test partiel CellProtected_________");
    Key* ktest = (Key*) malloc(sizeof(Key));
    ktest->val = 123456;
    ktest->n = 456789;
    Signature* signtest = sign("prout", ktest);
    Protected* prtest = init_protected(ktest, "osef", signtest);

    Key* k2test = (Key*) malloc(sizeof(Key));
    k2test->val = 456789;
    k2test->n = 123456;
    Signature* s2igntest = sign("caca", k2test);
    Protected* p2rtest = init_protected(k2test, "oui", s2igntest);
    CellProtected* c2prtest = create_cell_protected(p2rtest);

    CellProtected* cptest = create_cell_protected(prtest);
    insert_cell_protected(&cptest, c2prtest);
    print_list_protected(cptest);
    delete_list_cp(cptest);
    puts("______________________________________________\n");
    */
    // marche bien 0 leaks

    /*
    puts("_____________Test de CellProtected_____________");
    CellProtected *cp = read_protected("declarations.txt");
    puts("print après read_protected");
    print_list_protected(cp);
    puts("");

    Key *ktest = (Key *)malloc(sizeof(Key));
    ktest->val = 123456;
    ktest->n = 456789;
    Signature *signtest = sign("prout", ktest);
    Protected *prtest = init_protected(ktest, "osef", signtest);
    CellProtected *cptest = create_cell_protected(prtest);
    insert_cell_protected(&cp, cptest);

    puts("print après ajout d'un élément frauduleux");
    print_list_protected(cp);
    puts("");

    puts("print après suppression des fraudes\n");
    suppr_fraude(&cp);
    print_list_protected(cp);

    delete_list_cp(cp);
    puts("______________________________________________\n");
    */
    // marche bien 0 leaks

    //TESTS PARTIE 4
    /*
    puts("_______________Test de HashTable_______________");
    CellKey *voters = read_public_key("keys.txt"); 
    CellKey *candidates = read_public_key("candidates.txt");

    HashTable *hv = create_hashtable(voters, nbcitoyens);
    HashTable *hc = create_hashtable(candidates, nbcandidats);

    puts("Affichage de la HashTable des candidats :");
    print_hashtable(hc);
    puts("\nAffichage de la HashTable des citoyens :");
    print_hashtable(hv);
    delete_hashtable(hc);
    delete_hashtable(hv);

    CellProtected *cp2 = read_protected("declarations.txt");
    puts("");
    suppr_fraude(&cp2);
    
    Key* test_compute = compute_winner(cp2, candidates, voters, nbcandidats, len_cellprotected(cp2));
    char* lacrim = key_to_str(test_compute);
    printf("Voici sa clé publique : %s\n", lacrim);
    free(lacrim);
    free(test_compute);

    delete_list_cp(cp2);
    delete_list_ck(voters);
    delete_list_ck(candidates);
    puts("______________________________________________\n");
    */

    /*
    puts("________________Test d'openssl________________");
    const char *s = "Rosetta code";
    unsigned char *d = SHA256 ((const unsigned char*)s, strlen (s), 0);
    int i;
    for (i = 0; i < SHA256_DIGEST_LENGTH ; i++)
        printf ("%02x", d[i]);                     //boucle qui affiche le resultat par SHA256
    putchar ('\n');                                //on affiche case par case en hexadecimal

    unsigned char* mrmime = hash_sha(s);
    printf("%s", mrmime);                          //affichage du resultat de notre fonction qui renvoie
    puts("\n");                                    //directement un string avec tout en hexadecimal
    free(mrmime);
    */

    /*
    puts("_________________Test de Block_________________");
    Block* bloc = read_block("block.txt");         //les fonctions read et write affichent le nombre de déclarations
    write_block("secondblock.txt", bloc);          //de votes qui ont été traités dans le bloc, si cela correspond à nos attentes
                                                   //then good :+1:
    char* str = block_to_str(bloc);
    printf("%s", str);                             //on affiche le résultat de notre block_to_str
    puts("\n");                                      //pour vérifier qu'il n'y ait aucun \n
	
	test3(compute_proof_of_work, bloc, 0, 6, 1);    //Attention, fonction qui lance les tests sur compute_proof_of_work donc très lent
    
    free(str);
    complete_delete_block(bloc);
    puts("______________________________________________\n");
    */

    puts("__________________Test de vote__________________");
    remove("Pending_votes.txt");                    //on supprime Pending_votes.txt s'il existe
    removedir("BlockChain");                        //on supprime le répertoire BlockChain et tout son contenu s'il existe
    //char location[256];
    //printf("%s\n", getcwd(location, 100));
    mkdir("./BlockChain/", 0700);                   //on crée un répertoire BlockChaine avec toutes les autorisations (0700 = permissions en octale)
                                                    //actuellement toutes les sessions que ce soit Root, Admin ou Guest peuvent lire écrire et modifier le répertoire
    //*
    printf("Il y a %d citoyens et %d candidats\n\n", nbcitoyens, nbcandidats);

    puts("Generate_random_data va s'exécuter");
    generate_random_data(nbcitoyens, nbcandidats);
    puts("Generate_random_data a été executé");

    puts("Lecture des cles");
    CellKey *voters = read_public_key("keys.txt"); 
    CellKey *candidates = read_public_key("candidates.txt");

    puts("Lecture des protected");
    CellProtected *cp = read_protected("declarations.txt");
    puts("Suppression des fraudes\n");
    suppr_fraude(&cp);

    CellProtected* decl = cp;                       //Notre liste de déclarations tampon
    CellTree* t = NULL;                             //Notre arbre tampon qui va servir à créer l'arbre final
    CellTree* nodetree = NULL;                      //Noeud d'arbre tampon
    CellKey* author = voters;                       //Liste d'électeurs tampon
    char nomfic[256] = {0};
    strcpy(nomfic, "Bloc");
    char tmp[256] = {0};
    char path[256] = {0};
    strcpy(path, "./BlockChain/");
    char num[256] = {0};
    char filetype[256] = {0};
    strcpy(filetype, ".txt");
    int i = 0;
    int j = 0;
    size_t len = 256;
    //int compteur = 0;

    while(decl)                                     //on récupère toutes les déclarations une par une 
    {
        submit_vote(decl->data);                    //on rajoute le vote dans Pending_votes
        i++;                                        //on incrémente notre compteur i
        if (i == 10 || (!decl->next))               //comme on avait complètement oublié l'existence du modulo %
        {                                           //on rajoute le || (!decl->next) qui permet de rentrer dans la boucle lorsqu'on a un nombre d'électeurs dont % 10 != 0
            i = 0;                                  //on remet à 0 notre compteur 
            create_block(t, author->data, 3);       //tous les 10 votes on crée un bloc dans Pending_block
           
            sprintf(num, "%d", ++j);                //on incrémente j qui va nous servir de compteur pour les noms des fichiers à venir
            strcpy(tmp, nomfic);
            strcat(tmp, num);
            strcat(tmp, filetype);                  //on strcat tout dans tmp pour avoir le nom du fichier complet

            add_block(3, tmp);                      //si le bloc dans Pending_block est valide, on l'ajoute dans un fichier Blocj.txt dans le répertoire BlockChain

            strcat(path, tmp);
            if(!t)
            {   
                t = create_node(read_block(path));  //si c'est le tout premier block, il devient la racine de l'arbre et on create_node
            }
            else                                    //sinon c'est un fils
            {   
                nodetree = t;
                while (nodetree->firstChild)
                {
                    nodetree = nodetree->firstChild;                    //on atteint le dernier élément de la branche actuelle
                }
                add_child(nodetree, create_node(read_block(path)));     //et on l'ajoute comme fils (puis on met à jour la hauteur de tous les éléments précédents)
            }

            mem_reset(tmp, len);                    //on réinitialise tmp et path pour s'assurer qu'on recommence avec un str complètement vide
            mem_reset(path, len);                   //comme ça pas de problème de répertoire ou de nom de fichier
            strcpy(path, "./BlockChain/");          //on réécrit dans path comme au début lors de sa déclaration
        }
        author = author->next;                      //après qu'on ait submit_vote, on peut passer à la clé suivante
        decl = decl->next;                          //et la déclaration suivante
        //compteur++;
        //printf("compteur : %d\n", compteur);      //compteur pour tester le nombre de vote submit
    }
    //print_tree(t);                                //test pour voir si la boucle marchait et si print_tree marchait

    CellTree* final = read_tree();                  //on récupère l'arbre final avec tous les fichiers Bloc dans le répertoire BlockChain
    print_tree(final);                              //on le print
    puts("");

    Key* winner = compute_winner_BT(final, candidates, voters, nbcandidats, nbcitoyens);
    char* win = key_to_str(winner);
    printf("Le vainqueur de l'élection : %s\nVive la démocratie, vive la République et vive la France\n", win);

    //on libère la mémoire
    free(winner);
    free(win);
    delete_list_ck(voters);
    delete_list_ck(candidates);
    delete_list_cp(cp);
    delete_tree(t);
    delete_tree(final);
    puts("______________________________________________\n");
    //*/
    return 0;
}