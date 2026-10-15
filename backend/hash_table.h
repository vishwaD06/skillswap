#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "common.h"

#define HT_DEFAULT_CAPACITY 64

typedef struct HTEntry {
    char key[64];
    void *value;
    struct HTEntry *next;
} HTEntry;

typedef struct {
    HTEntry **buckets;
    int capacity;
    int count;
    int collisions;
} HashTable;

HashTable* ht_create(int capacity);
unsigned int ht_hash(const char *key, int capacity);
bool ht_insert(HashTable *ht, const char *key, void *value);
void* ht_search(const HashTable *ht, const char *key);
bool ht_delete(HashTable *ht, const char *key);
void ht_get_stats(const HashTable *ht, int *count, int *capacity, int *collisions, float *load_factor);
void ht_free(HashTable *ht, bool free_values);

#endif // HASH_TABLE_H
