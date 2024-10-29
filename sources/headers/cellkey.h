#include <stdlib.h>
#include "key.h"

#ifndef CELL_KEY
#define CELL_KEY

typedef struct cellKey
{
    Key* data;
    struct cellKey* next;
}
CellKey;

void mem_reset(char *buff, size_t n);

CellKey* create_cell_key (Key* key);
void insert_cell_key (CellKey** c, Key* key);
CellKey* read_public_key (char* nomfic);
void print_list_keys (CellKey* c);
void delete_cell_key (CellKey* c);
void delete_list_ck (CellKey* c);
int len_cellkey(CellKey* c);

#endif