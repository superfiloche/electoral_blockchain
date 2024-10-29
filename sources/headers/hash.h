#include "cellprotected.h"
#include "cellkey.h"

#ifndef HASH_CELL
#define HASH_CELL

typedef struct hashcell
{
    Key* key;
    int val;
}
HashCell;

typedef struct hashtable
{
    HashCell** tab;
    int size;
}
HashTable;

HashCell* create_hashcell (Key* key);
int hash_function (Key* key, int size);
int find_position (HashTable* t, Key* key);
HashTable* create_hashtable (CellKey* keys, int size);
void print_hashtable(HashTable* t);
void delete_hashtable(HashTable* t);
Key* compute_winner(CellProtected* decl, CellKey* candidates, CellKey* voters, int sizeC, int sizeV);

#endif