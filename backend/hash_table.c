#include "hash_table.h"

HashTable* ht_create(int capacity) {
    if (capacity <= 0) capacity = HT_DEFAULT_CAPACITY;
    HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
    if (!ht) return NULL;
    
    ht->capacity = capacity;
    ht->count = 0;
    ht->collisions = 0;
    ht->buckets = (HTEntry **)calloc(capacity, sizeof(HTEntry *));
    if (!ht->buckets) {
        free(ht);
        return NULL;
    }
    return ht;
}

// DJB2 Hash Algorithm: hash = ((hash << 5) + hash) + c
unsigned int ht_hash(const char *key, int capacity) {
    if (!key || capacity <= 0) return 0;
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*key++)) {
        hash = ((hash << 5) + hash) + tolower(c); // Case-insensitive
    }
    return (unsigned int)(hash % (unsigned long)capacity);
}

bool ht_insert(HashTable *ht, const char *key, void *value) {
    if (!ht || !key) return false;
    unsigned int idx = ht_hash(key, ht->capacity);
    HTEntry *curr = ht->buckets[idx];
    
    // Check if key already exists, update value
    while (curr) {
        if (strcasecmp(curr->key, key) == 0) {
            curr->value = value;
            return true;
        }
        curr = curr->next;
    }
    
    // If bucket already had entries, record collision
    if (ht->buckets[idx] != NULL) {
        ht->collisions++;
    }
    
    // Allocate new node and insert at head of bucket chain
    HTEntry *new_node = (HTEntry *)malloc(sizeof(HTEntry));
    if (!new_node) return false;
    strncpy(new_node->key, key, sizeof(new_node->key) - 1);
    new_node->key[sizeof(new_node->key) - 1] = '\0';
    new_node->value = value;
    new_node->next = ht->buckets[idx];
    ht->buckets[idx] = new_node;
    ht->count++;
    return true;
}

void* ht_search(const HashTable *ht, const char *key) {
    if (!ht || !key) return NULL;
    unsigned int idx = ht_hash(key, ht->capacity);
    HTEntry *curr = ht->buckets[idx];
    while (curr) {
        if (strcasecmp(curr->key, key) == 0) {
            return curr->value;
        }
        curr = curr->next;
    }
    return NULL;
}

bool ht_delete(HashTable *ht, const char *key) {
    if (!ht || !key) return false;
    unsigned int idx = ht_hash(key, ht->capacity);
    HTEntry *curr = ht->buckets[idx];
    HTEntry *prev = NULL;
    
    while (curr) {
        if (strcasecmp(curr->key, key) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                ht->buckets[idx] = curr->next;
            }
            free(curr);
            ht->count--;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

void ht_get_stats(const HashTable *ht, int *count, int *capacity, int *collisions, float *load_factor) {
    if (!ht) return;
    if (count) *count = ht->count;
    if (capacity) *capacity = ht->capacity;
    if (collisions) *collisions = ht->collisions;
    if (load_factor) *load_factor = (float)ht->count / (float)ht->capacity;
}

void ht_free(HashTable *ht, bool free_values) {
    if (!ht) return;
    for (int i = 0; i < ht->capacity; i++) {
        HTEntry *curr = ht->buckets[i];
        while (curr) {
            HTEntry *temp = curr;
            curr = curr->next;
            if (free_values && temp->value) {
                free(temp->value);
            }
            free(temp);
        }
    }
    free(ht->buckets);
    free(ht);
}
