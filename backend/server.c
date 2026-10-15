#include "server.h"

#ifdef _WIN32
  #include <winsock2.h>
  #include <windows.h>
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <errno.h>
  #include <signal.h>
  #include <sys/time.h>
  typedef int SOCKET;
  #define INVALID_SOCKET (-1)
  #define SOCKET_ERROR   (-1)
  #define closesocket(s) close(s)
#endif

#define BUFFER_SIZE 65536
#define MAX_RESPONSE_SIZE 262144

static ServerConfig g_config = { .port = 8080, .static_dir = "frontend", .running = false };
static SOCKET g_server_socket = INVALID_SOCKET;

// Application State in C
static Student g_students[MAX_STUDENTS];
static int g_num_students = 0;
static Skill g_skills[MAX_SKILLS];
static int g_num_skills = 0;
static LearningRequest g_requests[MAX_REQUESTS];
static int g_num_requests = 0;
static SessionDynArray *g_sessions = NULL;

static HashTable *g_student_hash = NULL;
static Trie *g_skill_trie = NULL;
static Graph g_graph;
static Queue g_request_queue;
static Stack g_action_stack;

// Helper: Extract JSON string field value
static bool get_json_str(const char *json, const char *key, char *out, size_t max_len) {
    if (!json || !key || !out) return false;
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *pos = strstr(json, pattern);
    if (!pos) return false;
    pos += strlen(pattern);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t')) pos++;
    if (*pos != '"') return false;
    pos++; // Skip opening quote
    size_t i = 0;
    while (*pos && *pos != '"' && i + 1 < max_len) {
        if (*pos == '\\' && *(pos + 1)) pos++; // skip escape
        out[i++] = *pos++;
    }
    out[i] = '\0';
    return true;
}

// Helper: Extract JSON int field value
static int get_json_int(const char *json, const char *key, int default_val) {
    if (!json || !key) return default_val;
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *pos = strstr(json, pattern);
    if (!pos) return default_val;
    pos += strlen(pattern);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t')) pos++;
    return atoi(pos);
}

// Helper: Extract JSON float field value
static float get_json_float(const char *json, const char *key, float default_val) {
    if (!json || !key) return default_val;
    char pattern[128];
    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    const char *pos = strstr(json, pattern);
    if (!pos) return default_val;
    pos += strlen(pattern);
    while (*pos && (*pos == ' ' || *pos == ':' || *pos == '\t')) pos++;
    return (float)atof(pos);
}

// Helper: URL decode
static void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) && (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= ('A' - 10);
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= ('A' - 10);
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

static void send_http_response(SOCKET client, int status_code, const char *status_text, 
                               const char *content_type, const char *body, size_t body_len) {
    char header[1024];
    int header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %d %s\r\n"
        "Content-Type: %s; charset=utf-8\r\n"
        "Content-Length: %lu\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, PUT, DELETE, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type, Authorization, X-Requested-With\r\n"
        "Connection: close\r\n\r\n",
        status_code, status_text, content_type, (unsigned long)body_len);
    
    send(client, header, header_len, 0);
    if (body && body_len > 0) {
        send(client, body, (int)body_len, 0);
    }
}

static const char* get_mime_type(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot) return "text/plain";
    if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) return "text/html";
    if (strcmp(dot, ".css") == 0) return "text/css";
    if (strcmp(dot, ".js") == 0) return "application/javascript";
    if (strcmp(dot, ".json") == 0) return "application/json";
    if (strcmp(dot, ".png") == 0) return "image/png";
    if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(dot, ".svg") == 0) return "image/svg+xml";
    if (strcmp(dot, ".ico") == 0) return "image/x-icon";
    return "text/plain";
}

static bool serve_static_file(SOCKET client, const char *req_path) {
    char file_path[512];
    const char *clean_path = req_path;
    while (*clean_path == '/') clean_path++;
    if (strlen(clean_path) == 0) {
        clean_path = "index.html";
    }
    
    // Remove query params if any
    char pure_path[256];
    strncpy(pure_path, clean_path, sizeof(pure_path) - 1);
    char *q = strchr(pure_path, '?');
    if (q) *q = '\0';
    
    snprintf(file_path, sizeof(file_path), "%s/%s", g_config.static_dir, pure_path);
    
    FILE *f = fopen(file_path, "rb");
    if (!f) return false;
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    char *content = (char *)malloc(file_size);
    if (!content) {
        fclose(f);
        return false;
    }
    
    fread(content, 1, file_size, f);
    fclose(f);
    
    send_http_response(client, 200, "OK", get_mime_type(pure_path), content, file_size);
    free(content);
    return true;
}

