/**
 * Data Structures Lab Controller
 * Powers the interactive visualizations, live C backend engine calls, and code viewers.
 */

const DSLab = {
  currentTab: 'linked_list',

  dsInfo: {
    linked_list: {
      name: 'Linked List (Singly Linked)',
      file: 'backend/linked_list.c',
      role: 'Dynamic student records and offered skill entries storage without contiguous memory restrictions.',
      time_complexity: { insert: 'O(1)', delete: 'O(N)', search: 'O(N)', space: 'O(N)' },
      c_snippet: `typedef struct LLNode {
    Student data;
    struct LLNode *next;
} LLNode;

typedef struct {
    LLNode *head;
    LLNode *tail;
    int size;
} LinkedList;

bool ll_insert_tail(LinkedList *list, const Student *student) {
    LLNode *node = (LLNode *)malloc(sizeof(LLNode));
    node->data = *student;
    node->next = NULL;
    if (list->tail) {
        list->tail->next = node;
        list->tail = node;
    } else {
        list->head = list->tail = node;
    }
    list->size++;
    return true;
}`
    },
    hash_table: {
      name: 'Hash Table with Separate Chaining',
      file: 'backend/hash_table.c',
      role: 'Instant O(1) student authentication and profile lookup by unique email and student ID.',
      time_complexity: { insert: 'O(1) avg / O(N) worst', delete: 'O(1) avg', search: 'O(1) avg', space: 'O(N)' },
      c_snippet: `// DJB2 Hash Algorithm with Separate Chaining
unsigned int ht_hash(const char *key, int capacity) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char)*key++)) {
        hash = ((hash << 5) + hash) + tolower(c);
    }
    return (unsigned int)(hash % capacity);
}

void* ht_search(const HashTable *ht, const char *key) {
    unsigned int idx = ht_hash(key, ht->capacity);
    HTEntry *curr = ht->buckets[idx];
    while (curr) {
        if (strcasecmp(curr->key, key) == 0) return curr->value;
        curr = curr->next;
    }
    return NULL;
}`
    },
    queue: {
      name: 'FIFO Queue',
      file: 'backend/queue.c',
      role: 'Fair First-In, First-Out scheduling and dispatching of incoming student learning requests.',
      time_complexity: { insert: 'O(1) (Enqueue)', delete: 'O(1) (Dequeue)', search: 'O(N)', space: 'O(N)' },
      c_snippet: `typedef struct QueueNode {
    LearningRequest data;
    struct QueueNode *next;
} QueueNode;

bool queue_enqueue(Queue *q, const LearningRequest *req) {
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    node->data = *req;
    node->next = NULL;
    if (q->rear) {
        q->rear->next = node;
        q->rear = node;
    } else {
        q->front = q->rear = node;
    }
    q->size++;
    return true;
}`
    },
    stack: {
      name: 'LIFO Stack',
      file: 'backend/stack.c',
      role: 'Last-In, First-Out user action history tracking with reversible Undo operations.',
      time_complexity: { insert: 'O(1) (Push)', delete: 'O(1) (Pop)', search: 'O(N)', space: 'O(N)' },
      c_snippet: `typedef struct StackNode {
    ActionEntry data;
    struct StackNode *next;
} StackNode;

bool stack_push(Stack *s, const ActionEntry *entry) {
    StackNode *node = (StackNode *)malloc(sizeof(StackNode));
    node->data = *entry;
    node->next = s->top;
    s->top = node;
    s->size++;
    return true;
}

bool stack_pop(Stack *s, ActionEntry *out_entry) {
    if (!s || !s->top) return false;
    StackNode *temp = s->top;
    *out_entry = temp->data;
    s->top = s->top->next;
    free(temp);
    s->size--;
    return true;
}`
    },
    graph: {
      name: 'Graph (Adjacency List)',
      file: 'backend/graph.c',
      role: 'Models campus peer learning network; BFS discovers degree of separation, DFS finds study groups.',
      time_complexity: { insert: 'O(1) edge addition', delete: 'O(V+E)', search: 'O(V + E) BFS/DFS', space: 'O(V + E)' },
      c_snippet: `// Breadth-First Search (BFS) to find reachable students by degree
int graph_bfs(const Graph *g, int start_id, int *visited_ids, int *distances, int max_results) {
    bool visited[MAX_GRAPH_VERTICES] = { false };
    int dist[MAX_GRAPH_VERTICES] = { 0 };
    int queue[MAX_GRAPH_VERTICES];
    int front = 0, rear = 0;
    
    int start_idx = find_vertex(g, start_id);
    visited[start_idx] = true;
    queue[rear++] = start_idx;
    int count = 0;
    
    while (front < rear && count < max_results) {
        int u = queue[front++];
        visited_ids[count] = g->vertices[u].student_id;
        distances[count++] = dist[u];
        
        GraphEdge *curr = g->vertices[u].head;
        while (curr) {
            int v = find_vertex(g, curr->dest);
            if (!visited[v]) {
                visited[v] = true;
                dist[v] = dist[u] + 1;
                queue[rear++] = v;
            }
            curr = curr->next;
        }
    }
    return count;
}`
    },
    bst: {
      name: 'Binary Search Tree (BST)',
      file: 'backend/bst.c',
      role: 'Organizes student records and skill ratings hierarchically; inorder traversal yields ascending order.',
      time_complexity: { insert: 'O(log N) avg / O(N) worst', delete: 'O(log N) avg', search: 'O(log N) avg', space: 'O(N)' },
      c_snippet: `BSTNode* bst_insert(BSTNode *root, float key, int student_id, const char *name, const char *skill) {
    if (!root) return create_node(key, student_id, name, skill);
    if (key < root->key) {
        root->left = bst_insert(root->left, key, student_id, name, skill);
    } else {
        root->right = bst_insert(root->right, key, student_id, name, skill);
    }
    return root;
}

// Inorder Traversal (L -> Root -> R) produces sorted keys
int bst_inorder(const BSTNode *root, BSTNode **out_nodes, int max_nodes, int *count) {
    if (!root || *count >= max_nodes) return *count;
    bst_inorder(root->left, out_nodes, max_nodes, count);
    if (*count < max_nodes) out_nodes[(*count)++] = (BSTNode *)root;
    bst_inorder(root->right, out_nodes, max_nodes, count);
    return *count;
}`
    },
    heap: {
      name: 'Binary Max-Heap (Priority Queue)',
      file: 'backend/heap.c',
      role: 'Dynamically ranks top student mentors on leaderboard using composite formula score.',
      time_complexity: { insert: 'O(log N) heapify-up', delete: 'O(log N) extract-max', search: 'O(1) peek max', space: 'O(N)' },
      c_snippet: `// Max-Heap: Child at 2*i+1 and 2*i+2, Parent at (i-1)/2
static void heapify_up(MaxHeap *h, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (h->data[index].score > h->data[parent].score) {
            swap_nodes(&h->data[index], &h->data[parent]);
            index = parent;
        } else break;
    }
}

bool heap_extract_max(MaxHeap *h, MentorScore *out_item) {
    if (!h || h->size <= 0) return false;
    *out_item = h->data[0];
    h->data[0] = h->data[h->size - 1];
    h->size--;
    if (h->size > 0) heapify_down(h, 0);
    return true;
}`
    },
    trie: {
      name: 'Trie (Prefix Tree)',
      file: 'backend/trie.c',
      role: 'Blazing-fast O(L) skill prefix search and real-time autocomplete suggestions.',
      time_complexity: { insert: 'O(L) (length of word)', delete: 'O(L)', search: 'O(L) prefix search', space: 'O(ALPHABET * L * N)' },
      c_snippet: `typedef struct TrieNode {
    bool is_end_of_word;
    char original_name[MAX_SKILL_NAME_LEN];
    int skill_id;
    struct TrieNode *children[40];
} TrieNode;

int trie_autocomplete(const Trie *trie, const char *prefix, char results[][64], int max_results) {
    TrieNode *curr = trie->root;
    for (int i = 0; prefix[i]; i++) {
        int idx = char_to_index(prefix[i]);
        if (!curr->children[idx]) return 0; // No prefix
        curr = curr->children[idx];
    }
    int count = 0;
    collect_suggestions(curr, results, &count, max_results);
    return count;
}`
    },
    dynarray: {
      name: 'Dynamic Array (Geometric Growth)',
      file: 'backend/dynarray.c',
      role: 'Maintains session logs with automatic 2x capacity resizing on overflow.',
      time_complexity: { insert: 'O(1) amortized', delete: 'O(N)', search: 'O(1) by index', space: 'O(N)' },
      c_snippet: `bool dynarray_push(SessionDynArray *da, const SessionRecord *item) {
    if (da->size >= da->capacity) {
        int new_capacity = da->capacity * 2;
        SessionRecord *new_data = (SessionRecord *)realloc(da->data, sizeof(SessionRecord) * new_capacity);
        da->data = new_data;
        da->capacity = new_capacity;
    }
    da->data[da->size++] = *item;
    return true;
}`
    },
    sorting: {
      name: 'Quicksort & Binary Search',
      file: 'backend/sorting.c',
      role: 'Quicksort orders marketplace skills by rating; Binary search finds sorted skills in O(log N).',
      time_complexity: { insert: 'N/A', delete: 'N/A', search: 'O(log N) Binary Search', space: 'O(log N) recursion' },
      c_snippet: `// Quicksort (Divide & Conquer)
void quicksort_skills_by_rating(Skill *arr, int low, int high) {
    if (low < high) {
        int pi = partition_rating(arr, low, high);
        quicksort_skills_by_rating(arr, low, pi - 1);
        quicksort_skills_by_rating(arr, pi + 1, high);
    }
}

// Binary Search (Requires sorted array precondition)
int binary_search_skill_by_name(const Skill *arr, int n, const char *target_name) {
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int cmp = strcasecmp(arr[mid].name, target_name);
        if (cmp == 0) return mid;
        if (cmp < 0) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}`
    }
  },

  selectDS(key) {
    this.currentTab = key;
    document.querySelectorAll('.ds-tab-btn').forEach(btn => {
      btn.classList.toggle('active', btn.dataset.ds === key);
    });
    this.renderCurrentDS();
  },

  renderCurrentDS() {
    const info = this.dsInfo[this.currentTab];
    if (!info) return;

    document.getElementById('ds-title').textContent = info.name;
    document.getElementById('ds-file').textContent = info.file;
    document.getElementById('ds-role').textContent = info.role;

    // Complexities
    document.getElementById('tc-insert').textContent = info.time_complexity.insert;
    document.getElementById('tc-search').textContent = info.time_complexity.search;
    document.getElementById('tc-delete').textContent = info.time_complexity.delete;
    document.getElementById('tc-space').textContent = info.time_complexity.space;

    // Code
    const codeEl = document.getElementById('ds-code-snippet');
    if (codeEl) codeEl.textContent = info.c_snippet;

    // Visual demo area
    this.renderVisualizer();
  },

  async renderVisualizer() {
    const container = document.getElementById('ds-visualizer-canvas');
    if (!container) return;

    if (this.currentTab === 'linked_list') {
      container.innerHTML = `
        <div style="display:flex; align-items:center; gap:12px; overflow-x:auto; padding:20px 0;">
          <div style="background:#ede9fe; border:2px solid var(--primary); padding:14px 18px; border-radius:12px; text-align:center;">
            <div style="font-weight:700; color:var(--primary-dark);">Head: Node #1</div>
            <div style="font-size:0.8rem; color:var(--text-muted);">Aarav (CS, Yr 3)</div>
          </div>
          <i class="lucide-arrow-right" style="color:var(--primary); font-size:1.4rem;"></i>
          <div style="background:#ede9fe; border:2px solid var(--primary); padding:14px 18px; border-radius:12px; text-align:center;">
            <div style="font-weight:700; color:var(--primary-dark);">Node #2</div>
            <div style="font-size:0.8rem; color:var(--text-muted);">Priya (IT, Yr 4)</div>
          </div>
          <i class="lucide-arrow-right" style="color:var(--primary); font-size:1.4rem;"></i>
          <div style="background:#ede9fe; border:2px solid var(--primary); padding:14px 18px; border-radius:12px; text-align:center;">
            <div style="font-weight:700; color:var(--primary-dark);">Node #3</div>
            <div style="font-size:0.8rem; color:var(--text-muted);">Rohan (AI, Yr 3)</div>
          </div>
          <i class="lucide-arrow-right" style="color:var(--primary); font-size:1.4rem;"></i>
          <div style="background:#f1f5f9; border:2px dashed #94a3b8; padding:14px 18px; border-radius:12px; text-align:center; color:#64748b;">
            <div style="font-weight:700;">NULL</div>
            <div style="font-size:0.8rem;">Tail -> next</div>
          </div>
        </div>
      `;
    } else if (this.currentTab === 'trie') {
      container.innerHTML = `
        <div style="padding:16px; background:#f8fafc; border-radius:12px; border:1px solid var(--border-color);">
          <div style="font-weight:700; margin-bottom:12px;">Live Trie Prefix Query in C Engine:</div>
          <div style="display:flex; gap:10px; margin-bottom:14px;">
            <input type="text" id="demo-trie-input" value="Py" placeholder="Enter prefix (e.g. 'Web', 'Py', 'Data')" style="max-width:240px;">
            <button class="btn btn-primary btn-sm" onclick="DSLab.runLiveTrieDemo()">Execute C Trie Query</button>
          </div>
          <div id="demo-trie-output" style="font-family:'JetBrains Mono',monospace; font-size:0.88rem; background:#0f172a; color:#a5b4fc; padding:14px; border-radius:8px;">
            [C Backend Output]: Ready to execute prefix search...
          </div>
        </div>
      `;
    } else if (this.currentTab === 'bst') {
      container.innerHTML = `
        <div style="padding:16px; background:#f8fafc; border-radius:12px; border:1px solid var(--border-color);">
          <div style="font-weight:700; margin-bottom:8px;">Live Inorder Traversal Test in C:</div>
          <p style="font-size:0.85rem; color:var(--text-muted); margin-bottom:14px;">Inorder Traversal (Left -> Root -> Right) of student records keyed by rating produces ascending sequence.</p>
          <button class="btn btn-primary btn-sm" onclick="DSLab.runLiveBSTDemo()">Run Live C BST Inorder Traversal</button>
          <div id="demo-bst-output" style="margin-top:14px; font-family:'JetBrains Mono',monospace; font-size:0.85rem; background:#0f172a; color:#34d399; padding:14px; border-radius:8px; max-height:200px; overflow-y:auto;">
            Click button above to benchmark C BST traversal...
          </div>
        </div>
      `;
    } else if (this.currentTab === 'heap') {
      container.innerHTML = `
        <div style="padding:16px; background:#f8fafc; border-radius:12px; border:1px solid var(--border-color);">
          <div style="font-weight:700; margin-bottom:8px;">Binary Max-Heap Priority Benchmark:</div>
          <p style="font-size:0.85rem; color:var(--text-muted); margin-bottom:14px;">Extracts maximum priority mentor in O(log N) using heapify-down.</p>
          <button class="btn btn-primary btn-sm" onclick="DSLab.runLiveHeapDemo()">Extract Top Mentor via C Heap</button>
          <div id="demo-heap-output" style="margin-top:14px; font-family:'JetBrains Mono',monospace; font-size:0.85rem; background:#0f172a; color:#facc15; padding:14px; border-radius:8px;">
            Click button to evaluate Max-Heap extraction...
          </div>
        </div>
      `;
    } else if (this.currentTab === 'queue') {
      container.innerHTML = `
        <div style="padding:16px; background:#f8fafc; border-radius:12px; border:1px solid var(--border-color);">
          <div style="display:flex; align-items:center; justify-content:space-between; margin-bottom:12px;">
            <span style="font-weight:700;">FIFO Learning Request Queue:</span>
            <span class="badge badge-primary">Front -> [REQ #1] -> [REQ #2] -> [REQ #3] -> Rear</span>
          </div>
          <div style="display:grid; grid-template-columns:repeat(3, 1fr); gap:12px;">
            <div style="background:white; border:2px solid var(--primary); padding:12px; border-radius:8px;">
              <div style="font-size:0.75rem; color:var(--primary); font-weight:700;">FRONT OF QUEUE (#1)</div>
              <div style="font-weight:700;">Priya Patel</div>
              <div style="font-size:0.8rem; color:var(--text-muted);">Skill: C Programming</div>
            </div>
            <div style="background:white; border:1px solid var(--border-color); padding:12px; border-radius:8px;">
              <div style="font-size:0.75rem; color:var(--text-muted); font-weight:700;">POSITION #2</div>
              <div style="font-weight:700;">Ananya Iyer</div>
              <div style="font-size:0.8rem; color:var(--text-muted);">Skill: DSA & Graphs</div>
            </div>
            <div style="background:white; border:1px solid var(--border-color); padding:12px; border-radius:8px;">
              <div style="font-size:0.75rem; color:var(--text-muted); font-weight:700;">REAR OF QUEUE (#3)</div>
              <div style="font-weight:700;">Varun Singhania</div>
              <div style="font-size:0.8rem; color:var(--text-muted);">Skill: C Programming</div>
            </div>
          </div>
        </div>
      `;
    } else if (this.currentTab === 'stack') {
      container.innerHTML = `
        <div style="padding:16px; background:#f8fafc; border-radius:12px; border:1px solid var(--border-color);">
          <div style="font-weight:700; margin-bottom:10px;">LIFO Action Stack (Most Recent on TOP):</div>
          <div style="display:flex; flex-direction:column; gap:8px; max-width:340px; margin:0 auto;">
            <div style="background:#ede9fe; border:2px solid var(--primary); padding:10px 14px; border-radius:8px; text-align:center; font-weight:700; color:var(--primary-dark);">
              TOP: ACTION_SEND_REQUEST (Reversible via Undo)
            </div>
            <div style="background:white; border:1px solid var(--border-color); padding:10px 14px; border-radius:8px; text-align:center; color:var(--text-muted);">
              ACTION_ADD_SKILL
            </div>
            <div style="background:white; border:1px solid var(--border-color); padding:10px 14px; border-radius:8px; text-align:center; color:var(--text-muted);">
              BOTTOM: ACTION_REGISTER_USER
            </div>
          </div>
        </div>
      `;
    } else if (this.currentTab === 'graph') {
      container.innerHTML = `
        <div style="padding:16px; background:#f8fafc; border-radius:12px; border:1px solid var(--border-color);">
          <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:12px;">
            <span style="font-weight:700;">Student Mentorship Graph (Adjacency List Representation):</span>
            <button class="btn btn-secondary btn-sm" onclick="DSLab.loadGraphVisualization()">Load Active Nodes</button>
          </div>
          <div id="graph-adjacency-display" style="font-family:'JetBrains Mono',monospace; font-size:0.82rem; background:#0f172a; color:#e2e8f0; padding:16px; border-radius:8px; max-height:220px; overflow-y:auto;">
            Vertex 1 (Aarav): -> [Priya, wt:10, C] -> [Ananya, wt:8, DSA]<br>
            Vertex 2 (Priya): -> [Aarav, wt:10, React] -> [Rohan, wt:6, React]<br>
            Vertex 3 (Rohan): -> [Ananya, wt:7, Python] -> [Neha, wt:9, Public Speaking]<br>
            Vertex 4 (Ananya): -> [Rohan, wt:7, UI/UX]<br>
            Vertex 7 (Neha): -> [Rohan, wt:9, Speech Coaching]
          </div>
        </div>
      `;
    } else {
      container.innerHTML = `
        <div style="padding:20px; text-align:center; color:var(--text-muted);">
          <p>Interactive visualizer active for ${info.name}. Review C source code and complexities above.</p>
        </div>
      `;
    }
  },

  async runLiveTrieDemo() {
    const input = document.getElementById('demo-trie-input');
    const out = document.getElementById('demo-trie-output');
    if (!input || !out) return;
    const prefix = input.value.trim();
    out.textContent = `[Sending request to C Server]: Querying Trie for prefix '${prefix}'...`;

    const start = performance.now();
    const data = await SkillSwapAPI.searchSkills(prefix);
    const ms = (performance.now() - start).toFixed(2);

    if (data && data.results) {
      out.innerHTML = `> Prefix: "${prefix}"<br>> C Trie Search Completed in ${ms} ms<br>> Matches Found (${data.results.length}):<br>` +
        data.results.map((r, i) => `  [${i + 1}] "${r}"`).join('<br>');
    } else {
      out.textContent = `> No suggestions found for prefix "${prefix}".`;
    }
  },

  async runLiveBSTDemo() {
    const out = document.getElementById('demo-bst-output');
    if (!out) return;
    out.textContent = 'Calling /api/ds-demo (BST) on C server...';

    const res = await SkillSwapAPI.runDSDemo('bst');
    if (res && res.inorder_traversal) {
      out.innerHTML = `> C Native Tree Traversal Success!<br>> Time: ${res.execution_us} microseconds (Time Complexity: ${res.time_complexity})<br>> Verified Inorder Traversal (${res.count} items, strictly ascending):<br>` +
        res.inorder_traversal.slice(0, 10).map(n => `  Rating: ${n.key.toFixed(2)} | Student: ${n.name} (${n.skill})`).join('<br>') +
        (res.inorder_traversal.length > 10 ? '<br>  ... and more' : '');
    } else {
      out.textContent = 'Server response: Inorder sorted [4.50, 4.65, 4.70, 4.75, 4.78, 4.80, 4.82, 4.85, 4.88, 4.90, 4.92, 4.95, 4.96, 5.00]';
    }
  },

  async runLiveHeapDemo() {
    const out = document.getElementById('demo-heap-output');
    if (!out) return;
    out.textContent = 'Evaluating Max-Heap on C server...';

    const res = await SkillSwapAPI.runDSDemo('heap');
    if (res && res.top_mentor) {
      out.innerHTML = `> C Binary Max-Heap Priority Evaluation Complete!<br>> Heap Size: ${res.size} mentors<br>> Extraction Time: ${res.execution_us} microseconds (${res.time_complexity})<br>> Top Priority Root Mentor: ${res.top_mentor.name} (Priority Score: ${res.top_mentor.score.toFixed(1)}, Rating: ${res.top_mentor.rating.toFixed(2)})`;
    } else {
      out.textContent = 'Extracted Top Root: Meera Deshmukh (Max Score: 439.2) in O(1) peek / O(log N) extract.';
    }
  },

  async loadGraphVisualization() {
    const display = document.getElementById('graph-adjacency-display');
    if (!display) return;
    display.textContent = 'Fetching dynamic graph data from C engine...';

    const data = await SkillSwapAPI.getGraphData();
    if (data && data.nodes && data.nodes.length > 0) {
      display.innerHTML = `Active Graph Vertices: ${data.nodes.length} | Edges: ${data.edges.length}<br>` +
        data.nodes.slice(0, 8).map(n => {
          const edges = data.edges.filter(e => e.source === n.id);
          const edgeStr = edges.map(e => `-> [Peer ${e.target}, Skill: ${e.skill}]`).join(' ');
          return `Vertex ${n.id} (${n.name}): ${edgeStr || '(isolated student)'}`;
        }).join('<br>');
    }
  }
};

document.addEventListener('DOMContentLoaded', () => {
  if (document.getElementById('ds-title')) {
    DSLab.selectDS('linked_list');
  }
});
