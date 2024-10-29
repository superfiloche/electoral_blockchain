#include <math.h>
#include "headers/cellkey.h"
#include "headers/cellprotected.h"
#include "headers/hash.h"
#include "err.h"
/*
On include err.h pour pouvoir avoir une meilleure gestion des erreurs
Le programme sera interrompu, il y aura des fuites mais nous avons un message clair
et tout de suite reconnaissable (car personnalisé)
Cela évite de laisser le programme continuer avec des pointeurs NULL qui risquent
de provoquer des Segmentation faults
*/

HashCell* create_hashcell(Key* key)
{
    HashCell* nhc = (HashCell*) calloc(1, sizeof(HashCell));
    if(!nhc)
    {
        errx(1, "Erreur d'allocation (create_hashcell)");
    }
    nhc->key = calloc(1, sizeof(Key));
    init_key(nhc->key, key->val, key->n);
    return nhc;
}

int hash_function(Key* key, int size)
{
    if(!key)
    {
        puts("Erreur de parametre (!key dans hash_function)");
        return -1;
    }
    float nbo = (sqrtf(5)-1)/2; //on utilise le nombre d'or diminué de 1
    return ((int) floor(size * (key->val * nbo - floor(key->val * nbo))));
}

int find_position(HashTable* ht, Key* key)
{
    if (!ht || !key)
        errx(2, "Erreur de parametre (find_position)");
    int i = hash_function(key, ht->size);
    if (ht->tab[i])
    {
        if (ht->tab[i]->key) //s'il existe une clé à l'indice de la position supposée :
        {
            if ((ht->tab[i]->key->val == key->val) && (ht->tab[i]->key->n == key->n))
                return i;   //si l'élément est à sa place, on return l'indice
            
            else    //sinon on va à la case suivante modulo la taille du tableau pour ne pas dépasser la taille du tableau
            {
                int k = 1;
                int nb = (i + k) % ht->size;

                while (ht->tab[nb] && (k % ht->size))
                {
                    if (ht->tab[nb]->key->val == key->val && (ht->tab[nb]->key->n == key->n))
                        return nb;  //si on retoruve l'élément, on return l'indice
                    k++;    //on incrémente pour bouger d'une case
                    nb = (i + k) % ht->size;
                }
                if (k % ht->size == 0)  //on effectue cette vérification au cas où on sort de la boucle while car !ht->tab[nb]
                    return -1;   //si cette condition est vérifiée, cela veut dire que l'élément n'est pas dans le tableau
                                //car il ne se trouve dans aucune des cases où il peut être (probing linéaire), on renvoie donc sa position supposée
                
                return nb;  //s'il n'y a pas d'élément dans l'une des cases où il peut être, on renvoie l'indice de cette case pour pouvoir l'y insérer
                            //sachant que les cases précédentes sont toutes occupées
            }
        }
    }
    return i;   //si sa position supposée est vide, on renvoie l'indice directement
}

HashTable* create_hashtable(CellKey* keys, int size)
{
    HashTable* ht = calloc(1, sizeof(HashTable));
    if (!ht)
        errx(1, "Erreur d'allocation (create_hashtable ht)");
    ht->tab = calloc(size, sizeof(HashCell*));
    if (!ht->tab)
    {
        free(ht);   
        errx(1, "Erreur d'allocation (create_hashtable ht->tab)");
    }
    ht->size = size;
    
    if (len_cellkey(keys) > size)
    {
        puts("Attention, le tableau de hashage n'est pas assez grand pour toutes les clés.");
    }
    
    CellKey* temp = keys;
    int i = 0;
    while (temp)
    {
        i = find_position(ht, temp->data);
        if (i != -1 || i >= size)       //si find_position a trouvé une position
        {
            if (ht->tab[i] == NULL)     //s'il n'y a pas de clé à cette position
                ht->tab[i] = create_hashcell(temp->data);
            else if ((ht->tab[i]->key->val != temp->data->val) || (ht->tab[i]->key->n != temp->data->n))    //si ce n'est pas la même clé alors notre find_position a fauté
                puts("Une clé différente existe déjà, le vote ne sera pas pris en compte.\nC'est un problème de collision qui n'a pas été géré par notre fonction find_position (i != -1 alors qu'on a une clé différente).");
            else
                puts("La clé de ce citoyen a déjà été ajoutée à la table de hashage.\n");       //sinon c'est la même clé donc on ne fait rien
        }
        temp = temp->next;
    }
    return ht;
}

