#include <stdio.h>
#include <openssl/sha.h>
#include "headers/block.h"

void write_block (char* nomfic, Block* bloc)
{
    FILE* fd = fopen(nomfic, "w");
    if (!fd)
    {
        errx(1, "Problème d'ouverture du fichier (write_block).");
    }

    char* pkey = key_to_str(bloc->author);                   //on doit convertir la clé en str avant le fprintf
    fprintf(fd, "%s\n", pkey);

    fprintf(fd, "%s\n", bloc->hash);

    fprintf(fd, "%s\n", bloc->previous_hash);

    fprintf(fd, "%d\n", bloc->nonce);

    CellProtected *tmp = bloc->votes;                       //comme on doit fprintf chaque déclaration, on doit parcourir le CellProtected
    int i = 0;                                              //compteur pour vérifier qu'on a bien le bon nombre de déclarations lues
    while (tmp)
    {
        char *ptos = protected_to_str(tmp->data);           //on convertit chaque déclaration en str
        fprintf(fd, "%s", ptos);                            //on les fprintf
        free(ptos);                                         //et on free aussitôt le str de la conversion
        tmp = tmp->next;
        i++;                                                
    }
    //printf("Nombre de déclarations écrites : %d\n", i);
    free(pkey);
    fclose(fd);
}

Block* read_block (char* nomfic)
{
    FILE* fd = fopen(nomfic, "r");
    if (!fd)
        errx(1, "Problème d'ouverture du fichier (write_block).");

    Block* bloc = calloc(1, sizeof(Block));
    if (!bloc)
        errx(1, "Problème d'allocation (calloc read_block)");

    char ligne[256] = {0};
    char* buff = ligne;
    size_t len = 256;
    char* temp = calloc(256, sizeof(char));

    //author
    long val;
    long n;
    fgets(buff, 256, fd);
    while (sscanf(buff, "(%lx,%lx)", &val, &n) != 2) //on vérifie qu'on a bien une clé
    {
        if (!fgets(buff, 256, fd))                   //on fgets tant qu'on a pas une clé (vérif par sscanf au dessus)
        {                                            //si fgets est NULL = fin du fichier
            puts("Pas de clé dans le fichier (starting point du bloc).");
            free(bloc);                              //on a atteint la fin du fichier donc on free bloc puis on ferme le fichier avant de
            fclose(fd);                              //trigger le message d'erreur qui va interrompre le programme (on limite les dégâts)
            errx(3, "Fin du fichier atteint.");
        }    
    }
    Key* auth = calloc(1, sizeof(Key));             
    init_key(auth, val, n);                         //on a récup une clé donc on calloc une clé et on l'initialise avec les valeurs récupérées
    bloc->author = auth;                            //puis affectation au bloc

    mem_reset(buff, len);                           //on réinitialise à 0 notre buffer pour avoir un buffer vierge

    //hash
    fgets(buff, 256, fd);      
    if (sscanf(buff, "%s\n", temp) != 1)            //on vérifie qu'on a bien récupéré qlq chose avec le sscanf
        errx(3, "Erreur de sscanf (hash dans read_block)");
    bloc->hash = (unsigned char*) strdup((const char*) temp);
    
    mem_reset(buff, len);
    mem_reset(temp, len);                           //on réinitialise notre temp qui sert à la vérification par sscanf

    //previous_hash
    fgets(buff, 256, fd);
    if (sscanf(buff, "%s\n", temp) != 1)
        errx(3, "Erreur de sscanf (previous_hash dans read_block)");
    bloc->previous_hash = (unsigned char*) strdup((const char*) temp);

    mem_reset(buff, len);
    mem_reset(temp, len);
    
    //nonce
    int nonce;
    fgets(buff, 256, fd);
    if (sscanf(buff, "%d", &nonce) != 1) //on vérifie si le sscanf fonctionne
    {
        if (bloc->hash)                  //si on a pas réussi à récuperer nonce, on free ce qu'on a alloué précédemment
            free(bloc->hash);            //pour limiter les memory leaks
        if (bloc->previous_hash)
            free(bloc->previous_hash);
        free(bloc);
        errx(1, "Erreur sscanf (pour nonce dans read_block)."); //tout ça avant de trigger l'affichage de l'erreur et l'interruption du programme
    }
    bloc->nonce = nonce;

    mem_reset(buff, len);

    //votes
    CellProtected* cp = NULL;
    int i = 0;
    while (fgets(buff, 256, fd))
    {
        insert_cell_protected(&cp, create_cell_protected(str_to_protected(buff)));  //ici pas de variable temporaire pour les CellProtected
        mem_reset(buff, len);                                                       //puisqu'on souhaite calloc pour chaque déclaration
        i++;                                                                        //donc on utilise directement create_cell_protected qui alloue déjà de l'espace
    }                                                                               //notre fonction traite aussi le cas où le contenu du premier pointeur est vide
    //printf("Nombre de déclarations lues : %d\n", i);
    bloc->votes = cp;

    free(temp);
    fclose(fd);
    return bloc;
}


