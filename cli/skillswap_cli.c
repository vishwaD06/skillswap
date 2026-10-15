#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../backend/common.h"
#include "../backend/student.h"
#include "../backend/skill.h"
#include "../backend/linked_list.h"
#include "../backend/hash_table.h"
#include "../backend/queue.h"
#include "../backend/stack.h"
#include "../backend/graph.h"
#include "../backend/bst.h"
#include "../backend/heap.h"
#include "../backend/trie.h"
#include "../backend/dynarray.h"
#include "../backend/sorting.h"
#include "../backend/matching.h"
#include "../backend/storage.h"

static Student g_students[MAX_STUDENTS];
static int g_num_students = 0;
static Skill g_skills[MAX_SKILLS];
static int g_num_skills = 0;
static Trie *g_trie = NULL;
static Graph g_graph;
static Queue g_queue;
static Stack g_stack;

void init_system(void) {
    storage_set_dir("data");
    g_num_students = storage_load_students(g_students, MAX_STUDENTS);
    g_num_skills = storage_load_skills(g_skills, MAX_SKILLS);
    
    // Initialize Trie
    g_trie = trie_create();
    for (int i = 0; i < g_num_skills; i++) {
        trie_insert(g_trie, g_skills[i].name, g_skills[i].id, g_skills[i].popularity);
    }
    
    // Initialize Graph
    graph_init(&g_graph);
    for (int i = 0; i < g_num_students; i++) {
        graph_find_or_add_vertex(&g_graph, g_students[i].id, g_students[i].name, g_students[i].dept);
    }
    // Add sample peer relationships
    graph_add_edge(&g_graph, 1, 2, 10, "C Programming");
    graph_add_edge(&g_graph, 2, 1, 10, "React");
    graph_add_edge(&g_graph, 1, 4, 8, "DSA");
    graph_add_edge(&g_graph, 3, 4, 7, "Python");
    graph_add_edge(&g_graph, 2, 3, 6, "React");
    graph_add_edge(&g_graph, 7, 3, 9, "Public Speaking");
    
    // Initialize Queue & Stack
    queue_init(&g_queue);
    stack_init(&g_stack);
    
    LearningRequest reqs[50];
    int num_reqs = storage_load_requests(reqs, 50);
    for (int i = 0; i < num_reqs; i++) {
        if (strcmp(reqs[i].status, "PENDING") == 0) {
            queue_enqueue(&g_queue, &reqs[i]);
        }
    }
}

void print_banner(void) {
    printf("\n=======================================================================\n");
    printf("   SkillSwap - FODS (Fundamentals of Data Structures in C) CLI Engine  \n");
    printf("   Peer-to-Peer Skill Exchange Platform | Academic Viva Demo Console   \n");
    printf("=======================================================================\n");
}

void demo_view_students(void) {
    printf("\n--- Active Students in Memory (%d loaded) ---\n", g_num_students);
    printf("%-4s | %-18s | %-18s | %-4s | %-6s | %-8s\n", "ID", "Name", "Department", "Year", "Rating", "Sessions");
    printf("-----------------------------------------------------------------------\n");
    for (int i = 0; i < g_num_students; i++) {
        printf("%-4d | %-18s | %-18s | %-4d | %-6.2f | %-8d\n",
               g_students[i].id, g_students[i].name, g_students[i].dept,
               g_students[i].year, g_students[i].rating, g_students[i].completed_sessions);
    }
}

void demo_trie_search(void) {
    printf("\n--- [Data Structure: TRIE] Prefix Autocomplete Search ---\n");
    printf("Theoretical Time Complexity: O(L) where L is length of prefix\n");
    printf("Try prefix like 'Web', 'Py', 'Data', 'C', or enter your custom prefix:\n");
    printf("Enter prefix to search: ");
    char prefix[64];
    if (scanf("%63s", prefix) != 1) return;
    
    char results[10][MAX_SKILL_NAME_LEN];
    int count = trie_autocomplete(g_trie, prefix, results, 10);
    printf("Found %d matching skill suggestion(s):\n", count);
    for (int i = 0; i < count; i++) {
        printf("  [%d] %s\n", i + 1, results[i]);
    }
}

