#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
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

static int g_tests_passed = 0;
static int g_tests_failed = 0;

#define TEST_ASSERT(cond, msg) do { \
    if (cond) { \
        printf("  [PASS] %s\n", msg); \
        g_tests_passed++; \
    } else { \
        printf("  [FAIL] %s (Line %d)\n", msg, __LINE__); \
        g_tests_failed++; \
    } \
} while(0)

void test_linked_list(void) {
    printf("\n=== 1. Testing Linked List (FODS: Dynamic Node Management) ===\n");
    LinkedList list;
    ll_init(&list);
    
    Student s1, s2, s3;
    student_init(&s1, 101, "Alice Smith", "alice@college.edu", "pass1", "CS", 2, "Loves C", "");
    student_init(&s2, 102, "Bob Jones", "bob@college.edu", "pass2", "IT", 3, "Web Dev", "");
    student_init(&s3, 103, "Charlie Day", "charlie@college.edu", "pass3", "AI", 1, "Robotics", "");
    
    ll_insert_tail(&list, &s1);
    ll_insert_tail(&list, &s2);
    ll_insert_head(&list, &s3); // Charlie is now head
    
    TEST_ASSERT(list.size == 3, "Linked list size is 3 after insertions");
    TEST_ASSERT(list.head->data.id == 103, "Charlie is at head");
    TEST_ASSERT(list.tail->data.id == 102, "Bob is at tail");
    
    Student *found = ll_search_by_id(&list, 101);
    TEST_ASSERT(found != NULL && strcmp(found->name, "Alice Smith") == 0, "Found Alice by ID=101");
    
    bool del_res = ll_delete_by_id(&list, 101);
    TEST_ASSERT(del_res == true && list.size == 2, "Deleted Alice, size is now 2");
    TEST_ASSERT(ll_search_by_id(&list, 101) == NULL, "Alice is no longer present");
    
    ll_free(&list);
    TEST_ASSERT(list.size == 0 && list.head == NULL, "Linked list successfully freed");
}

void test_hash_table(void) {
    printf("\n=== 2. Testing Hash Table (FODS: Separate Chaining Collision Resolution) ===\n");
    HashTable *ht = ht_create(8); // Small capacity to force collisions
    
    int val1 = 100, val2 = 200, val3 = 300, val4 = 400;
    ht_insert(ht, "alice@college.edu", &val1);
    ht_insert(ht, "bob@college.edu", &val2);
    ht_insert(ht, "charlie@college.edu", &val3);
    ht_insert(ht, "diana@college.edu", &val4);
    
    TEST_ASSERT(ht->count == 4, "Hash table count is 4");
    
    int *res = (int *)ht_search(ht, "alice@college.edu");
    TEST_ASSERT(res != NULL && *res == 100, "Search 'alice@college.edu' returned 100");
    
    res = (int *)ht_search(ht, "bob@college.edu");
    TEST_ASSERT(res != NULL && *res == 200, "Search 'bob@college.edu' returned 200");
    
    int count, cap, coll; float lf;
    ht_get_stats(ht, &count, &cap, &coll, &lf);
    printf("  [INFO] Hash Table Stats: Count=%d, Cap=%d, Collisions=%d, Load Factor=%.2f\n", count, cap, coll, lf);
    TEST_ASSERT(lf > 0.0f, "Load factor computed correctly");
    
    bool del_res = ht_delete(ht, "bob@college.edu");
    TEST_ASSERT(del_res == true && ht->count == 3, "Deleted 'bob@college.edu', count is 3");
    TEST_ASSERT(ht_search(ht, "bob@college.edu") == NULL, "Bob is no longer found in Hash Table");
    
    ht_free(ht, false);
}

void test_queue(void) {
    printf("\n=== 3. Testing Queue (FODS: FIFO Processing of Learning Requests) ===\n");
    Queue q;
    queue_init(&q);
    
    LearningRequest r1 = { .id = 1, .requester_id = 10, .mentor_id = 20, .skill_name = "C Programming" };
    LearningRequest r2 = { .id = 2, .requester_id = 11, .mentor_id = 20, .skill_name = "React" };
    LearningRequest r3 = { .id = 3, .requester_id = 12, .mentor_id = 20, .skill_name = "Python" };
    
    queue_enqueue(&q, &r1);
    queue_enqueue(&q, &r2);
    queue_enqueue(&q, &r3);
    
    TEST_ASSERT(queue_size(&q) == 3, "Queue size is 3");
    TEST_ASSERT(queue_peek(&q)->id == 1, "Queue peek returns first request (ID=1)");
    
    LearningRequest out1, out2, out3;
    queue_dequeue(&q, &out1);
    queue_dequeue(&q, &out2);
    queue_dequeue(&q, &out3);
    
    TEST_ASSERT(out1.id == 1, "First dequeued is ID=1 (FIFO Order)");
    TEST_ASSERT(out2.id == 2, "Second dequeued is ID=2 (FIFO Order)");
    TEST_ASSERT(out3.id == 3, "Third dequeued is ID=3 (FIFO Order)");
    TEST_ASSERT(queue_is_empty(&q) == true, "Queue is empty after 3 dequeues");
    
    queue_free(&q);
}

