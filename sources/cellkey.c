#include <stdlib.h>
#include "headers/cellkey.h"

void mem_reset(char *buff, size_t n) //reset memoire du buffer
{
    for (size_t i = 0; i < n; i++)   //parcours du str buff
        buff[i] = '\0';              //on remplace chaque case par \0 pour réinitialiser
}

CellKey * create_cell_key(Key* key)
{
    CellKey * ck = (CellKey*) malloc(sizeof(CellKey));
    ck->data = key;
    ck->next = NULL;
    return ck;
}

void insert_cell_key (CellKey** ck, Key* c)
{
    CellKey* a = create_cell_key(c);
    if (*ck == NULL)
    {
        *ck = a;
        return;
    }
    a->next = *ck;
    *ck = a;
}

CellKey* read_public_key(char* nomfic)
{
    FILE* fd = fopen(nomfic, "r");
    if (!fd)
    {
        puts("Erreur lors de l'ouverture du fichier.");
        return NULL;
    }

    char ligne[256] = {0};
    char* buff = ligne;
    char public[256];
    char private[256];
    CellKey* ck = NULL;
    
    while (fgets(buff, 256, fd))
    {
        sscanf(buff, "%s %s", public, private);
        insert_cell_key(&ck, str_to_key(public));
        mem_reset(buff, (size_t) 256);
    }

    fclose(fd);
    return ck;
}

void print_list_keys (CellKey* c)
{
    int nb = 0;
    CellKey* temp = c;
    char* data;
    while (temp)
    {
        data = key_to_str(temp->data);
        printf("%s\n", data);
        free(data);
        temp = temp->next;
        nb++;
    }
    printf("Nbr de CellKey : %d\n", nb);
}

void delete_cell_key (CellKey* c)
{
    free(c->data);
    c->next = NULL;
    free(c);
}

void delete_list_ck (CellKey* c)
{
    CellKey* temp = NULL;
    while (c)
    {
        temp = c->next;
        delete_cell_key(c);
        c = temp;
    }
    free(c);
}

int len_cellkey(CellKey* c)
{
    int nb = 0;
    CellKey* temp = c;
    while (temp)
    {
        nb++;
        temp = temp->next;
    }
    return nb;
}