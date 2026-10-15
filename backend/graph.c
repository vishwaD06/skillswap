#include "graph.h"

void graph_init(Graph *g) {
    if (!g) return;
    g->num_vertices = 0;
    for (int i = 0; i < MAX_GRAPH_VERTICES; i++) {
        g->vertices[i].student_id = -1;
        g->vertices[i].head = NULL;
        g->vertices[i].name[0] = '\0';
        g->vertices[i].dept[0] = '\0';
    }
}

int graph_find_or_add_vertex(Graph *g, int student_id, const char *name, const char *dept) {
    if (!g || student_id < 0) return -1;
    // Check if exists
    for (int i = 0; i < g->num_vertices; i++) {
        if (g->vertices[i].student_id == student_id) {
            if (name && strlen(name) > 0) strncpy(g->vertices[i].name, name, MAX_NAME_LEN - 1);
            if (dept && strlen(dept) > 0) strncpy(g->vertices[i].dept, dept, MAX_DEPT_LEN - 1);
            return i;
        }
    }
    // Add new
    if (g->num_vertices >= MAX_GRAPH_VERTICES) return -1;
    int idx = g->num_vertices++;
    g->vertices[idx].student_id = student_id;
    strncpy(g->vertices[idx].name, name ? name : "Student", MAX_NAME_LEN - 1);
    strncpy(g->vertices[idx].dept, dept ? dept : "Engineering", MAX_DEPT_LEN - 1);
    g->vertices[idx].head = NULL;
    return idx;
}

bool graph_add_edge(Graph *g, int src_id, int dest_id, int weight, const char *skill_exchanged) {
    if (!g || src_id == dest_id) return false;
    
    int u = graph_find_or_add_vertex(g, src_id, NULL, NULL);
    int v = graph_find_or_add_vertex(g, dest_id, NULL, NULL);
    if (u < 0 || v < 0) return false;
    
    // Check if edge already exists from u to v
    GraphEdge *curr = g->vertices[u].head;
    while (curr) {
        if (curr->dest == dest_id) {
            curr->weight += weight;
            return true;
        }
        curr = curr->next;
    }
    
    // Add directed edge u -> v
    GraphEdge *edge = (GraphEdge *)malloc(sizeof(GraphEdge));
    if (!edge) return false;
    edge->dest = dest_id;
    edge->weight = weight;
    strncpy(edge->skill_exchanged, skill_exchanged ? skill_exchanged : "General", MAX_SKILL_NAME_LEN - 1);
    edge->next = g->vertices[u].head;
    g->vertices[u].head = edge;
    
    return true;
}

// Breadth-First Search (BFS) to discover reachable students level by level
int graph_bfs(const Graph *g, int start_id, int *visited_ids, int *distances, int max_results) {
    if (!g || max_results <= 0) return 0;
    
    int start_idx = -1;
    for (int i = 0; i < g->num_vertices; i++) {
        if (g->vertices[i].student_id == start_id) {
            start_idx = i;
            break;
        }
    }
    if (start_idx < 0) return 0;
    
    bool visited[MAX_GRAPH_VERTICES] = { false };
    int dist[MAX_GRAPH_VERTICES] = { 0 };
    int queue[MAX_GRAPH_VERTICES];
    int front = 0, rear = 0;
    
    visited[start_idx] = true;
    dist[start_idx] = 0;
    queue[rear++] = start_idx;
    
    int count = 0;
    
    while (front < rear && count < max_results) {
        int u = queue[front++];
        
        // Add to result (exclude starting node if desired, or include it)
        visited_ids[count] = g->vertices[u].student_id;
        if (distances) distances[count] = dist[u];
        count++;
        
        // Traverse adjacency list
        GraphEdge *curr = g->vertices[u].head;
        while (curr) {
            // Find vertex index for dest
            for (int v = 0; v < g->num_vertices; v++) {
                if (g->vertices[v].student_id == curr->dest) {
                    if (!visited[v]) {
                        visited[v] = true;
                        dist[v] = dist[u] + 1;
                        if (rear < MAX_GRAPH_VERTICES) {
                            queue[rear++] = v;
                        }
                    }
                    break;
                }
            }
            curr = curr->next;
        }
    }
    return count;
}

static void dfs_util(const Graph *g, int u, bool *visited, int *visited_ids, int *count, int max_results) {
    if (*count >= max_results || visited[u]) return;
    
    visited[u] = true;
    visited_ids[(*count)++] = g->vertices[u].student_id;
    
    GraphEdge *curr = g->vertices[u].head;
    while (curr && *count < max_results) {
        for (int v = 0; v < g->num_vertices; v++) {
            if (g->vertices[v].student_id == curr->dest) {
                if (!visited[v]) {
                    dfs_util(g, v, visited, visited_ids, count, max_results);
                }
                break;
            }
        }
        curr = curr->next;
    }
}

// Depth-First Search (DFS) to explore student learning clusters
int graph_dfs(const Graph *g, int start_id, int *visited_ids, int max_results) {
    if (!g || max_results <= 0) return 0;
    
    int start_idx = -1;
    for (int i = 0; i < g->num_vertices; i++) {
        if (g->vertices[i].student_id == start_id) {
            start_idx = i;
            break;
        }
    }
    if (start_idx < 0) return 0;
    
    bool visited[MAX_GRAPH_VERTICES] = { false };
    int count = 0;
    dfs_util(g, start_idx, visited, visited_ids, &count, max_results);
    return count;
}

void graph_free(Graph *g) {
    if (!g) return;
    for (int i = 0; i < g->num_vertices; i++) {
        GraphEdge *curr = g->vertices[i].head;
        while (curr) {
            GraphEdge *next = curr->next;
            free(curr);
            curr = next;
        }
        g->vertices[i].head = NULL;
    }
    g->num_vertices = 0;
}
