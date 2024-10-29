#include "headers/tree.h"

CellTree* create_node(Block* b)
{
    CellTree* nct = (CellTree*) calloc(1, sizeof(CellTree));
    if(!nct)
    {
        errx(1, "Erreur d allocation de la memoire : create_node()");
    }
    nct->block = b;
    nct->father = NULL;
    nct->firstChild = NULL;
    nct->nextBro = NULL;
    nct->height = 0;
    return nct;
}

CellTree* find_root(CellTree* ct) //fonction renvoyant un pointeur vers la racine de l'arbre
{
    if (!ct)
        errx(2, "Erreur de paramètre (CellTree NULL dans find_root)");

    if (!ct->father)
        return ct;
    
    CellTree *root = ct;
    while(root->father)
    {//si un noeud n'a pas de father, alors c'est la racine de l'arbre
        root = root->father;
    }

    return root;
}

int update_height(CellTree* father, CellTree* child)
{ 
    if(father->height <= child->height)
    {
        father->height = child->height + 1;
        return 1;
    }
    
    //La hauteur n a pas été modifiée, on renvoie 0
    return 0;
}

/*void update_height2(CellTree* cell)       inutile, update_height à l'envers qui met à jour quand on supprime un node
{
    int height = cell->height;
    cell->height = (highest_child(cell)->height + 1);
    if (height != cell->height)
        puts("Hauteur modifiée");
}*/

void add_child(CellTree* father, CellTree* child)
{
    if(!father->firstChild)
    {
        father->firstChild = child;             //si le père n'a pas de fils, il devient le firstchild
    }
    else
    {
        CellTree* ct_tmp = father->firstChild;  //si le père a au moins un fils, on parcourt ses fils et on se place en dernière position
        while (ct_tmp->nextBro)
            ct_tmp = ct_tmp->nextBro;           
        ct_tmp->nextBro = child;                //on l'ajoute comme frère
    }
    child->father = father;
    CellTree* ct1 = father;                     //ensuite on met à jour la hauteur de tous les ascendants
    CellTree* ct2 = child;
    while (ct1)
    {
        if (update_height(ct1, ct2))
        ct2 = ct1;
        ct1 = ct1->father;
    }
}

void print_tree(CellTree* ct)
{
    if (ct)
    {
        printf("Hauteur du noeud : %d et valeur hashé du bloc : %s\n", ct->height, ct->block->hash);
        if (ct->firstChild)
            print_tree(ct->firstChild);
         if (ct->nextBro)
            print_tree(ct->nextBro);
    } 
}

/*
void delete_node(CellTree* node)        inutile, delete_node trop compliqué qui cherchait à update_height2 à l'envers
{
    CellTree* father = node->father;
    CellTree* child = node->firstChild;
    CellTree* bro = node->nextBro;
    CellTree* temp = NULL;
    if (father)
    {
        if (father->firstChild == node)
        {
            if (child)
            {
                father->firstChild = child;
            }
            else
            {
                father->firstChild = bro;
            }
        }
        else
        {
            temp = father->firstChild;
            while (temp->nextBro != node)
                temp = temp->nextBro;
            
            if (bro)
            {
                temp->nextBro = bro;
            }
            else
            {
                temp->nextBro = NULL;
            }
        }
        
        update_height2(temp);
        while(temp->father)
        {
            temp = temp->father;
            update_height2(temp);
        }
    }
    else
    {
        temp = child;
        while (temp)
        {
            temp->father = NULL;
            temp = temp->nextBro;
        }
    }
    delete_block(node->block);
    node->father = NULL;
    node->firstChild = NULL;
    node->nextBro = NULL;
    free(node);
}
*/

void delete_node(CellTree *node) 
{
  complete_delete_block(node->block);
  node->father = NULL;
  node->firstChild = NULL;
  node->nextBro = NULL;
  free(node);
}

void delete_tree (CellTree* tree) //ne marche qui si on est à la racine
{
    if(tree->firstChild)
        delete_tree(tree->firstChild);
    if (tree->nextBro)
        delete_tree(tree->nextBro);
    delete_node(tree);
}

CellTree* highest_child(CellTree* cell)
{
    if (!cell)
        errx(2, "Erreur de paramètre (CellTree NULL dans highest_child)");
    if (!cell->firstChild)
    {   
        puts("cell n'a pas fils");      //s'il n'y pas de fils, on return NULL
        return NULL;
    }

    CellTree* max = cell->firstChild;   //sinon on récupère le firstChild
    CellTree* temp = max;
    max = temp;
    while (temp->nextBro)               //parcourt les nextBro tant que temp->nextBro existe
    {
        temp = temp->nextBro;
        if (temp->height > max->height) //si on trouve une hauteur supérieur à max->height, on décale le pointeur max
            max = temp;
    }
    return max;
}

CellTree* last_node (CellTree* tree)
{
    if (!tree)
        errx(2, "Erreur de paramètre (CellTree NULL dans last_node)");

    CellTree* temp = tree;
    while (temp->firstChild)
    {
        temp = highest_child(temp); //on récupère le highestchild tant que le firstchild du CellTree existe
    }
    return temp;
}

void fuse_cp (CellProtected** first, CellProtected** second) //fusion de deux listes chaînées de déclarations signées
{                                                            //on utilise les adresses au lieu de pointeur
    if (!*first)                    //si le premier est vide
    {
        *first = *second;           //on assigne le premier CellProtected de second à first
        *second = NULL;
        return;
    }
    CellProtected* temp = *first;   
    while (temp->next)
    {
        temp = temp->next;          //sinon on arrive au dernier CellProtected de first
    }
    temp->next = *second;           //et on lui assigne le premier élément de second
    *second = NULL;                 //puis on met second à NULL
} //il suffit d'avoir un pointeur en plus sur le dernier élément de la liste chaînée

CellProtected* fuse_cp_block (CellTree* tree)
{
    if (!tree)
        errx(2, "Erreur de paramètre (CellTree NULL dans fuse_cp_block)");
    if (!tree->block->votes)
        errx(2, "Erreur de paramètre (pas de liste chaînée dans ce bloc dans fuse_cp_block)");

    CellProtected* res = NULL;
    fuse_cp(&res, &tree->block->votes);
    CellTree* temp = tree;
    
    while (temp->firstChild)
    {
        temp = highest_child(temp);              
        fuse_cp(&res, &temp->block->votes);     //on prend que la plus longue branche grâce aux highestchild
    }

    return res;
}