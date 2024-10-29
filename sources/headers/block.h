#include "hash.h"
#include <openssl/sha.h>
#include <err.h>

#ifndef BLOCK_H
#define BLOCK_H

typedef struct block
{
    Key* author;
    CellProtected* votes;
    unsigned char* hash;
    unsigned char* previous_hash;
    int nonce;
}
Block;

void write_block (char* nomfic, Block* bloc);
Block* read_block (char* nomfic);
char* block_to_str(Block* block);
unsigned char* hash_sha (const char* s);
int nb_de_z(unsigned char* str);
void compute_proof_of_work (Block* B, int d);
int verify_block (Block* b, int d);
void delete_block(Block* bloc);
void complete_delete_block (Block* bloc);
void delete_block_wovotes (Block* bloc);
void test3 (void (*fct)(Block*, int), Block* b, int min, int max, int pas);

#endif