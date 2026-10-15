#ifndef SERVER_H
#define SERVER_H

#include "common.h"
#include "student.h"
#include "skill.h"
#include "linked_list.h"
#include "hash_table.h"
#include "queue.h"
#include "stack.h"
#include "graph.h"
#include "bst.h"
#include "heap.h"
#include "trie.h"
#include "dynarray.h"
#include "sorting.h"
#include "matching.h"
#include "storage.h"

typedef struct {
    int port;
    const char *static_dir;
    bool running;
} ServerConfig;

void server_init(int port, const char *static_dir);
int  server_run(void);
void server_stop(void);

#endif // SERVER_H