void demo_max_heap_leaderboard(void) {
    printf("\n--- [Data Structure: MAX-HEAP] Mentor Leaderboard Generation ---\n");
    printf("Formula: Score = (Rating * 20) + (Sessions * 10) + (Reviews * 5)\n");
    printf("Complexity: O(N log N) build/extraction using binary max-heap\n");
    
    MaxHeap heap;
    heap_init(&heap);
    
    for (int i = 0; i < g_num_students; i++) {
        if (g_students[i].completed_sessions > 0) {
            MentorScore ms;
            ms.student_id = g_students[i].id;
            strncpy(ms.name, g_students[i].name, MAX_NAME_LEN - 1);
            strncpy(ms.dept, g_students[i].dept, MAX_DEPT_LEN - 1);
            ms.year = g_students[i].year;
            strncpy(ms.top_skill, g_students[i].skills_offered_count > 0 ? 
                    g_students[i].skills_offered[0].skill_name : "General", MAX_SKILL_NAME_LEN - 1);
            ms.rating = g_students[i].rating;
            ms.completed_sessions = g_students[i].completed_sessions;
            ms.reviews_count = g_students[i].rating_count;
            ms.score = heap_calculate_score(ms.rating, ms.completed_sessions, ms.reviews_count);
            heap_insert(&heap, &ms);
        }
    }
    
    printf("\n%-4s | %-18s | %-16s | %-6s | %-8s | %-8s\n", "Rank", "Mentor Name", "Primary Skill", "Rating", "Sessions", "Heap Score");
    printf("--------------------------------------------------------------------------\n");
    int rank = 1;
    MentorScore top;
    while (heap_extract_max(&heap, &top)) {
        printf("#%-3d | %-18s | %-16s | %-6.2f | %-8d | %-8.1f\n",
               rank++, top.name, top.top_skill, top.rating, top.completed_sessions, top.score);
    }
}

void demo_skill_matching(void) {
    printf("\n--- [Algorithm: GRAPH & BIPARTITE MATCHING] Peer Recommendations ---\n");
    printf("Select a student to compute recommendations for:\n");
    for (int i = 0; i < 5 && i < g_num_students; i++) {
        printf("  [%d] %s (%s, offers: %s, wants: %s)\n",
               g_students[i].id, g_students[i].name, g_students[i].dept,
               g_students[i].skills_offered_count > 0 ? g_students[i].skills_offered[0].skill_name : "None",
               g_students[i].skills_wanted_count > 0 ? g_students[i].skills_wanted[0] : "None");
    }
    printf("Enter Student ID (default 1): ");
    int id = 1;
    if (scanf("%d", &id) != 1) id = 1;
    
    Student *target = NULL;
    for (int i = 0; i < g_num_students; i++) {
        if (g_students[i].id == id) {
            target = &g_students[i];
            break;
        }
    }
    if (!target) {
        printf("Invalid student ID.\n");
        return;
    }
    
    MatchRecommendation recs[10];
    int count = find_recommendations_for_student(target, g_students, g_num_students, &g_graph, recs, 10);
    printf("\nComputed %d match recommendation(s) for %s:\n", count, target->name);
    for (int i = 0; i < count; i++) {
        printf("  [%d] Mentor: %s | Teaches: %s (%s) | Rating: %.2f | Match: %d%%\n",
               i + 1, recs[i].mentor_name, recs[i].skill_offered, recs[i].proficiency,
               recs[i].mentor_rating, recs[i].match_score);
        printf("      Reason: %s\n", recs[i].reason);
    }
}

