#include "block.h"

#ifndef TREE_H
#define TREE_H

typedef struct block_tree_cell
{
    Block* block;
    struct block_tree_cell* father;
    struct block_tree_cell* firstChild;
    struct block_tree_cell* nextBro;
    int height;
}
CellTree;

CellTree* create_node(Block* b);
CellTree* find_root(CellTree* ct);
int update_height(CellTree* father, CellTree* child);
//void update_height2(CellTree* cell);
void add_child(CellTree* father, CellTree* child);
void print_tree(CellTree* ct);
void delete_node(CellTree* node);
void delete_tree (CellTree* tree);
CellTree* highest_child (CellTree* cell);
CellTree* last_node (CellTree* tree);
void fuse_cp (CellProtected** first, CellProtected** second);
CellProtected* fuse_cp_block (CellTree* tree);

#endif