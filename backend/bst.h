#ifndef BST_H
#define BST_H

#include "common.h"

typedef struct BSTNode {
    float key;                 // Rating or score
    int student_id;
    char student_name[MAX_NAME_LEN];
    char primary_skill[MAX_SKILL_NAME_LEN];
    struct BSTNode *left;
    struct BSTNode *right;
} BSTNode;

typedef struct {
    BSTNode *root;
    int count;
} BST;

void bst_init(BST *tree);
BSTNode* bst_insert(BSTNode *root, float key, int student_id, const char *name, const char *skill);
bool bst_add(BST *tree, float key, int student_id, const char *name, const char *skill);
BSTNode* bst_search(BSTNode *root, float key);
int bst_inorder(const BSTNode *root, BSTNode **out_nodes, int max_nodes, int *count);
int bst_preorder(const BSTNode *root, BSTNode **out_nodes, int max_nodes, int *count);
int bst_postorder(const BSTNode *root, BSTNode **out_nodes, int max_nodes, int *count);
BSTNode* bst_find_min(BSTNode *root);
BSTNode* bst_find_max(BSTNode *root);
void bst_free_node(BSTNode *root);
void bst_free(BST *tree);

#endif // BST_H
