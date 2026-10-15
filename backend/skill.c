#include "skill.h"

void skill_init(Skill *sk, int id, const char *name, const char *category,
                int mentor_id, const char *mentor_name, const char *mentor_dept,
                int mentor_year, const char *proficiency, const char *format,
                float rating, int completed_sessions, int popularity) {
    if (!sk) return;
    memset(sk, 0, sizeof(Skill));
    sk->id = id;
    strncpy(sk->name, name ? name : "", MAX_SKILL_NAME_LEN - 1);
    strncpy(sk->category, category ? category : "General", MAX_CATEGORY_LEN - 1);
    sk->mentor_id = mentor_id;
    strncpy(sk->mentor_name, mentor_name ? mentor_name : "Anonymous", MAX_NAME_LEN - 1);
    strncpy(sk->mentor_dept, mentor_dept ? mentor_dept : "Computer Science", MAX_DEPT_LEN - 1);
    sk->mentor_year = mentor_year > 0 ? mentor_year : 1;
    strncpy(sk->proficiency, proficiency ? proficiency : "Intermediate", MAX_PROFICIENCY_LEN - 1);
    strncpy(sk->format, format ? format : "1-on-1", MAX_FORMAT_LEN - 1);
    sk->rating = rating >= 0.0f ? rating : 5.0f;
    sk->completed_sessions = completed_sessions;
    sk->popularity = popularity;
}

void skill_to_json(const Skill *sk, char *buffer, size_t buf_size) {
    if (!sk || !buffer || buf_size == 0) return;
    snprintf(buffer, buf_size,
        "{\"id\":%d,\"name\":\"%s\",\"category\":\"%s\",\"mentor_id\":%d,"
        "\"mentor_name\":\"%s\",\"mentor_dept\":\"%s\",\"mentor_year\":%d,"
        "\"proficiency\":\"%s\",\"format\":\"%s\",\"rating\":%.2f,"
        "\"completed_sessions\":%d,\"popularity\":%d}",
        sk->id, sk->name, sk->category, sk->mentor_id,
        sk->mentor_name, sk->mentor_dept, sk->mentor_year,
        sk->proficiency, sk->format, sk->rating,
        sk->completed_sessions, sk->popularity);
}