void demo_bst_inorder(void) {
    printf("\n--- [Data Structure: BINARY SEARCH TREE] Inorder Rating Sort ---\n");
    printf("Constructing BST using student rating as key...\n");
    BST tree;
    bst_init(&tree);
    
    for (int i = 0; i < g_num_students; i++) {
        bst_add(&tree, g_students[i].rating, g_students[i].id, g_students[i].name,
                g_students[i].skills_offered_count > 0 ? g_students[i].skills_offered[0].skill_name : "General");
    }
    
    BSTNode *inorder_nodes[MAX_STUDENTS];
    int count = 0;
    bst_inorder(tree.root, inorder_nodes, MAX_STUDENTS, &count);
    
    printf("Inorder Traversal Output (L-Root-R) sorted ascending by rating:\n");
    for (int i = 0; i < count; i++) {
        printf("  Rating: %.2f | ID: %d | Name: %s (%s)\n",
               inorder_nodes[i]->key, inorder_nodes[i]->student_id,
               inorder_nodes[i]->student_name, inorder_nodes[i]->primary_skill);
    }
    
    BSTNode *min_node = bst_find_min(tree.root);
    BSTNode *max_node = bst_find_max(tree.root);
    printf("BST Min: %.2f (%s) | BST Max: %.2f (%s)\n",
           min_node ? min_node->key : 0, min_node ? min_node->student_name : "",
           max_node ? max_node->key : 0, max_node ? max_node->student_name : "");
    
    bst_free(&tree);
}

void demo_queue(void) {
    printf("\n--- [Data Structure: QUEUE] FIFO Learning Requests ---\n");
    printf("Pending Requests in FIFO Queue: %d\n", queue_size(&g_queue));
    
    Queue temp_q = g_queue;
    QueueNode *curr = temp_q.front;
    int pos = 1;
    while (curr) {
        printf("  #%d [ID:%d] %s -> %s for '%s' (Status: %s)\n",
               pos++, curr->data.id, curr->data.requester_name,
               curr->data.mentor_name, curr->data.skill_name, curr->data.status);
        curr = curr->next;
    }
    
    if (!queue_is_empty(&g_queue)) {
        LearningRequest top;
        queue_dequeue(&g_queue, &top);
        printf("\n  [Simulated Action] Dequeued Front Request ID %d for processing!\n", top.id);
        printf("  Remaining Queue Size: %d\n", queue_size(&g_queue));
    }
}

void demo_graph_traversal(void) {
    printf("\n--- [Data Structure: GRAPH] BFS & DFS Traversal of Student Network ---\n");
    int start_id = 1;
    printf("Starting traversal from Student ID 1 (Aarav Sharma):\n");
    
    int bfs_ids[20];
    int distances[20];
    int bfs_count = graph_bfs(&g_graph, start_id, bfs_ids, distances, 20);
    printf("BFS (Breadth-First Search) Level-by-Level Discovery:\n");
    for (int i = 0; i < bfs_count; i++) {
        printf("  -> Student ID: %d (Degree of Separation / Distance: %d)\n", bfs_ids[i], distances[i]);
    }
    
    int dfs_ids[20];
    int dfs_count = graph_dfs(&g_graph, start_id, dfs_ids, 20);
    printf("\nDFS (Depth-First Search) Cluster Exploration:\n");
    for (int i = 0; i < dfs_count; i++) {
        printf("  -> Student ID: %d\n", dfs_ids[i]);
    }
}

int main(void) {
    init_system();
    print_banner();
    
    int choice = 0;
    while (1) {
        printf("\n=======================================================================\n");
        printf("  FODS DEMONSTRATION MENU:\n");
        printf("  1. View All Students (File Storage & Array Mapping)\n");
        printf("  2. Trie: Prefix Autocomplete Skill Search\n");
        printf("  3. Max-Heap: Ranked Mentor Leaderboard\n");
        printf("  4. Graph & Bipartite Matching: Skill Recommendations\n");
        printf("  5. Binary Search Tree (BST): Inorder Traversal\n");
        printf("  6. FIFO Queue: Learning Requests Processing\n");
        printf("  7. Graph Traversal: BFS Degrees & DFS Clusters\n");
        printf("  8. Exit CLI\n");
        printf("=======================================================================\n");
        printf("Enter your choice (1-8): ");
        if (scanf("%d", &choice) != 1) {
            break;
        }
        
        switch (choice) {
            case 1: demo_view_students(); break;
            case 2: demo_trie_search(); break;
            case 3: demo_max_heap_leaderboard(); break;
            case 4: demo_skill_matching(); break;
            case 5: demo_bst_inorder(); break;
            case 6: demo_queue(); break;
            case 7: demo_graph_traversal(); break;
            case 8:
                printf("\nExiting SkillSwap CLI Demo. Good luck with your FODS viva!\n");
                return 0;
            default:
                printf("Invalid option. Please choose 1-8.\n");
        }
    }
    return 0;
}
