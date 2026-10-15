#ifndef TRIE_H
#define TRIE_H

#include "common.h"

#define TRIE_ALPHABET_SIZE 40
#define MAX_AUTOCOMPLETE_RESULTS 10

typedef struct TrieNode {
    bool is_end_of_word;
    char original_name[MAX_SKILL_NAME_LEN];
    int skill_id;
    int frequency; // Popularity weighting
    struct TrieNode *children[TRIE_ALPHABET_SIZE];
} TrieNode;

typedef struct {
    TrieNode *root;
    int total_words;
} Trie;

Trie* trie_create(void);
bool trie_insert(Trie *trie, const char *word, int skill_id, int frequency);
bool trie_search_exact(const Trie *trie, const char *word);
int trie_autocomplete(const Trie *trie, const char *prefix, char results[][MAX_SKILL_NAME_LEN], int max_results);
void trie_free(Trie *trie);

#endif // TRIE_H
