#include "bst.h"

void bst_init(BST *tree) {
    if (!tree) return;
    tree->root = NULL;
    tree->count = 0;
}

static BSTNode* create_node(float key, int student_id, const char *name, const char *skill) {
    BSTNode *node = (BSTNode *)malloc(sizeof(BSTNode));
    if (!node) return NULL;
    node->key = key;
    node->student_id = student_id;
    strncpy(node->student_name, name ? name : "", MAX_NAME_LEN - 1);
    node->student_name[MAX_NAME_LEN - 1] = '\0';
    strncpy(node->primary_skill, skill ? skill : "", MAX_SKILL_NAME_LEN - 1);
    node->primary_skill[MAX_SKILL_NAME_LEN - 1] = '\0';
    node->left = NULL;
    node->right = NULL;
    return node;
}

BSTNode* bst_insert(BSTNode *root, float key, int student_id, const char *name, const char *skill) {
    if (!root) {
        return create_node(key, student_id, name, skill);
    }
    if (key < root->key) {
        root->left = bst_insert(root->left, key, student_id, name, skill);
    } else {
        root->right = bst_insert(root->right, key, student_id, name, skill);
    }
    return root;
}

bool bst_add(BST *tree, float key, int student_id, const char *name, const char *skill) {
    if (!tree) return false;
    tree->root = bst_insert(tree->root, key, student_id, name, skill);
    if (tree->root) {
        tree->count++;
        return true;
    }
    return false;
}

BSTNode* bst_search(BSTNode *root, float key) {
    if (!root || (root->key >= key - 0.001f && root->key <= key + 0.001f)) {
        return root;
    }
    if (key < root->key) {
        return bst_search(root->left, key);
    }
    return bst_search(root->right, key);
}

int bst_inorder(const BSTNode *root, BSTNode **out_nodes, int max_nodes, int *count) {
    if (!root || *count >= max_nodes) return *count;
    bst_inorder(root->left, out_nodes, max_nodes, count);
    if (*count < max_nodes) {
        out_nodes[(*count)++] = (BSTNode *)root;
    }
    bst_inorder(root->right, out_nodes, max_nodes, count);
    return *count;
}

int bst_preorder(const BSTNode *root, BSTNode **out_nodes, int max_nodes, int *count) {
    if (!root || *count >= max_nodes) return *count;
    if (*count < max_nodes) {
        out_nodes[(*count)++] = (BSTNode *)root;
    }
    bst_preorder(root->left, out_nodes, max_nodes, count);
    bst_preorder(root->right, out_nodes, max_nodes, count);
    return *count;
}

int bst_postorder(const BSTNode *root, BSTNode **out_nodes, int max_nodes, int *count) {
    if (!root || *count >= max_nodes) return *count;
    bst_postorder(root->left, out_nodes, max_nodes, count);
    bst_postorder(root->right, out_nodes, max_nodes, count);
    if (*count < max_nodes) {
        out_nodes[(*count)++] = (BSTNode *)root;
    }
    return *count;
}

BSTNode* bst_find_min(BSTNode *root) {
    if (!root) return NULL;
    while (root->left) {
        root = root->left;
    }
    return root;
}

BSTNode* bst_find_max(BSTNode *root) {
    if (!root) return NULL;
    while (root->right) {
        root = root->right;
    }
    return root;
}

void bst_free_node(BSTNode *root) {
    if (!root) return;
    bst_free_node(root->left);
    bst_free_node(root->right);
    free(root);
}

void bst_free(BST *tree) {
    if (!tree) return;
    bst_free_node(tree->root);
    tree->root = NULL;
    tree->count = 0;
}