void test_stack(void) {
    printf("\n=== 4. Testing Stack (FODS: LIFO Undo Action Management) ===\n");
    Stack s;
    stack_init(&s);
    
    ActionEntry a1 = { .id = 1, .type = ACTION_ADD_SKILL, .student_id = 1, .details = "Added skill C" };
    ActionEntry a2 = { .id = 2, .type = ACTION_SEND_REQUEST, .student_id = 1, .details = "Sent request to Bob" };
    ActionEntry a3 = { .id = 3, .type = ACTION_UPDATE_BIO, .student_id = 1, .details = "Updated bio" };
    
    stack_push(&s, &a1);
    stack_push(&s, &a2);
    stack_push(&s, &a3);
    
    TEST_ASSERT(stack_size(&s) == 3, "Stack size is 3");
    TEST_ASSERT(stack_peek(&s)->id == 3, "Stack peek returns top action (ID=3)");
    
    ActionEntry popped;
    stack_pop(&s, &popped);
    TEST_ASSERT(popped.id == 3, "Popped most recent action ID=3 (LIFO Undo)");
    
    stack_pop(&s, &popped);
    TEST_ASSERT(popped.id == 2, "Popped previous action ID=2");
    
    TEST_ASSERT(stack_size(&s) == 1, "Stack size is now 1");
    stack_free(&s);
}

void test_graph(void) {
    printf("\n=== 5. Testing Graph (FODS: Adjacency List, BFS Network & DFS Clusters) ===\n");
    Graph g;
    graph_init(&g);
    
    // Create network: Aarav (1) <-> Priya (2) <-> Rohan (3) <-> Ananya (4)
    graph_add_edge(&g, 1, 2, 5, "C Programming");
    graph_add_edge(&g, 2, 1, 5, "React");
    graph_add_edge(&g, 2, 3, 3, "Python");
    graph_add_edge(&g, 3, 2, 3, "Python");
    graph_add_edge(&g, 3, 4, 4, "UI/UX");
    graph_add_edge(&g, 4, 3, 4, "UI/UX");
    
    TEST_ASSERT(g.num_vertices >= 4, "Graph has at least 4 vertices");
    
    int bfs_ids[10];
    int distances[10];
    int bfs_count = graph_bfs(&g, 1, bfs_ids, distances, 10);
    
    TEST_ASSERT(bfs_count >= 4, "BFS traversed connected component");
    TEST_ASSERT(bfs_ids[0] == 1 && distances[0] == 0, "BFS start node is 1 at distance 0");
    TEST_ASSERT(bfs_ids[1] == 2 && distances[1] == 1, "BFS 1st degree neighbor is 2 at distance 1");
    printf("  [INFO] BFS Traversal Order from Student 1: ");
    for (int i = 0; i < bfs_count; i++) {
        printf("%d(dist=%d) ", bfs_ids[i], distances[i]);
    }
    printf("\n");
    
    int dfs_ids[10];
    int dfs_count = graph_dfs(&g, 1, dfs_ids, 10);
    TEST_ASSERT(dfs_count >= 4, "DFS explored entire connected cluster");
    printf("  [INFO] DFS Exploration Order from Student 1: ");
    for (int i = 0; i < dfs_count; i++) {
        printf("%d ", dfs_ids[i]);
    }
    printf("\n");
    
    graph_free(&g);
}

void test_bst(void) {
    printf("\n=== 6. Testing Binary Search Tree (FODS: Inorder Traversal & Ordering) ===\n");
    BST tree;
    bst_init(&tree);
    
    bst_add(&tree, 4.80f, 10, "Student 4.80", "Python");
    bst_add(&tree, 4.50f, 20, "Student 4.50", "C");
    bst_add(&tree, 4.95f, 30, "Student 4.95", "DSA");
    bst_add(&tree, 4.65f, 40, "Student 4.65", "React");
    bst_add(&tree, 4.90f, 50, "Student 4.90", "Design");
    
    TEST_ASSERT(tree.count == 5, "BST has 5 nodes");
    
    BSTNode *search_res = bst_search(tree.root, 4.95f);
    TEST_ASSERT(search_res != NULL && search_res->student_id == 30, "BST found key 4.95 (ID=30)");
    
    BSTNode *min_node = bst_find_min(tree.root);
    TEST_ASSERT(min_node != NULL && min_node->key == 4.50f, "BST Min is 4.50");
    
    BSTNode *max_node = bst_find_max(tree.root);
    TEST_ASSERT(max_node != NULL && max_node->key == 4.95f, "BST Max is 4.95");
    
    BSTNode *inorder_arr[10];
    int count = 0;
    bst_inorder(tree.root, inorder_arr, 10, &count);
    
    TEST_ASSERT(count == 5, "Inorder collected 5 nodes");
    bool is_sorted = true;
    for (int i = 0; i < count - 1; i++) {
        if (inorder_arr[i]->key > inorder_arr[i + 1]->key) {
            is_sorted = false;
        }
    }
    TEST_ASSERT(is_sorted == true, "BST Inorder Traversal strictly produces sorted ascending keys");
    
    printf("  [INFO] Inorder Keys: ");
    for (int i = 0; i < count; i++) {
        printf("%.2f ", inorder_arr[i]->key);
    }
    printf("\n");
    
    bst_free(&tree);
}

