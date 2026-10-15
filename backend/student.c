#include "student.h"

void student_init(Student *s, int id, const char *name, const char *email, 
                  const char *password, const char *dept, int year, 
                  const char *bio, const char *avatar) {
    if (!s) return;
    memset(s, 0, sizeof(Student));
    s->id = id;
    strncpy(s->name, name ? name : "", MAX_NAME_LEN - 1);
    strncpy(s->email, email ? email : "", MAX_EMAIL_LEN - 1);
    s->password_hash = password ? hash_password(password) : 0;
    strncpy(s->dept, dept ? dept : "Computer Science", MAX_DEPT_LEN - 1);
    s->year = year > 0 ? year : 1;
    strncpy(s->bio, bio ? bio : "", MAX_BIO_LEN - 1);
    strncpy(s->avatar, avatar ? avatar : "https://api.dicebear.com/7.x/avataaars/svg?seed=Felix", 127);
    s->rating = 5.0f;
    s->rating_count = 0;
    s->completed_sessions = 0;
    s->skills_offered_count = 0;
    s->skills_wanted_count = 0;
}

bool student_verify_password(const Student *s, const char *password) {
    if (!s || !password) return false;
    return s->password_hash == hash_password(password);
}

bool student_add_skill_offered(Student *s, const char *skill_name, 
                               const char *category, const char *proficiency, 
                               const char *format) {
    if (!s || !skill_name || s->skills_offered_count >= MAX_SKILLS_PER_STUDENT) return false;
    
    // Check if already offered
    for (int i = 0; i < s->skills_offered_count; i++) {
        if (strcasecmp(s->skills_offered[i].skill_name, skill_name) == 0) {
            // Update existing
            if (category) strncpy(s->skills_offered[i].category, category, MAX_CATEGORY_LEN - 1);
            if (proficiency) strncpy(s->skills_offered[i].proficiency, proficiency, MAX_PROFICIENCY_LEN - 1);
            if (format) strncpy(s->skills_offered[i].format, format, MAX_FORMAT_LEN - 1);
            return true;
        }
    }
    
    SkillOffered *entry = &s->skills_offered[s->skills_offered_count];
    strncpy(entry->skill_name, skill_name, MAX_SKILL_NAME_LEN - 1);
    strncpy(entry->category, category ? category : "General", MAX_CATEGORY_LEN - 1);
    strncpy(entry->proficiency, proficiency ? proficiency : "Intermediate", MAX_PROFICIENCY_LEN - 1);
    strncpy(entry->format, format ? format : "1-on-1", MAX_FORMAT_LEN - 1);
    s->skills_offered_count++;
    return true;
}

bool student_remove_skill_offered(Student *s, const char *skill_name) {
    if (!s || !skill_name) return false;
    int found_idx = -1;
    for (int i = 0; i < s->skills_offered_count; i++) {
        if (strcasecmp(s->skills_offered[i].skill_name, skill_name) == 0) {
            found_idx = i;
            break;
        }
    }
    if (found_idx == -1) return false;
    
    for (int i = found_idx; i < s->skills_offered_count - 1; i++) {
        s->skills_offered[i] = s->skills_offered[i + 1];
    }
    s->skills_offered_count--;
    return true;
}

bool student_add_skill_wanted(Student *s, const char *skill_name) {
    if (!s || !skill_name || s->skills_wanted_count >= MAX_SKILLS_PER_STUDENT) return false;
    for (int i = 0; i < s->skills_wanted_count; i++) {
        if (strcasecmp(s->skills_wanted[i], skill_name) == 0) return true; // already exists
    }
    strncpy(s->skills_wanted[s->skills_wanted_count], skill_name, MAX_SKILL_NAME_LEN - 1);
    s->skills_wanted_count++;
    return true;
}

bool student_remove_skill_wanted(Student *s, const char *skill_name) {
    if (!s || !skill_name) return false;
    int found_idx = -1;
    for (int i = 0; i < s->skills_wanted_count; i++) {
        if (strcasecmp(s->skills_wanted[i], skill_name) == 0) {
            found_idx = i;
            break;
        }
    }
    if (found_idx == -1) return false;
    
    for (int i = found_idx; i < s->skills_wanted_count - 1; i++) {
        strncpy(s->skills_wanted[i], s->skills_wanted[i + 1], MAX_SKILL_NAME_LEN - 1);
    }
    s->skills_wanted_count--;
    return true;
}

void student_record_session_rating(Student *s, float new_rating) {
    if (!s) return;
    if (s->rating_count == 0) {
        s->rating = new_rating;
    } else {
        s->rating = ((s->rating * s->rating_count) + new_rating) / (s->rating_count + 1);
    }
    s->rating_count++;
    s->completed_sessions++;
}

// Helper to escape JSON strings
static void escape_json_str(const char *src, char *dest, size_t max_len) {
    size_t j = 0;
    for (size_t i = 0; src[i] && j + 2 < max_len; i++) {
        if (src[i] == '"' || src[i] == '\\') {
            dest[j++] = '\\';
            dest[j++] = src[i];
        } else if (src[i] == '\n') {
            dest[j++] = '\\';
            dest[j++] = 'n';
        } else if (src[i] == '\r') {
            // skip
        } else {
            dest[j++] = src[i];
        }
    }
    dest[j] = '\0';
}

void student_to_json(const Student *s, char *buffer, size_t buf_size) {
    if (!s || !buffer || buf_size == 0) return;
    
    char clean_name[MAX_NAME_LEN * 2];
    char clean_bio[MAX_BIO_LEN * 2];
    char clean_dept[MAX_DEPT_LEN * 2];
    
    escape_json_str(s->name, clean_name, sizeof(clean_name));
    escape_json_str(s->bio, clean_bio, sizeof(clean_bio));
    escape_json_str(s->dept, clean_dept, sizeof(clean_dept));
    
    int written = snprintf(buffer, buf_size, 
        "{\"id\":%d,\"name\":\"%s\",\"email\":\"%s\",\"dept\":\"%s\",\"year\":%d,"
        "\"bio\":\"%s\",\"avatar\":\"%s\",\"rating\":%.2f,\"rating_count\":%d,"
        "\"completed_sessions\":%d,\"skills_offered\":[",
        s->id, clean_name, s->email, clean_dept, s->year,
        clean_bio, s->avatar, s->rating, s->rating_count, s->completed_sessions);
    
    for (int i = 0; i < s->skills_offered_count && written < (int)buf_size - 100; i++) {
        char item[256];
        snprintf(item, sizeof(item), "%s{\"name\":\"%s\",\"category\":\"%s\",\"proficiency\":\"%s\",\"format\":\"%s\"}",
            i > 0 ? "," : "",
            s->skills_offered[i].skill_name,
            s->skills_offered[i].category,
            s->skills_offered[i].proficiency,
            s->skills_offered[i].format);
        strncat(buffer, item, buf_size - strlen(buffer) - 1);
    }
    
    strncat(buffer, "],\"skills_wanted\":[", buf_size - strlen(buffer) - 1);
    for (int i = 0; i < s->skills_wanted_count && strlen(buffer) < buf_size - 100; i++) {
        char item[128];
        snprintf(item, sizeof(item), "%s\"%s\"", i > 0 ? "," : "", s->skills_wanted[i]);
        strncat(buffer, item, buf_size - strlen(buffer) - 1);
    }
    
    strncat(buffer, "]}", buf_size - strlen(buffer) - 1);
}
