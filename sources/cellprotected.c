#include <stdlib.h>
#include "headers/cellkey.h"
#include "headers/cellprotected.h"

CellProtected* create_cell_protected (Protected* pr)
{
    CellProtected* Cpr = (CellProtected*) malloc(sizeof(CellProtected));
    if (!Cpr)
    {
        puts("Erreur d'allocation");
        return NULL;
    }
    Cpr->data = pr;
    Cpr->next = NULL;
    return Cpr;
}

void insert_cell_protected (CellProtected** cp, CellProtected* pr)
{
    if (*cp == NULL)
    {
        *cp = pr;
        return;
    }
    pr->next = *cp;
    *cp = pr;
}

CellProtected* read_protected(char* nomfic)
{
    FILE* fd = fopen(nomfic, "r");
    if (!fd)
    {
        puts("Erreur lors de l'ouverture du fichier.");
        return NULL;
    }

    char ligne[256] = {0};
    char* buff = ligne;
    size_t n = 256;
    CellProtected* cp = NULL;
    Protected* pr = NULL;
    CellProtected* temp = NULL;

    while (getline(&buff, &n, fd) > 0)
    {
        if(strcmp(buff, "\n") != 0)                 //si on récupère quelque chose
        {
            pr = str_to_protected(buff);
            temp = create_cell_protected(pr);
            insert_cell_protected(&cp, temp);
        }
        mem_reset(buff, n);
    }

    fclose(fd);
    return cp;
}

void print_list_protected (CellProtected* cp)
{
    int nb = 0;
    CellProtected* temp = cp;
    char* data;
    while (temp)
    {
        data = protected_to_str(temp->data);              //on utilise un tampon car protected_to_str renvoie un char* qui a été calloc
        printf("%s", data);
        free(data);                                       //d'où le free
        temp = temp->next;
        nb++;
    }
    printf("Nbr de CellProtected : %d\n", nb);
}

void delete_cell_protected (CellProtected* cp)
{
    freeprotected(cp->data);
    cp->next = NULL;
    free(cp);
}

void delete_list_cp (CellProtected* cp)
{
    CellProtected* temp = NULL;
    while (cp)
    {
        temp = cp->next;
        delete_cell_protected(cp);
        cp = temp;
    }
    free(cp);
}

void suppr_fraude(CellProtected** cp)
{
    CellProtected* prec = *cp;
    
    if (!prec->data || !verify(prec->data))         //si prec->data n'existe pas ou bien prec->data n'est pas valide
    {
        *cp = (*cp)->next;                          
        delete_cell_protected(prec);                //on supprime la cellule frauduleuse
    }
    
    prec = *cp;
    CellProtected* temp = (*cp)->next;

    while (temp)                                    //on fait de même pour tous les autres éléments
    {
        if (!temp->data || !verify(temp->data))
        {
            if (!temp->next)
            {
                delete_cell_protected(temp);
                prec->next = NULL;
            }
            else
            {
                prec->next = temp->next;
                delete_cell_protected(temp);
            }
        }
        prec = prec->next;
        temp = temp->next;
    }
}

int len_cellprotected(CellProtected* cp)            //fonction renvoyant la longueur d'un CellProtected
{
    int nb = 0;
    CellProtected* temp = cp;
    while (temp)
    {
        nb++;
        temp = temp->next;
    }
    return nb;
}