// Reload state and rebuild data structures
static void reload_data_structures(void) {
    g_num_students = storage_load_students(g_students, MAX_STUDENTS);
    g_num_skills = storage_load_skills(g_skills, MAX_SKILLS);
    g_num_requests = storage_load_requests(g_requests, MAX_REQUESTS);
    
    if (g_sessions) dynarray_free(g_sessions);
    g_sessions = dynarray_create(64);
    SessionRecord s_temp[MAX_SESSIONS];
    int num_sess = storage_load_sessions(s_temp, MAX_SESSIONS);
    for (int i = 0; i < num_sess; i++) {
        dynarray_push(g_sessions, &s_temp[i]);
    }
    
    // Hash table for students
    if (g_student_hash) ht_free(g_student_hash, false);
    g_student_hash = ht_create(64);
    for (int i = 0; i < g_num_students; i++) {
        ht_insert(g_student_hash, g_students[i].email, &g_students[i]);
        char id_str[16];
        snprintf(id_str, sizeof(id_str), "%d", g_students[i].id);
        ht_insert(g_student_hash, id_str, &g_students[i]);
    }
    
    // Trie for skills
    if (g_skill_trie) trie_free(g_skill_trie);
    g_skill_trie = trie_create();
    for (int i = 0; i < g_num_skills; i++) {
        trie_insert(g_skill_trie, g_skills[i].name, g_skills[i].id, g_skills[i].popularity);
    }
    
    // Graph initialization
    graph_init(&g_graph);
    for (int i = 0; i < g_num_students; i++) {
        graph_find_or_add_vertex(&g_graph, g_students[i].id, g_students[i].name, g_students[i].dept);
    }
    for (int i = 0; i < g_sessions->size; i++) {
        SessionRecord *s = dynarray_get(g_sessions, i);
        if (s) {
            graph_add_edge(&g_graph, s->mentor_id, s->mentee_id, 1, s->skill_name);
            graph_add_edge(&g_graph, s->mentee_id, s->mentor_id, 1, s->skill_name);
        }
    }
    
    // Queue initialization
    queue_init(&g_request_queue);
    for (int i = 0; i < g_num_requests; i++) {
        if (strcmp(g_requests[i].status, "PENDING") == 0) {
            queue_enqueue(&g_request_queue, &g_requests[i]);
        }
    }
}