void test_heap(void) {
    printf("\n=== 7. Testing Max-Heap (FODS: Leaderboard Priority Queue) ===\n");
    MaxHeap h;
    heap_init(&h);
    
    MentorScore m1 = { .student_id = 1, .name = "Aarav", .score = heap_calculate_score(4.95f, 14, 12) };
    MentorScore m2 = { .student_id = 2, .name = "Priya", .score = heap_calculate_score(4.90f, 21, 18) };
    MentorScore m3 = { .student_id = 3, .name = "Rohan", .score = heap_calculate_score(4.85f, 11, 9) };
    MentorScore m4 = { .student_id = 13, .name = "Meera", .score = heap_calculate_score(4.96f, 24, 20) };
    
    heap_insert(&h, &m1);
    heap_insert(&h, &m2);
    heap_insert(&h, &m3);
    heap_insert(&h, &m4);
    
    TEST_ASSERT(heap_size(&h) == 4, "Max-Heap size is 4");
    
    MentorScore top;
    heap_extract_max(&h, &top);
    TEST_ASSERT(top.student_id == 13, "Top mentor extracted is Meera (Highest Score)");
    
    MentorScore second;
    heap_extract_max(&h, &second);
    TEST_ASSERT(second.student_id == 2, "Second mentor extracted is Priya");
    
    printf("  [INFO] Ranked mentors: #1 %s (Score: %.1f), #2 %s (Score: %.1f)\n",
           top.name, top.score, second.name, second.score);
}

void test_trie(void) {
    printf("\n=== 8. Testing Trie (FODS: Prefix Search & Autocomplete) ===\n");
    Trie *trie = trie_create();
    
    trie_insert(trie, "Web Development", 1, 90);
    trie_insert(trie, "Web Design", 2, 70);
    trie_insert(trie, "Python", 3, 95);
    trie_insert(trie, "PyTorch", 4, 80);
    trie_insert(trie, "Data Structures", 5, 99);
    
    TEST_ASSERT(trie->total_words == 5, "Trie has 5 words inserted");
    TEST_ASSERT(trie_search_exact(trie, "Python") == true, "Exact search 'Python' found");
    TEST_ASSERT(trie_search_exact(trie, "Java") == false, "Exact search 'Java' correctly not found");
    
    char suggestions[5][MAX_SKILL_NAME_LEN];
    int num_sugg = trie_autocomplete(trie, "Web", suggestions, 5);
    TEST_ASSERT(num_sugg == 2, "Prefix 'Web' yielded 2 autocomplete suggestions");
    printf("  [INFO] Autocomplete for 'Web': ");
    for (int i = 0; i < num_sugg; i++) {
        printf("'%s' ", suggestions[i]);
    }
    printf("\n");
    
    num_sugg = trie_autocomplete(trie, "Py", suggestions, 5);
    TEST_ASSERT(num_sugg == 2, "Prefix 'Py' yielded 2 autocomplete suggestions");
    
    trie_free(trie);
}

