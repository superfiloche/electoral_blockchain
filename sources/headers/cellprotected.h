#include <stdlib.h>
#include "key.h"
#include "signature.h"

#ifndef CELL_PROTECTED
#define CELL_PROTECTED

typedef struct cellProtected
{
    Protected* data;
    struct cellProtected* next;
}
CellProtected;

CellProtected* create_cell_protected (Protected* pr);
void insert_cell_protected (CellProtected** cp, CellProtected* pr);
CellProtected* read_protected (char* nomfic);
void print_list_protected (CellProtected* cp);
void delete_cell_protected (CellProtected* cp);
void delete_list_cp (CellProtected* cp);
void suppr_fraude(CellProtected** cp);
int len_cellprotected(CellProtected* cp);

#endif