char* block_to_str(Block* block)
{
    char* pkey = key_to_str(block->author);
    int len1 = strlen(pkey) + 1;
    
    char* str = calloc((len1), sizeof(char));           //chaîne résultat de la fonction calloc
    
    strcat(str, pkey);                                    //on y ajoute les éléments un par un
    free(pkey);                                           //et on free de suite chaque élément pour ne pas oublier

    if (block->previous_hash)
    {
        char* prevhash = strdup((const char*) block->previous_hash);
        int len2 = strlen(prevhash) + 1;
        str = realloc(str, (len1 + len2) * sizeof(char));
        strcat(str, prevhash);
        free(prevhash);
    }
    int length = strlen(str);

    int len3 = snprintf(NULL, 0, "%d", block->nonce) + 1; //"snprintf tells you length if you call it with NULL, 0 as first parameters" pour nonce
                                                          //https://stackoverflow.com/questions/8257714/how-to-convert-an-int-to-string-in-c
    char* nonce = calloc((len3), sizeof(char));
    snprintf(nonce, len3, "%d", block->nonce);

    CellProtected* tmp = block->votes;
    int len4 = 0;
    while (tmp)
    {
        char* data = protected_to_str(tmp->data);
        strtok(data, "\n");             //strtok permet de découper un string en le séparant en deux parties (token) à partir de "\n"
                                        //buff sera alors le string composé de toute ce qui précède "\n"
                                        //pour récupérer la seconde partie de buff, il faut rappeler strtok avec un NULL pointeur : char* spart = strtok(NULL, "\n");
                                        //en découpant le buff de cette manière, on enlève le "\n" qui nuit au block_to_str
        
        len4 += strlen(data) + 1;                            //len4 = lentotal des déclarations de vote
        str = realloc(str, (length + len3 + len4) * sizeof(char));  //on realloc avant de strcat pour avoir assez d'espace mémoire
        strcat(str, data);
        tmp = tmp->next;
        free(data);
    }

    strcat(str, nonce);                                      //on rajoute nonce à la fin et on free nonce
    free(nonce);

    int lenfin = strlen(str) + 1;                            //enfin, on refait un realloc avec seulement la taille nécessaire
    str = realloc(str, lenfin * sizeof(char));               //càd, la longueur réelle du str résultant + 1 ("\0")

    return str;
}

unsigned char* hash_sha (const char* s)
{
    unsigned char* hash = SHA256((const unsigned char*) s, strlen(s), 0);
    if (!hash)
        errx(3, "Erreur SHA256 (pour hash dans hash_sha)");

    unsigned char* sha = calloc(256, sizeof(unsigned char));
    if (!sha)
        errx(1, "Erreur d'allocation (pour sha dans hash_sha)");

    unsigned char temp[256] = {0};
    //size_t len = 256;
    //mem_reset(temp, len); conflicting types

    for (int i = 0; i < SHA256_DIGEST_LENGTH ; i++)
    {
        sprintf ((char*) temp, "%02x", hash[i]);
        strcat((char*) sha, (char*) temp);
        //mem_reset(temp);
    }
    return sha;
}

int nb_de_z(unsigned char* str)       //renvoie le nombre de 0 dans un str (unsigned char)
{
    int i = 0;
    while (str[i] == '0')
        i++;
    return i;
}

