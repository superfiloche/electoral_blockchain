#include "headers/vote.h"

void submit_vote(Protected* p)
{
    FILE* fd = fopen("Pending_votes.txt", "a");
    char* temp = protected_to_str(p);
    fprintf(fd, "%s", temp);            //on fprintf le protected_to_str
    free(temp);                         //avant de free
    fclose(fd);
}

void create_block(CellTree* tree, Key* author, int d)
{
    Block* new = calloc(1, sizeof(Block));
    new->votes = NULL;
    if (tree)                                                                           //si l'arbre existe
    {
        CellTree* temp = last_node(tree);                                               //on récupère le hash du dernier élément
        new->previous_hash = (unsigned char*) strdup((const char*) temp->block->hash);  //et on le copie dans notre nouveau bloc
    }
    else
    {
        new->previous_hash = NULL;                                                      //sinon previous est NULL
    }
    CellProtected* cpr = read_protected("Pending_votes.txt");       //on récupère les déclarations dans le fichier
    
    Key* key = calloc(1, sizeof(Key));
    init_key(key, author->val, author->n);                          //on initialise la clé avec celle passée en paramètres
    new->author = key;
    new->votes = cpr;
    
    new->nonce = 0;
    compute_proof_of_work(new, d);                                  //on écrit hash avec compute

    remove("Pending_votes.txt");                                    //on supprime Pending_vote.txt pour ne pas reprendre les anciennes déclarations
    write_block("Pending_block.txt", new);                          //et on écrit le bloc dans Pending_block.txt 

    complete_delete_block(new);                                     //avant de supprimer le bloc qu'on a calloc au début
}

void add_block(int d, char* name)
{
    size_t len = 256;
    Block* new = read_block("Pending_block.txt");                   //on récupère le bloc dans le fichier
    char temp[256] = {0};


    if (verify_block(new, d) == 0)                                  //si compute a marché et que le bloc est valide
    {

        strcpy(temp, "./BlockChain/");
        strcat(temp, name);

        write_block(temp, new);                                     //on écrit ce bloc dans un fichier texte dans le bon répertoire (./BlockChain/)

        mem_reset(temp, len);
    }
    else
    {
        puts("Le bloc n'est pas valide (add_block)");               //sinon, on ne l'ajoute pas
    }

    remove("Pending_block.txt");                                    //on supprime Pending_block.txt
    complete_delete_block(new);                                     //on supprime le bloc qu'on a read
}

CellTree* read_tree()
{
    DIR* rep = opendir("./BlockChain/");
    CellTree** T = NULL;
    CellTree* root = NULL;
    char temp[256] = {0};
    char path[14];
    strcpy(path, "./BlockChain/");
    size_t len = 256;
    if (rep)
    {
        struct dirent* dir;
        int i = 0;
        while ((dir = readdir(rep)))        //on parcourt le répertoire rep (./BlockChain/)
        {   
            if (strcmp(dir->d_name,".") != 0 && strcmp(dir->d_name, "..") != 0) //si le fichier n'est pas le répertoire . ni .., alors on rentre dans la boucle
            {
                if (++i == 1)               //si on est au premier fichier
                {
                    T = calloc(1, sizeof(CellTree*));     //on calloc un tableau de CellTree
                }
                else
                {
                    T = realloc(T, i * sizeof(CellTree*));  //sinon on realloc la taille du du tableau
                }
                strcpy(temp, path);
                strcat(temp, dir->d_name);
                T[i - 1] = create_node(read_block(temp));   //on create_node le bloc du fichier à sa place
                mem_reset(temp, len);                       //on réinitialise le tampon
            }
        }

        for (int j = 0; j < i; j++)
        {
            for (int k = 0; k < i; k++)
            {
                if (k != j)
                {
                    if (strcmp((const char*) (T[k]->block)->previous_hash, (const char*) (T[j]->block)->hash) == 0)
                    {
                        add_child(T[j], T[k]);  //si on a correspondance entre previous_hash d'un bloc et hash d'un autre, on add_child dans le bon sens
                    }
                } 
            }
        }
        
        for (int j = 0; j < i; j++)
        {
            if (T[j]->father == NULL)
                root = T[j];
        }
    }

    closedir(rep);      //on ferme pour éviter les leaks
    free(T);            //on free le tableau
    return root;
}

Key* compute_winner_BT (CellTree* tree, CellKey* candidates, CellKey* voters, int sizeC, int sizeV)
{
    CellTree* root = find_root(tree);                                       //on récupère la racine de l'arbre au cas où on y serait pas
    CellProtected* decl = fuse_cp_block(root);                              //on récupère la liste chaînée de déclaration de la branche la plus longue de l'arbre en paramètres
    suppr_fraude(&decl);                                                    //on supprime les fraudes
    Key* winner = compute_winner(decl, candidates, voters, sizeC, sizeV);   //on compute_winner pour récupérer la clé du vainqueur
    delete_list_cp(decl);                                                   //on supprime la liste chaînée
    return winner;
}