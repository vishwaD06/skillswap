# SkillSwap – A Skill Exchange Program
### Fundamentals of Data Structures (FODS) Using C Language

**SkillSwap** is a full-featured, college-level peer-to-peer skill-sharing web application built for the **Fundamentals of Data Structures (FODS)** course. It enables students to teach skills they know and learn skills from their peers without any monetary transactions.

Unlike superficial web demos that simulate algorithms in JavaScript, SkillSwap's core data structures, algorithms, and matching mechanics are genuinely implemented in **C language**, backed by a native C HTTP REST server (`Winsock2`), an interactive academic CLI demonstration console, and persistent file storage.

---

## 🏛️ System Architecture

SkillSwap is structured into three clear logical layers:

```
┌─────────────────────────────────────────────────────────┐
│               1. Presentation Layer (Frontend)          │
│   HTML5 • Modern Vanilla CSS3 • JavaScript (ES6 Modules)│
│   7 Pages: Landing, Dashboard, Marketplace, Profile,    │
│            Requests, Leaderboard, Data Structures Lab   │
└───────────────────────────┬─────────────────────────────┘
                            │ REST API (JSON / HTTP)
┌───────────────────────────▼─────────────────────────────┐
│          2. Application Layer (Native C Engine)         │
│   • Linked List     • Hash Table (Chaining) • FIFO Queue│
│   • LIFO Stack      • Graph (Adjacency List) • BST Tree │
│   • Binary Max-Heap • Trie (Prefix Tree)   • Quicksort  │
│   • Bipartite Graph Skill Matching Engine               │
│   • Lightweight Winsock HTTP Server (port 8080)         │
└───────────────────────────┬─────────────────────────────┘
                            │ File I/O
┌───────────────────────────▼─────────────────────────────┐
│               3. Persistent Storage Layer               │
│   • data/students.dat    • data/skills.dat              │
│   • data/requests.dat    • data/sessions.dat            │
└─────────────────────────────────────────────────────────┘
```

---

## 🧩 10 Data Structures & Algorithms in C

| # | Data Structure / Algorithm | C Implementation File | Real Application within SkillSwap | Time Complexity |
|---|-----------------------------|-----------------------|------------------------------------|-----------------|
| 1 | **Singly Linked List** | `backend/linked_list.c` | Dynamic student nodes and offered skills list without contiguous memory allocation | Insertion: $O(1)$, Search/Delete: $O(N)$ |
| 2 | **Hash Table** | `backend/hash_table.c` | Separate chaining collision resolution (DJB2 hash) for instant student login and profile lookup | Search: $O(1)$ avg, $O(N)$ worst |
| 3 | **FIFO Queue** | `backend/queue.c` | First-In, First-Out fair queue scheduling of incoming peer learning requests | Enqueue: $O(1)$, Dequeue: $O(1)$ |
| 4 | **LIFO Stack** | `backend/stack.c` | User action history tracking with reversible live Undo operations | Push: $O(1)$, Pop: $O(1)$ |
| 5 | **Graph (Adjacency List)** | `backend/graph.c` | Student network modeling. BFS computes degree of separation; DFS identifies connected study groups | Traversal: $O(V + E)$ |
| 6 | **Binary Search Tree (BST)** | `backend/bst.c` | Hierarchical indexing by rating. Inorder traversal (L-Root-R) yields sorted order | Search/Insert: $O(\log N)$ avg |
| 7 | **Binary Max-Heap** | `backend/heap.c` | Priority queue ranking mentors for the Leaderboard using composite formula scores | Extract-Max: $O(\log N)$, Peek: $O(1)$ |
| 8 | **Trie (Prefix Tree)** | `backend/trie.c` | 40-character alphabet tree for real-time skill prefix search and autocomplete | Search/Autocomplete: $O(L)$ where $L = \text{length}$ |
| 9 | **Dynamic Array** | `backend/dynarray.c` | Geometrically doubling session logs and review history store (realloc 2x) | Append: $O(1)$ amortized |
| 10 | **Quicksort & Binary Search** | `backend/sorting.c` | Quicksort sorts marketplace skills by rating; Binary search finds sorted items in $O(\log N)$ | Quicksort: $O(N \log N)$ avg, BSearch: $O(\log N)$ |

### Graph-Based Bipartite Skill Matching Algorithm (`backend/matching.c`)
- **Direct Match**: Identifies mentors teaching what the user wants to learn.
- **Mutual Skill Swap**: Detects a 2-way reciprocal cycle (User A teaches X, wants Y; User B teaches Y, wants X). Awards a **+35% bonus score**.
- **Affinity Scoring**: Combines rating weight (up to 15 pts), session activity, and campus department affinity for a final 0–100% match score.

---

## 📂 Project Directory Structure