void compute_proof_of_work (Block* B, int d)
{
    if (!B->hash)
    {
        char* new = block_to_str(B);                            //si hash n'existe pas, on en crée un
        B->hash = hash_sha(new);
        free(new);
    }

    B->nonce = 0;
    int nb = nb_de_z(B->hash);                                  //si le nb de 0 est déjà bon on return
    if (nb == d)
    {
        printf("Le Bloc est deja valide\n");
        return;
    }
    
    unsigned char* hash = NULL;
    unsigned char* res = NULL;
    unsigned char nonce[11];
    int len = strlen((const char*) B->hash) + 1;                //pour calloc seulement le nécessaire
    int lennonce = 11;
    int lentot = len + lennonce;
    int bool = 0;
    
    while (nb != d)                                              //tant que le nb de 0 est inférieur à d
    {   
        bool = 0;
        //lennonce = snprintf(NULL, 0, "%d", B->nonce);           //nb de char du int B->nonce s'il était converti en str
        //nonce = calloc(lennonce + 1 , sizeof(unsigned char));   //on calloc seulement le nombre nécessaire
                                                                //en réalité on pourrait enlever le +1 puisque pas besoin de '\0'
        sprintf((char*) nonce, "%d", B->nonce);                 //et ensuite on convertit notre int en str et on le stocke dans le char nonce
        
        lentot = len + lennonce;                                //on additionne la taille de nonce en str + la taille de B->hash + 1 (c'est le \0)
        hash = calloc(lentot, sizeof(unsigned char));           //on calloc seulement le nécessaire
        strcpy((char*) hash, (char*) B->hash);                  //on peut alors y copier B->hash
        strcat((char*) hash, (char*) nonce);                    //et concaténer nonce
        
        res = hash_sha((const char*) hash);                     //on hash avec notre fonction et on stocke dans un str
        
        nb = nb_de_z(res);                                      //on vérifie qu'on a le bon nb de 0
        if (nb == d)
        {
            break;                                              //si c'est bon, on break into free les variables temporaires + affectation de hash
        }
        else                                                    //sinon, on free les variables temporaires pour permettre de nouveaux calloc
        {                                                       //plus long et plus coûteux en mémoire mais plus sûr => des str totalement vierges
            B->nonce++;                                         //et on incrémente B->nonce
            free(hash);
            bool = 1;
            if (nb != d)
                free(res);
        }
    }
    if (bool == 0)    
        free(hash);
    free(B->hash);
    B->hash = res;                                              //on affecte à B->hash le nouveau hash avec le nombre de 0 valide
    return;
}

int verify_block (Block* b, int d)
{
    if (!b)
        errx(2, "Erreur de paramètre (verify_block)");
    
    if (nb_de_z(b->hash) != d)          //on vérifie juste que le nombre de 0 correspond à d
        return 1; 
    
    return 0;
}


void delete_block(Block* bloc)
{
    bloc->author = NULL;    //on met à NULL pour ne pas le free
    free(bloc->hash);
    if (bloc->previous_hash)
        free(bloc->previous_hash);
    
    CellProtected* temp = NULL;             //on crée deux pointeurs temporaires qui vont nous permettre 
    CellProtected* parcours = bloc->votes;  //de free les CellProtected
    
    bloc->votes = NULL; //on met bloc->votes à NULL pour pouvoir free bloc
    
    while (parcours)
    {
        temp = parcours;
        parcours = parcours->next;
        temp->data = NULL; //on met data à NULL (pointeur vers le Protected)
        temp->next = NULL; //de même pour next
        free(temp);        //et on free seulement le CellProtected
    }
    bloc->nonce = 0;

    free(bloc);
}

void complete_delete_block (Block* bloc) //fonction qui free completement un bloc
{                                        //cad la cle + bloc->votes (le cellprotected)
    free(bloc->author);                  //pour éviter les leaks pendant les tests
    free(bloc->hash);
    if (bloc->previous_hash)
        free(bloc->previous_hash);
    if (bloc->votes)
        delete_list_cp(bloc->votes);
    free(bloc);
}

void delete_block_wovotes (Block* bloc)
{
    free(bloc->author);
    if (bloc->hash)
        free(bloc->hash);
    if (bloc->previous_hash)
        free(bloc->previous_hash);
    bloc->votes = NULL;
    free(bloc);
}

void test3 (void (*fct)(Block*, int), Block* b, int min, int max, int pas)
{
    //srand(time(NULL));                             //inutile
	clock_t temps_initial;
	clock_t temps_final;
    double temps_cpu;
    FILE* fd = fopen("compute_pof3.txt", "w");
    int i = 0;                                      //compteur de tour
    while (min < max)
    {
        temps_initial = clock();
        fct(b, min);                                //lance compute_proof_of_work  
        temps_final = clock();
        temps_cpu = ((double) (temps_final - temps_initial)) / CLOCKS_PER_SEC;

        printf("Test n°%d\n", ++i);                 //printf en stdout le numéro du test
        fprintf(fd, "%d %f\n", min, temps_cpu);     //fprintf dans fd le temps que ça a pris
        min += pas;                                 //on incrémente de pas
    }
    
    //si notre incrémentation dépasse le max (ou est égale à max mais on sort de la boucle), on effectue un dernier test à la valeur max
    temps_initial = clock();
    fct(b, max);
    temps_final = clock();
    temps_cpu = ((double) (temps_final - temps_initial)) / CLOCKS_PER_SEC;
    
    printf("Test n°%d\n", ++i);
    fprintf(fd, "%d %f\n", max, temps_cpu);
    fclose(fd);
}