// Request Router
static void handle_request(SOCKET client, const char *method, const char *path, const char *body) {
    static char resp_buf[MAX_RESPONSE_SIZE];
    resp_buf[0] = '\0';
    
    // Handle OPTIONS (CORS preflight)
    if (strcmp(method, "OPTIONS") == 0) {
        send_http_response(client, 204, "No Content", "text/plain", "", 0);
        return;
    }
    
    // 1. GET /api/status
    if (strcmp(method, "GET") == 0 && strncmp(path, "/api/status", 11) == 0) {
        int ht_count, ht_cap, ht_coll; float ht_lf;
        ht_get_stats(g_student_hash, &ht_count, &ht_cap, &ht_coll, &ht_lf);
        snprintf(resp_buf, sizeof(resp_buf),
            "{\"status\":\"online\",\"server\":\"SkillSwap C Native HTTP Server\","
            "\"students_count\":%d,\"skills_count\":%d,\"requests_count\":%d,"
            "\"sessions_count\":%d,\"queue_pending\":%d,\"stack_actions\":%d,"
            "\"hash_table\":{\"count\":%d,\"capacity\":%d,\"collisions\":%d,\"load_factor\":%.2f},"
            "\"graph\":{\"vertices\":%d}}",
            g_num_students, g_num_skills, g_num_requests,
            dynarray_size(g_sessions), queue_size(&g_request_queue), stack_size(&g_action_stack),
            ht_count, ht_cap, ht_coll, ht_lf, g_graph.num_vertices);
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 2. GET /api/students
    if (strcmp(method, "GET") == 0 && strncmp(path, "/api/students", 13) == 0) {
        // Check for ?id= query param
        const char *id_param = strstr(path, "id=");
        if (id_param) {
            int sid = atoi(id_param + 3);
            char id_key[16];
            snprintf(id_key, sizeof(id_key), "%d", sid);
            Student *s = (Student *)ht_search(g_student_hash, id_key);
            if (s) {
                student_to_json(s, resp_buf, sizeof(resp_buf));
                send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
            } else {
                send_http_response(client, 404, "Not Found", "application/json", "{\"error\":\"Student not found\"}", 28);
            }
            return;
        }
        
        // Return all students
        snprintf(resp_buf, sizeof(resp_buf), "[");
        for (int i = 0; i < g_num_students; i++) {
            char item[2048];
            student_to_json(&g_students[i], item, sizeof(item));
            if (i > 0) strncat(resp_buf, ",", sizeof(resp_buf) - strlen(resp_buf) - 1);
            strncat(resp_buf, item, sizeof(resp_buf) - strlen(resp_buf) - 1);
        }
        strncat(resp_buf, "]", sizeof(resp_buf) - strlen(resp_buf) - 1);
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 3. POST /api/auth/register
    if (strcmp(method, "POST") == 0 && strstr(path, "/api/auth/register")) {
        char name[MAX_NAME_LEN] = "", email[MAX_EMAIL_LEN] = "", pass[64] = "", dept[MAX_DEPT_LEN] = "", bio[MAX_BIO_LEN] = "";
        get_json_str(body, "name", name, sizeof(name));
        get_json_str(body, "email", email, sizeof(email));
        get_json_str(body, "password", pass, sizeof(pass));
        get_json_str(body, "dept", dept, sizeof(dept));
        get_json_str(body, "bio", bio, sizeof(bio));
        int year = get_json_int(body, "year", 1);
        
        if (strlen(name) == 0 || strlen(email) == 0 || strlen(pass) == 0) {
            send_http_response(client, 400, "Bad Request", "application/json", "{\"error\":\"Missing required fields\"}", 34);
            return;
        }
        
        // Check duplicate email using Hash Table in O(1)
        if (ht_search(g_student_hash, email) != NULL) {
            send_http_response(client, 409, "Conflict", "application/json", "{\"error\":\"Email already registered\"}", 35);
            return;
        }
        
        int new_id = g_num_students + 1;
        Student new_s;
        student_init(&new_s, new_id, name, email, pass, dept, year, bio, "");
        g_students[g_num_students++] = new_s;
        
        // Push to Stack for Undo capability
        ActionEntry act = { .id = stack_size(&g_action_stack) + 1, .type = ACTION_UPDATE_BIO, .student_id = new_id };
        snprintf(act.details, sizeof(act.details), "Registered student %s", name);
        stack_push(&g_action_stack, &act);
        
        storage_save_students(g_students, g_num_students);
        reload_data_structures();
        
        student_to_json(&new_s, resp_buf, sizeof(resp_buf));
        send_http_response(client, 201, "Created", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 4. POST /api/auth/login
    if (strcmp(method, "POST") == 0 && strstr(path, "/api/auth/login")) {
        char email[MAX_EMAIL_LEN] = "", pass[64] = "";
        get_json_str(body, "email", email, sizeof(email));
        get_json_str(body, "password", pass, sizeof(pass));
        
        // Look up via Hash Table in O(1)
        Student *s = (Student *)ht_search(g_student_hash, email);
        if (s && student_verify_password(s, pass)) {
            char user_json[2048];
            student_to_json(s, user_json, sizeof(user_json));
            snprintf(resp_buf, sizeof(resp_buf), "{\"success\":true,\"user\":%s}", user_json);
            send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        } else {
            send_http_response(client, 401, "Unauthorized", "application/json", "{\"error\":\"Invalid email or password\"}", 37);
        }
        return;
    }
    
    // 5. GET /api/skills
    if (strcmp(method, "GET") == 0 && strncmp(path, "/api/skills", 11) == 0) {
        snprintf(resp_buf, sizeof(resp_buf), "[");
        for (int i = 0; i < g_num_skills; i++) {
            char item[512];
            skill_to_json(&g_skills[i], item, sizeof(item));
            if (i > 0) strncat(resp_buf, ",", sizeof(resp_buf) - strlen(resp_buf) - 1);
            strncat(resp_buf, item, sizeof(resp_buf) - strlen(resp_buf) - 1);
        }
        strncat(resp_buf, "]", sizeof(resp_buf) - strlen(resp_buf) - 1);
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 6. POST /api/skills (Add offered skill)
    if (strcmp(method, "POST") == 0 && strstr(path, "/api/skills")) {
        int mentor_id = get_json_int(body, "mentor_id", 1);
        char skill_name[MAX_SKILL_NAME_LEN] = "", category[MAX_CATEGORY_LEN] = "General";
        char proficiency[MAX_PROFICIENCY_LEN] = "Intermediate", format[MAX_FORMAT_LEN] = "1-on-1";
        get_json_str(body, "name", skill_name, sizeof(skill_name));
        get_json_str(body, "category", category, sizeof(category));
        get_json_str(body, "proficiency", proficiency, sizeof(proficiency));
        get_json_str(body, "format", format, sizeof(format));
        
        char id_str[16]; snprintf(id_str, sizeof(id_str), "%d", mentor_id);
        Student *s = (Student *)ht_search(g_student_hash, id_str);
        if (s) {
            student_add_skill_offered(s, skill_name, category, proficiency, format);
        }
        
        int new_skill_id = g_num_skills + 1;
        Skill sk;
        skill_init(&sk, new_skill_id, skill_name, category, mentor_id,
                   s ? s->name : "Mentor", s ? s->dept : "General",
                   s ? s->year : 1, proficiency, format, s ? s->rating : 5.0f,
                   s ? s->completed_sessions : 0, 50);
        g_skills[g_num_skills++] = sk;
        
        // Push Action to Stack
        ActionEntry act = { .id = stack_size(&g_action_stack) + 1, .type = ACTION_ADD_SKILL, .student_id = mentor_id, .target_id = new_skill_id };
        snprintf(act.details, sizeof(act.details), "Added skill '%s'", skill_name);
        stack_push(&g_action_stack, &act);
        
        storage_save_skills(g_skills, g_num_skills);
        storage_save_students(g_students, g_num_students);
        reload_data_structures();
        
        skill_to_json(&sk, resp_buf, sizeof(resp_buf));
        send_http_response(client, 201, "Created", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 7. GET /api/search?q=prefix (Trie prefix autocomplete)
    if (strcmp(method, "GET") == 0 && strstr(path, "/api/search")) {
        char prefix[64] = "";
        const char *q_param = strstr(path, "q=");
        if (q_param) {
            url_decode(prefix, q_param + 2);
            char *amp = strchr(prefix, '&');
            if (amp) *amp = '\0';
        }
        
        char results[10][MAX_SKILL_NAME_LEN];
        int count = trie_autocomplete(g_skill_trie, prefix, results, 10);
        
        snprintf(resp_buf, sizeof(resp_buf), "{\"prefix\":\"%s\",\"algorithm\":\"Trie (Prefix Tree)\",\"count\":%d,\"results\":[",
                 prefix, count);
        for (int i = 0; i < count; i++) {
            char item[128];
            snprintf(item, sizeof(item), "%s\"%s\"", i > 0 ? "," : "", results[i]);
            strncat(resp_buf, item, sizeof(resp_buf) - strlen(resp_buf) - 1);
        }
        strncat(resp_buf, "]}", sizeof(resp_buf) - strlen(resp_buf) - 1);
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 8. GET /api/recommendations?student_id=id (Graph Matching)
    if (strcmp(method, "GET") == 0 && strstr(path, "/api/recommendations")) {
        int target_id = 1;
        const char *sid_param = strstr(path, "student_id=");
        if (sid_param) target_id = atoi(sid_param + 11);
        
        Student *target = NULL;
        for (int i = 0; i < g_num_students; i++) {
            if (g_students[i].id == target_id) { target = &g_students[i]; break; }
        }
        if (!target && g_num_students > 0) target = &g_students[0];
        
        MatchRecommendation recs[10];
        int count = find_recommendations_for_student(target, g_students, g_num_students, &g_graph, recs, 10);
        
        snprintf(resp_buf, sizeof(resp_buf), "{\"target_id\":%d,\"algorithm\":\"Bipartite Graph Skill-Swap Matching\",\"count\":%d,\"recommendations\":[",
                 target ? target->id : 0, count);
        for (int i = 0; i < count; i++) {
            char item[512];
            recommendation_to_json(&recs[i], item, sizeof(item));
            if (i > 0) strncat(resp_buf, ",", sizeof(resp_buf) - strlen(resp_buf) - 1);
            strncat(resp_buf, item, sizeof(resp_buf) - strlen(resp_buf) - 1);
        }
        strncat(resp_buf, "]}", sizeof(resp_buf) - strlen(resp_buf) - 1);
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 9. GET /api/leaderboard (Max-Heap Priority Queue)
    if (strcmp(method, "GET") == 0 && strstr(path, "/api/leaderboard")) {
        MaxHeap heap;
        heap_init(&heap);
        
        for (int i = 0; i < g_num_students; i++) {
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
        
        snprintf(resp_buf, sizeof(resp_buf), "{\"algorithm\":\"Max-Heap Priority Queue\",\"formula\":\"(Rating*20)+(Sessions*10)+(Reviews*5)\",\"leaderboard\":[");
        int rank = 1;
        MentorScore top;
        bool first = true;
        while (heap_extract_max(&heap, &top)) {
            char item[512];
            snprintf(item, sizeof(item),
                "%s{\"rank\":%d,\"student_id\":%d,\"name\":\"%s\",\"dept\":\"%s\",\"year\":%d,"
                "\"top_skill\":\"%s\",\"rating\":%.2f,\"completed_sessions\":%d,"
                "\"reviews_count\":%d,\"score\":%.1f}",
                first ? "" : ",", rank++, top.student_id, top.name, top.dept, top.year,
                top.top_skill, top.rating, top.completed_sessions, top.reviews_count, top.score);
            first = false;
            strncat(resp_buf, item, sizeof(resp_buf) - strlen(resp_buf) - 1);
        }
        strncat(resp_buf, "]}", sizeof(resp_buf) - strlen(resp_buf) - 1);
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 10. GET /api/requests
    if (strcmp(method, "GET") == 0 && strncmp(path, "/api/requests", 13) == 0) {
        int sid = -1;
        const char *sid_param = strstr(path, "student_id=");
        if (sid_param) sid = atoi(sid_param + 11);
        
        snprintf(resp_buf, sizeof(resp_buf), "[");
        bool first = true;
        for (int i = 0; i < g_num_requests; i++) {
            if (sid != -1 && g_requests[i].requester_id != sid && g_requests[i].mentor_id != sid) {
                continue;
            }
            char item[512];
            request_to_json(&g_requests[i], item, sizeof(item));
            if (!first) strncat(resp_buf, ",", sizeof(resp_buf) - strlen(resp_buf) - 1);
            first = false;
            strncat(resp_buf, item, sizeof(resp_buf) - strlen(resp_buf) - 1);
        }
        strncat(resp_buf, "]", sizeof(resp_buf) - strlen(resp_buf) - 1);
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 11. POST /api/requests (Enqueue new request into FIFO Queue)
    if (strcmp(method, "POST") == 0 && strstr(path, "/api/requests") && !strstr(path, "/action")) {
        LearningRequest req;
        memset(&req, 0, sizeof(LearningRequest));
        req.id = g_num_requests + 1;
        req.requester_id = get_json_int(body, "requester_id", 1);
        req.mentor_id = get_json_int(body, "mentor_id", 2);
        get_json_str(body, "requester_name", req.requester_name, sizeof(req.requester_name));
        get_json_str(body, "mentor_name", req.mentor_name, sizeof(req.mentor_name));
        get_json_str(body, "skill_name", req.skill_name, sizeof(req.skill_name));
        get_json_str(body, "message", req.message, sizeof(req.message));
        get_json_str(body, "proposed_time", req.proposed_time, sizeof(req.proposed_time));
        strncpy(req.status, "PENDING", sizeof(req.status) - 1);
        
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
        snprintf(req.created_at, sizeof(req.created_at), "%04d-%02d-%02d %02d:%02d",
                 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
        
        // Prevent duplicate pending requests
        for (int i = 0; i < g_num_requests; i++) {
            if (g_requests[i].requester_id == req.requester_id &&
                g_requests[i].mentor_id == req.mentor_id &&
                strcasecmp(g_requests[i].skill_name, req.skill_name) == 0 &&
                strcmp(g_requests[i].status, "PENDING") == 0) {
                send_http_response(client, 400, "Bad Request", "application/json", "{\"error\":\"A pending request already exists for this skill\"}", 58);
                return;
            }
        }
        
        // Enqueue into FIFO Queue
        queue_enqueue(&g_request_queue, &req);
        g_requests[g_num_requests++] = req;
        
        // Push Action to Stack
        ActionEntry act = { .id = stack_size(&g_action_stack) + 1, .type = ACTION_SEND_REQUEST, .student_id = req.requester_id, .target_id = req.id };
        snprintf(act.details, sizeof(act.details), "Sent request for '%s'", req.skill_name);
        stack_push(&g_action_stack, &act);
        
        storage_save_requests(g_requests, g_num_requests);
        
        request_to_json(&req, resp_buf, sizeof(resp_buf));
        send_http_response(client, 201, "Created", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 12. POST /api/requests/action (Accept, Reject, Complete)
    if (strcmp(method, "POST") == 0 && strstr(path, "/api/requests/action")) {
        int req_id = get_json_int(body, "request_id", 0);
        char action[32] = "";
        get_json_str(body, "action", action, sizeof(action));
        
        LearningRequest *r = NULL;
        for (int i = 0; i < g_num_requests; i++) {
            if (g_requests[i].id == req_id) { r = &g_requests[i]; break; }
        }
        if (!r) {
            send_http_response(client, 404, "Not Found", "application/json", "{\"error\":\"Request not found\"}", 28);
            return;
        }
        
        if (strcasecmp(action, "ACCEPT") == 0) {
            strncpy(r->status, "ACCEPTED", sizeof(r->status) - 1);
            // Schedule session record
            SessionRecord sess;
            memset(&sess, 0, sizeof(SessionRecord));
            sess.id = g_sessions->size + 1;
            sess.request_id = r->id;
            sess.mentor_id = r->mentor_id;
            strncpy(sess.mentor_name, r->mentor_name, MAX_NAME_LEN - 1);
            sess.mentee_id = r->requester_id;
            strncpy(sess.mentee_name, r->requester_name, MAX_NAME_LEN - 1);
            strncpy(sess.skill_name, r->skill_name, MAX_SKILL_NAME_LEN - 1);
            strncpy(sess.date, "2026-10-15", 31);
            sess.duration_mins = 60;
            strncpy(sess.status, "SCHEDULED", sizeof(sess.status) - 1);
            dynarray_push(g_sessions, &sess);
            storage_save_sessions(g_sessions->data, g_sessions->size);
        } else if (strcasecmp(action, "REJECT") == 0) {
            strncpy(r->status, "REJECTED", sizeof(r->status) - 1);
        } else if (strcasecmp(action, "COMPLETE") == 0) {
            strncpy(r->status, "COMPLETED", sizeof(r->status) - 1);
        }
        
        storage_save_requests(g_requests, g_num_requests);
        reload_data_structures();
        
        request_to_json(r, resp_buf, sizeof(resp_buf));
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 13. POST /api/undo (LIFO Stack Undo)
    if (strcmp(method, "POST") == 0 && strstr(path, "/api/undo")) {
        if (stack_is_empty(&g_action_stack)) {
            send_http_response(client, 400, "Bad Request", "application/json", "{\"error\":\"Undo stack is empty\"}", 30);
            return;
        }
        ActionEntry popped;
        stack_pop(&g_action_stack, &popped);
        
        snprintf(resp_buf, sizeof(resp_buf),
            "{\"success\":true,\"action_undone\":{\"id\":%d,\"type\":%d,\"details\":\"%s\"},\"remaining_stack_size\":%d}",
            popped.id, popped.type, popped.details, stack_size(&g_action_stack));
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 14. GET /api/sessions
    if (strcmp(method, "GET") == 0 && strncmp(path, "/api/sessions", 13) == 0) {
        int sid = -1;
        const char *sid_param = strstr(path, "student_id=");
        if (sid_param) sid = atoi(sid_param + 11);
        
        snprintf(resp_buf, sizeof(resp_buf), "[");
        bool first = true;
        for (int i = 0; i < g_sessions->size; i++) {
            SessionRecord *s = dynarray_get(g_sessions, i);
            if (!s) continue;
            if (sid != -1 && s->mentee_id != sid && s->mentor_id != sid) continue;
            char item[512];
            session_to_json(s, item, sizeof(item));
            if (!first) strncat(resp_buf, ",", sizeof(resp_buf) - strlen(resp_buf) - 1);
            first = false;
            strncat(resp_buf, item, sizeof(resp_buf) - strlen(resp_buf) - 1);
        }
        strncat(resp_buf, "]", sizeof(resp_buf) - strlen(resp_buf) - 1);
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 15. POST /api/sessions/review (Submit rating & review)
    if (strcmp(method, "POST") == 0 && strstr(path, "/api/sessions/review")) {
        int sess_id = get_json_int(body, "session_id", 0);
        float rating = get_json_float(body, "rating", 5.0f);
        char feedback[MAX_MESSAGE_LEN] = "";
        get_json_str(body, "feedback", feedback, sizeof(feedback));
        
        SessionRecord *sess = NULL;
        for (int i = 0; i < g_sessions->size; i++) {
            SessionRecord *s = dynarray_get(g_sessions, i);
            if (s && s->id == sess_id) { sess = s; break; }
        }
        if (!sess) {
            send_http_response(client, 404, "Not Found", "application/json", "{\"error\":\"Session not found\"}", 27);
            return;
        }
        if (sess->has_review) {
            send_http_response(client, 400, "Bad Request", "application/json", "{\"error\":\"Review already submitted for this session\"}", 52);
            return;
        }
        
        sess->has_review = true;
        sess->rating = rating;
        strncpy(sess->feedback, feedback, sizeof(sess->feedback) - 1);
        strncpy(sess->status, "COMPLETED", sizeof(sess->status) - 1);
        
        // Update mentor's average rating in Student record
        for (int i = 0; i < g_num_students; i++) {
            if (g_students[i].id == sess->mentor_id) {
                student_record_session_rating(&g_students[i], rating);
                break;
            }
        }
        
        storage_save_sessions(g_sessions->data, g_sessions->size);
        storage_save_students(g_students, g_num_students);
        reload_data_structures();
        
        session_to_json(sess, resp_buf, sizeof(resp_buf));
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 16. GET /api/graph (Graph nodes and edges for Lab Visualization)
    if (strcmp(method, "GET") == 0 && strstr(path, "/api/graph")) {
        snprintf(resp_buf, sizeof(resp_buf), "{\"nodes\":[");
        for (int i = 0; i < g_graph.num_vertices; i++) {
            char node_item[256];
            snprintf(node_item, sizeof(node_item), "%s{\"id\":%d,\"name\":\"%s\",\"dept\":\"%s\"}",
                     i > 0 ? "," : "", g_graph.vertices[i].student_id,
                     g_graph.vertices[i].name, g_graph.vertices[i].dept);
            strncat(resp_buf, node_item, sizeof(resp_buf) - strlen(resp_buf) - 1);
        }
        strncat(resp_buf, "],\"edges\":[", sizeof(resp_buf) - strlen(resp_buf) - 1);
        bool first_edge = true;
        for (int i = 0; i < g_graph.num_vertices; i++) {
            GraphEdge *curr = g_graph.vertices[i].head;
            while (curr) {
                char edge_item[256];
                snprintf(edge_item, sizeof(edge_item), "%s{\"source\":%d,\"target\":%d,\"weight\":%d,\"skill\":\"%s\"}",
                         first_edge ? "" : ",", g_graph.vertices[i].student_id,
                         curr->dest, curr->weight, curr->skill_exchanged);
                first_edge = false;
                strncat(resp_buf, edge_item, sizeof(resp_buf) - strlen(resp_buf) - 1);
                curr = curr->next;
            }
        }
        strncat(resp_buf, "]}", sizeof(resp_buf) - strlen(resp_buf) - 1);
        send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
        return;
    }
    
    // 17. POST / GET /api/ds-demo (Interactive live C Data Structure test execution)
    if (strstr(path, "/api/ds-demo")) {
        char ds_type[32] = "";
        get_json_str(body, "type", ds_type, sizeof(ds_type));
        if (strlen(ds_type) == 0) {
            const char *tp = strstr(path, "type=");
            if (tp) sscanf(tp + 5, "%31[^& ]", ds_type);
        }
        if (strlen(ds_type) == 0) {
            if (strstr(body, "bst") || strstr(path, "bst")) strcpy(ds_type, "bst");
            else if (strstr(body, "heap") || strstr(path, "heap")) strcpy(ds_type, "heap");
            else if (strstr(body, "quicksort") || strstr(path, "quicksort")) strcpy(ds_type, "quicksort");
        }
        
        // High-precision timing for benchmarks
#ifdef _WIN32
        LARGE_INTEGER freq, start, end;
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&start);
        #define ELAPSED_US() ((double)(end.QuadPart - start.QuadPart) * 1000000.0 / freq.QuadPart)
        #define MARK_END() QueryPerformanceCounter(&end)
#else
        struct timeval tv_start, tv_end;
        gettimeofday(&tv_start, NULL);
        #define ELAPSED_US() (((double)(tv_end.tv_sec - tv_start.tv_sec) * 1000000.0) + (double)(tv_end.tv_usec - tv_start.tv_usec))
        #define MARK_END() gettimeofday(&tv_end, NULL)
#endif
        
        if (strcasecmp(ds_type, "bst") == 0) {
            BST tree;
            bst_init(&tree);
            for (int i = 0; i < g_num_students; i++) {
                bst_add(&tree, g_students[i].rating, g_students[i].id, g_students[i].name,
                        g_students[i].skills_offered_count > 0 ? g_students[i].skills_offered[0].skill_name : "General");
            }
            BSTNode *in_nodes[MAX_STUDENTS]; int count = 0;
            bst_inorder(tree.root, in_nodes, MAX_STUDENTS, &count);
            MARK_END();
            double us = ELAPSED_US();
            
            snprintf(resp_buf, sizeof(resp_buf),
                "{\"type\":\"bst\",\"count\":%d,\"execution_us\":%.2f,\"time_complexity\":\"O(N) traversal, O(log N) search\","
                "\"inorder_traversal\":[", count, us);
            for (int i = 0; i < count; i++) {
                char item[256];
                snprintf(item, sizeof(item), "%s{\"key\":%.2f,\"id\":%d,\"name\":\"%s\",\"skill\":\"%s\"}",
                         i > 0 ? "," : "", in_nodes[i]->key, in_nodes[i]->student_id,
                         in_nodes[i]->student_name, in_nodes[i]->primary_skill);
                strncat(resp_buf, item, sizeof(resp_buf) - strlen(resp_buf) - 1);
            }
            strncat(resp_buf, "]}", sizeof(resp_buf) - strlen(resp_buf) - 1);
            bst_free(&tree);
            send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
            return;
        } else if (strcasecmp(ds_type, "heap") == 0) {
            MaxHeap h;
            heap_init(&h);
            for (int i = 0; i < g_num_students; i++) {
                MentorScore ms;
                ms.student_id = g_students[i].id;
                strncpy(ms.name, g_students[i].name, MAX_NAME_LEN - 1);
                strncpy(ms.dept, g_students[i].dept, MAX_DEPT_LEN - 1);
                ms.year = g_students[i].year;
                strncpy(ms.top_skill, g_students[i].skills_offered_count > 0 ? g_students[i].skills_offered[0].skill_name : "General", MAX_SKILL_NAME_LEN - 1);
                ms.rating = g_students[i].rating;
                ms.completed_sessions = g_students[i].completed_sessions;
                ms.reviews_count = g_students[i].rating_count;
                ms.score = heap_calculate_score(ms.rating, ms.completed_sessions, ms.reviews_count);
                heap_insert(&h, &ms);
            }
            MARK_END();
            double us = ELAPSED_US();
            snprintf(resp_buf, sizeof(resp_buf),
                "{\"type\":\"heap\",\"size\":%d,\"execution_us\":%.2f,\"time_complexity\":\"O(log N) insert/extract, O(1) peek\","
                "\"top_mentor\":{\"name\":\"%s\",\"score\":%.1f,\"rating\":%.2f}}",
                h.size, us, h.size > 0 ? h.data[0].name : "None",
                h.size > 0 ? h.data[0].score : 0, h.size > 0 ? h.data[0].rating : 0);
            send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
            return;
        } else if (strcasecmp(ds_type, "quicksort") == 0) {
            Skill temp_skills[MAX_SKILLS];
            memcpy(temp_skills, g_skills, sizeof(Skill) * g_num_skills);
            quicksort_skills_by_rating(temp_skills, 0, g_num_skills - 1);
            MARK_END();
            double us = ELAPSED_US();
            snprintf(resp_buf, sizeof(resp_buf),
                "{\"type\":\"quicksort\",\"count\":%d,\"execution_us\":%.2f,\"time_complexity\":\"O(N log N) avg, O(N^2) worst\","
                "\"top_sorted\":{\"name\":\"%s\",\"rating\":%.2f}}",
                g_num_skills, us, temp_skills[0].name, temp_skills[0].rating);
            send_http_response(client, 200, "OK", "application/json", resp_buf, strlen(resp_buf));
            return;
        }
#undef ELAPSED_US
#undef MARK_END
        
        send_http_response(client, 400, "Bad Request", "application/json", "{\"error\":\"Unknown DS type\"}", 26);
        return;
    }
    
    // Otherwise, try serving static frontend files
    if (serve_static_file(client, path)) {
        return;
    }
    
    // If not found, 404
    send_http_response(client, 404, "Not Found", "text/plain", "404 Not Found in SkillSwap C Server", 35);
}

void server_init(int port, const char *static_dir) {
    g_config.port = port > 0 ? port : 8080;
    g_config.static_dir = static_dir ? static_dir : "frontend";
    g_config.running = true;
    
    // Load data and seed structures
    storage_set_dir("data");
    reload_data_structures();
}

int server_run(void) {
#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("[ERROR] Failed to initialize Winsock.\n");
        return 1;
    }
#else
    // Ignore SIGPIPE on macOS/Linux so send() on closed connections doesn't crash
    signal(SIGPIPE, SIG_IGN);
#endif
    
    g_server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (g_server_socket == INVALID_SOCKET) {
        printf("[ERROR] Failed to create socket.\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    int opt = 1;
    setsockopt(g_server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));
    
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(g_config.port);
    
    if (bind(g_server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
#ifdef _WIN32
        printf("[ERROR] Failed to bind to port %d. Error: %d\n", g_config.port, WSAGetLastError());
#else
        printf("[ERROR] Failed to bind to port %d: %s\n", g_config.port, strerror(errno));
#endif
        closesocket(g_server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    if (listen(g_server_socket, 10) == SOCKET_ERROR) {
        printf("[ERROR] Failed to listen on socket.\n");
        closesocket(g_server_socket);
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }
    
    printf("=======================================================================\n");
    printf("   SkillSwap C Native HTTP REST Server\n");
    printf("   Port: %d | Frontend: %s\n", g_config.port, g_config.static_dir);
    printf("   Local URL: http://localhost:%d\n", g_config.port);
    printf("   Data Structures: Linked List, Hash Table, Queue, Stack, Graph,\n");
    printf("                    BST, Max-Heap, Trie, Dynamic Array, Quicksort/BSearch\n");
    printf("=======================================================================\n");
    printf("[READY] Server is actively accepting connections...\n");
    
    while (g_config.running) {
        struct sockaddr_in client_addr;
#ifdef _WIN32
        int client_len = sizeof(client_addr);
#else
        socklen_t client_len = sizeof(client_addr);
#endif
        SOCKET client = accept(g_server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client == INVALID_SOCKET) {
            if (!g_config.running) break;
            continue;
        }
        
        char buffer[BUFFER_SIZE];
        int bytes_received = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            
            char method[16] = "", path[256] = "";
            sscanf(buffer, "%15s %255s", method, path);
            
            // Find body if any (after \r\n\r\n)
            char *body = strstr(buffer, "\r\n\r\n");
            if (body) {
                body += 4;
                
                // Read full body if Content-Length specified
                char *cl = strstr(buffer, "Content-Length:");
                if (!cl) cl = strstr(buffer, "content-length:");
                if (cl) {
                    int content_len = atoi(cl + 15);
                    int current_body_len = bytes_received - (int)(body - buffer);
                    while (current_body_len < content_len && bytes_received < BUFFER_SIZE - 1) {
                        int n = recv(client, buffer + bytes_received, BUFFER_SIZE - 1 - bytes_received, 0);
                        if (n <= 0) break;
                        bytes_received += n;
                        buffer[bytes_received] = '\0';
                        current_body_len += n;
                    }
                }
            } else {
                body = "";
            }
            
            handle_request(client, method, path, body);
        }
        
        closesocket(client);
    }
    
    closesocket(g_server_socket);
#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}

void server_stop(void) {
    g_config.running = false;
    if (g_server_socket != INVALID_SOCKET) {
        closesocket(g_server_socket);
        g_server_socket = INVALID_SOCKET;
    }
}
