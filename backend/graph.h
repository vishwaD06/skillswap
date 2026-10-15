#ifndef GRAPH_H
#define GRAPH_H

#include "common.h"

#define MAX_GRAPH_VERTICES 256

typedef struct GraphEdge {
    int dest;                  // Target student ID
    int weight;                // e.g. number of shared sessions / exchange affinity
    char skill_exchanged[MAX_SKILL_NAME_LEN];
    struct GraphEdge *next;
} GraphEdge;

typedef struct {
    int student_id;
    char name[MAX_NAME_LEN];
    char dept[MAX_DEPT_LEN];
    GraphEdge *head;
} Vertex;

typedef struct {
    int num_vertices;
    Vertex vertices[MAX_GRAPH_VERTICES];
} Graph;

void graph_init(Graph *g);
int graph_find_or_add_vertex(Graph *g, int student_id, const char *name, const char *dept);
bool graph_add_edge(Graph *g, int src_id, int dest_id, int weight, const char *skill_exchanged);
int graph_bfs(const Graph *g, int start_id, int *visited_ids, int *distances, int max_results);
int graph_dfs(const Graph *g, int start_id, int *visited_ids, int max_results);
void graph_free(Graph *g);

#endif // GRAPH_H