```
SkillSwap/
├── backend/                  # Modular C backend codebase
│   ├── common.h              # Shared definitions, limits, and hashing
│   ├── student.h / .c        # Student model & JSON serialization
│   ├── skill.h / .c          # Skill model
│   ├── linked_list.h / .c    # Linked list implementation
│   ├── hash_table.h / .c     # Hash table with separate chaining
│   ├── queue.h / .c          # FIFO Request Queue
│   ├── stack.h / .c          # LIFO Action Stack & Undo
│   ├── graph.h / .c          # Graph adjacency list, BFS, DFS
│   ├── bst.h / .c            # Binary search tree
│   ├── heap.h / .c           # Binary Max-Heap priority queue
│   ├── trie.h / .c           # Trie prefix search
│   ├── dynarray.h / .c       # Dynamic array
│   ├── sorting.h / .c        # Quicksort, Merge sort, Binary search
│   ├── matching.h / .c       # Bipartite graph recommendation engine
│   ├── storage.h / .c        # Text file persistence handler
│   ├── server.h / .c         # Native Winsock HTTP server with CORS
│   └── main.c                # C Server entry point
├── cli/
│   └── skillswap_cli.c       # Interactive terminal viva demonstration tool
├── tests/
│   └── test_data_structures.c # 55 unit test assertions in C
├── data/                     # Persistent database files
│   ├── students.dat          # 16 realistic student profiles
│   ├── skills.dat            # 24 campus skills
│   ├── requests.dat          # Learning exchange requests
│   └── sessions.dat          # Sessions, ratings, and reviews
├── frontend/                 # Presentation layer
│   ├── index.html            # Landing page
│   ├── dashboard.html        # Student dashboard & recommendations
│   ├── marketplace.html      # Skill marketplace & filters
│   ├── profile.html          # Student profile & review history
│   ├── requests.html         # Learning request queue management
│   ├── leaderboard.html      # Max-Heap mentor leaderboard
│   ├── data-structures.html  # Interactive Data Structures Lab
│   ├── css/
│   │   └── style.css         # Modern responsive CSS design system
│   └── js/
│       ├── api.js            # API client bridging frontend to C backend
│       ├── app.js            # App state, user switcher, modals, toasts
│       └── data-structures.js # Lab visualizers and live C benchmark calls
├── Makefile                  # Cross-platform Makefile (Windows & Linux)
├── build.bat                 # 1-Click Windows compiler script
├── run.bat                   # 1-Click Windows runner script
└── README.md                 # Complete documentation
```

---

## ⚡ Quick Start & Running Instructions

### Prerequisites
- **GCC Compiler** (MinGW GCC on Windows, or GCC on Linux/macOS)
- Any modern web browser (Chrome, Edge, Firefox)

### Option 1: One-Click Build & Run (Windows)
1. Double-click `build.bat` (or run in terminal):
   ```powershell
   .\build.bat
   ```
2. Double-click `run.bat` (or run in terminal):
   ```powershell
   .\run.bat
   ```
3. The C server will start on port `8080` and automatically open `http://localhost:8080`.

### Option 2: Using Makefile
```bash
# Build all components (Server, Test Suite, CLI)
make all

# Run automated C unit test suite (55 assertions)
make test

# Start the native C server
./skillswap_server.exe
```

### Option 3: Manual GCC Compilation
```bash
# 1. Compile C HTTP Server
gcc -O2 -Wall backend/*.c -lws2_32 -o skillswap_server.exe

# 2. Compile Test Suite
gcc -O2 -Wall backend/student.c backend/skill.c backend/linked_list.c backend/hash_table.c backend/queue.c backend/stack.c backend/graph.c backend/bst.c backend/heap.c backend/trie.c backend/dynarray.c backend/sorting.c backend/matching.c backend/storage.c tests/test_data_structures.c -o tests/test_data_structures.exe

# 3. Compile Interactive CLI Demo
gcc -O2 -Wall backend/student.c backend/skill.c backend/linked_list.c backend/hash_table.c backend/queue.c backend/stack.c backend/graph.c backend/bst.c backend/heap.c backend/trie.c backend/dynarray.c backend/sorting.c backend/matching.c backend/storage.c cli/skillswap_cli.c -o cli/skillswap_cli.exe
```

---

## 🎓 Evaluator & Viva Examination Guide

To demonstrate the C implementations during your course evaluation:

### 1. Interactive CLI Viva Mode
Run `cli/skillswap_cli.exe` directly in the terminal:
```powershell
.\cli\skillswap_cli.exe
```
This presents an interactive menu to test:
- **Trie prefix matching** on arbitrary input strings.
- **Max-Heap extraction** with score calculation.
- **Graph BFS level discovery** and **DFS cluster exploration**.
- **BST inorder sorted display**.
- **FIFO Request queue manipulation**.

### 2. Live Data Structures Lab in Browser
Navigate to `http://localhost:8080/data-structures.html` to run live interactive benchmarks against the C server, view theoretical complexities, and display exact C source code snippets.

### 3. Automated Test Suite
Run `tests/test_data_structures.exe` to showcase **55 automated unit tests** passing with $100\%$ success rate.
