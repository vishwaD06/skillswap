#include "trie.h"

static int char_to_index(char c) {
    c = (char)tolower((unsigned char)c);
    if (c >= 'a' && c <= 'z') return c - 'a';
    if (c >= '0' && c <= '9') return 26 + (c - '0');
    if (c == ' ') return 36;
    if (c == '+') return 37;
    if (c == '#') return 38;
    if (c == '-') return 39;
    return -1;
}

static TrieNode* create_trie_node(void) {
    TrieNode *node = (TrieNode *)malloc(sizeof(TrieNode));
    if (!node) return NULL;
    node->is_end_of_word = false;
    node->original_name[0] = '\0';
    node->skill_id = 0;
    node->frequency = 0;
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        node->children[i] = NULL;
    }
    return node;
}

Trie* trie_create(void) {
    Trie *trie = (Trie *)malloc(sizeof(Trie));
    if (!trie) return NULL;
    trie->root = create_trie_node();
    trie->total_words = 0;
    return trie;
}

bool trie_insert(Trie *trie, const char *word, int skill_id, int frequency) {
    if (!trie || !trie->root || !word) return false;
    
    TrieNode *curr = trie->root;
    for (int i = 0; word[i]; i++) {
        int idx = char_to_index(word[i]);
        if (idx < 0) continue; // Skip unsupported punctuation safely
        
        if (!curr->children[idx]) {
            curr->children[idx] = create_trie_node();
            if (!curr->children[idx]) return false;
        }
        curr = curr->children[idx];
    }
    
    if (!curr->is_end_of_word) {
        curr->is_end_of_word = true;
        trie->total_words++;
    }
    strncpy(curr->original_name, word, MAX_SKILL_NAME_LEN - 1);
    curr->original_name[MAX_SKILL_NAME_LEN - 1] = '\0';
    curr->skill_id = skill_id;
    curr->frequency = frequency;
    return true;
}

bool trie_search_exact(const Trie *trie, const char *word) {
    if (!trie || !trie->root || !word) return false;
    
    TrieNode *curr = trie->root;
    for (int i = 0; word[i]; i++) {
        int idx = char_to_index(word[i]);
        if (idx < 0) continue;
        if (!curr->children[idx]) return false;
        curr = curr->children[idx];
    }
    return (curr != NULL && curr->is_end_of_word);
}

static void collect_suggestions(TrieNode *node, char results[][MAX_SKILL_NAME_LEN], int *count, int max_results) {
    if (!node || *count >= max_results) return;
    
    if (node->is_end_of_word) {
        // Avoid duplicate strings in results
        bool already_added = false;
        for (int i = 0; i < *count; i++) {
            if (strcasecmp(results[i], node->original_name) == 0) {
                already_added = true;
                break;
            }
        }
        if (!already_added) {
            strncpy(results[*count], node->original_name, MAX_SKILL_NAME_LEN - 1);
            results[*count][MAX_SKILL_NAME_LEN - 1] = '\0';
            (*count)++;
        }
    }
    
    for (int i = 0; i < TRIE_ALPHABET_SIZE && *count < max_results; i++) {
        if (node->children[i]) {
            collect_suggestions(node->children[i], results, count, max_results);
        }
    }
}

int trie_autocomplete(const Trie *trie, const char *prefix, char results[][MAX_SKILL_NAME_LEN], int max_results) {
    if (!trie || !trie->root || !prefix || max_results <= 0) return 0;
    
    TrieNode *curr = trie->root;
    for (int i = 0; prefix[i]; i++) {
        int idx = char_to_index(prefix[i]);
        if (idx < 0) continue;
        if (!curr->children[idx]) {
            return 0; // Prefix does not exist in trie
        }
        curr = curr->children[idx];
    }
    
    int count = 0;
    collect_suggestions(curr, results, &count, max_results);
    return count;
}

static void free_trie_node(TrieNode *node) {
    if (!node) return;
    for (int i = 0; i < TRIE_ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            free_trie_node(node->children[i]);
        }
    }
    free(node);
}

void trie_free(Trie *trie) {
    if (!trie) return;
    if (trie->root) {
        free_trie_node(trie->root);
    }
    free(trie);
}