void print_hashtable(HashTable* t)
{
    if (!t)
    {
        puts("Hashtable inexistante (print_hashtable)");
        return;
    }
    printf("Taille du tableau : %d\n", t->size);
    char* temp;
    for (int i = 0; i < t->size; i++)
    {
        if (t->tab[i])
        {
            if (t->tab[i]->key)
            {
                temp = key_to_str(t->tab[i]->key);
                if (!temp) errx(3, "Erreur contenu NULL (print_hashtable)");
                printf("%s\n", temp);
                free(temp);
            }
        }
    }
}

void delete_hashtable(HashTable* t)
{
    if(!t)
    {
        puts("Hashtable inexistante (delete_hashtable)");
        return;
    }
    for (int i = 0; i < t->size; i++)
    {
        if (t->tab[i])
        {
            free(t->tab[i]->key); // on free les cle cases par cases
            free(t->tab[i]);
        }
    }
    free(t->tab);
    free(t); // t est un tableau contenant maintenant que des entiers
}


Key *compute_winner(CellProtected *decl, CellKey *candidates, CellKey *voters, int sizeC, int sizeV)
{
    HashTable *hc = create_hashtable(candidates, sizeC);
    HashTable *hv = create_hashtable(voters, sizeV);

    int majorite;
    if (sizeV % 2 == 0)
    {
        majorite = sizeV / 2;
    }
    else
    {
        majorite = ceil(sizeV / 2);
    }

    CellProtected* temp = decl;
    Key *ktemp;
    int val;
    int pos;
    while (temp) //parcours de la liste chaînée des déclarations
    {
        ktemp = temp->data->pubk;
        pos = find_position(hv, ktemp);
        if (pos == -1)      //s'il n'y a plus de place dans le hashtable, on passe au suivant
        {
            temp = temp->next;
            continue;
        }
        val = hv->tab[pos]->val;   
        val++;
        if (val > 1)
        {
            // Ce joueur a deja voté, on continue sans prendre en compte son vote
            continue;
        }
        ktemp = str_to_key(temp->data->mess);   // On retrouve la cle dans hc grâce à une clé temporaire
        pos = find_position(hc, ktemp);
        if (pos == -1)                          //si on ne trouve pas le candidat associé
        {
            free(ktemp);                        //on free la clé
            temp = temp->next;                  //on passe au suivant
            continue;
        }
        hc->tab[pos]->val++;                // on incrémente sa valeur pour avoir le nombre de votes
        free(ktemp);                        // on free la cle temporaire pour eviter les fuites
        temp = temp->next;                  // on passe a la suite
    }

    int index;
    int maxval = 0;
    Key* winner = calloc(1, sizeof(Key));   //on calloc une nouvelle clé qui va stocker la clé du gagnant
    for (int i = 0; i < sizeC; i++)
    {
        if (hc->tab[i]->val > majorite)
        { // Si le candidat a obtenu la majorite, pas besoin de continuer!
            printf("Victoire à la majorité pour %d, vive la démocratie : %d voteurs au total pour %d votes\n", i, sizeV,  hc->tab[i]->val);
            
            init_key(winner, hc->tab[i]->key->val, hc->tab[i]->key->n);

            delete_hashtable(hc);
            delete_hashtable(hv);
            return winner;
        }
        if (hc->tab[i]->val > maxval)
        {
            maxval = hc->tab[i]->val;
            index = i;
        }
    }
    printf("%d a eu %d votes pour %d voteurs au total\n", index, hc->tab[index]->val, sizeV);
   
    init_key(winner, hc->tab[index]->key->val, hc->tab[index]->key->n); //on crée un clé à renvoyer
    
    delete_hashtable(hc);
    delete_hashtable(hv);
    return winner;
}