void test_sorting_and_searching(void) {
    printf("\n=== 9. Testing Sorting & Binary Search (FODS: Quicksort & Binary Search) ===\n");
    Skill skills[5];
    skill_init(&skills[0], 1, "Python", "Programming", 1, "Rohan", "AI", 3, "Expert", "1-on-1", 4.85f, 10, 80);
    skill_init(&skills[1], 2, "C Programming", "Programming", 2, "Aarav", "CS", 3, "Expert", "1-on-1", 4.95f, 15, 95);
    skill_init(&skills[2], 3, "React", "Web Dev", 3, "Priya", "IT", 4, "Expert", "1-on-1", 4.90f, 20, 99);
    skill_init(&skills[3], 4, "Design", "Design", 4, "Ananya", "Media", 2, "Expert", "1-on-1", 4.70f, 5, 75);
    skill_init(&skills[4], 5, "Algorithms", "Programming", 5, "Kavya", "CS", 3, "Expert", "1-on-1", 4.88f, 12, 85);
    
    // Sort by rating descending
    quicksort_skills_by_rating(skills, 0, 4);
    TEST_ASSERT(skills[0].rating >= skills[1].rating && skills[1].rating >= skills[2].rating,
                "Quicksort ordered skills by rating descending");
    TEST_ASSERT(strcmp(skills[0].name, "C Programming") == 0, "Highest rated is C Programming (4.95)");
    
    // Sort by name ascending for binary search
    quicksort_skills_by_name(skills, 0, 4);
    TEST_ASSERT(strcmp(skills[0].name, "Algorithms") == 0, "First alphabetical is Algorithms");
    
    int bsearch_idx = binary_search_skill_by_name(skills, 5, "Python");
    TEST_ASSERT(bsearch_idx >= 0 && strcmp(skills[bsearch_idx].name, "Python") == 0,
                "Binary search found 'Python' on sorted array in O(log n)");
    
    int missing = binary_search_skill_by_name(skills, 5, "NonExistentSkill");
    TEST_ASSERT(missing == -1, "Binary search correctly returned -1 for missing skill");
}

void test_dynarray(void) {
    printf("\n=== 10. Testing Dynamic Array (FODS: Geometric Growth & Session Store) ===\n");
    SessionDynArray *da = dynarray_create(2); // Small capacity 2 to test automatic doubling
    TEST_ASSERT(da->capacity == 2, "Initial capacity is 2");
    
    SessionRecord s1 = { .id = 1, .skill_name = "C Programming", .rating = 5.0f };
    SessionRecord s2 = { .id = 2, .skill_name = "React", .rating = 4.9f };
    SessionRecord s3 = { .id = 3, .skill_name = "Python", .rating = 4.8f }; // Triggers resize
    
    dynarray_push(da, &s1);
    dynarray_push(da, &s2);
    dynarray_push(da, &s3);
    
    TEST_ASSERT(da->size == 3, "Dynamic array size is 3");
    TEST_ASSERT(da->capacity >= 4, "Capacity doubled geometrically to at least 4");
    TEST_ASSERT(dynarray_get(da, 2)->id == 3, "Indexed element 2 is ID=3");
    
    dynarray_free(da);
}

void test_matching_algorithm(void) {
    printf("\n=== 11. Testing Graph-Based Skill Matching Algorithm ===\n");
    Student s_aarav, s_priya, s_rohan;
    student_init(&s_aarav, 1, "Aarav", "aarav@college.edu", "pass", "CS", 3, "bio", "");
    student_add_skill_offered(&s_aarav, "C Programming", "Programming", "Expert", "1-on-1");
    student_add_skill_wanted(&s_aarav, "React");
    
    student_init(&s_priya, 2, "Priya", "priya@college.edu", "pass", "IT", 4, "bio", "");
    student_add_skill_offered(&s_priya, "React", "Web Dev", "Expert", "1-on-1");
    student_add_skill_wanted(&s_priya, "C Programming");
    
    student_init(&s_rohan, 3, "Rohan", "rohan@college.edu", "pass", "AI", 3, "bio", "");
    student_add_skill_offered(&s_rohan, "Python", "Programming", "Expert", "Online");
    student_add_skill_wanted(&s_rohan, "Guitar");
    
    Student all[3] = { s_aarav, s_priya, s_rohan };
    MatchRecommendation recs[5];
    int count = find_recommendations_for_student(&s_aarav, all, 3, NULL, recs, 5);
    
    TEST_ASSERT(count >= 1, "Found at least 1 match recommendation for Aarav");
    TEST_ASSERT(recs[0].mentor_id == 2, "Top recommended match is Priya");
    TEST_ASSERT(recs[0].is_mutual_swap == true, "Identified perfect 2-way Mutual Skill Swap!");
    printf("  [INFO] Recommended Match: %s (Match Score: %d%%, %s)\n",
           recs[0].mentor_name, recs[0].match_score, recs[0].reason);
}

int main(void) {
    printf("=================================================================\n");
    printf("   SkillSwap - FODS (Fundamentals of Data Structures in C) Tests  \n");
    printf("=================================================================\n");
    
    test_linked_list();
    test_hash_table();
    test_queue();
    test_stack();
    test_graph();
    test_bst();
    test_heap();
    test_trie();
    test_sorting_and_searching();
    test_dynarray();
    test_matching_algorithm();
    
    printf("\n=================================================================\n");
    printf("  TEST RESULTS: Passed: %d | Failed: %d\n", g_tests_passed, g_tests_failed);
    printf("=================================================================\n");
    
    return (g_tests_failed == 0) ? 0 : 1;
}
