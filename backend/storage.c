#include "storage.h"

static char g_data_dir[256] = "data";

void storage_set_dir(const char *dir_path) {
    if (dir_path && strlen(dir_path) > 0) {
        strncpy(g_data_dir, dir_path, sizeof(g_data_dir) - 1);
        g_data_dir[sizeof(g_data_dir) - 1] = '\0';
    }
}

static void get_filepath(char *dest, size_t dest_size, const char *filename) {
    snprintf(dest, dest_size, "%s/%s", g_data_dir, filename);
}

bool storage_save_students(const Student *arr, int count) {
    if (!arr || count < 0) return false;
    char path[256];
    get_filepath(path, sizeof(path), "students.dat");
    FILE *f = fopen(path, "w");
    if (!f) return false;

    // Header comment
    fprintf(f, "# SkillSwap Students Record File (Pipe-Delimited)\n");
    fprintf(f, "# ID|Name|Email|PasswordHash|Dept|Year|Rating|RatingCount|CompletedSessions|Avatar|Bio|OfferedSkills|WantedSkills\n");

    for (int i = 0; i < count; i++) {
        const Student *s = &arr[i];
        fprintf(f, "%d|%s|%s|%lu|%s|%d|%.2f|%d|%d|%s|%s|",
                s->id, s->name, s->email, s->password_hash,
                s->dept, s->year, s->rating, s->rating_count,
                s->completed_sessions, s->avatar, s->bio);

        // Serialize offered skills: name:cat:prof:format;...
        for (int j = 0; j < s->skills_offered_count; j++) {
            fprintf(f, "%s:%s:%s:%s%s",
                    s->skills_offered[j].skill_name,
                    s->skills_offered[j].category,
                    s->skills_offered[j].proficiency,
                    s->skills_offered[j].format,
                    (j < s->skills_offered_count - 1) ? ";" : "");
        }
        fprintf(f, "|");

        // Serialize wanted skills: name;name;...
        for (int j = 0; j < s->skills_wanted_count; j++) {
            fprintf(f, "%s%s", s->skills_wanted[j], (j < s->skills_wanted_count - 1) ? ";" : "");
        }
        fprintf(f, "\n");
    }

    fclose(f);
    return true;
}

int storage_load_students(Student *arr, int max_students) {
    if (!arr || max_students <= 0) return 0;
    char path[256];
    get_filepath(path, sizeof(path), "students.dat");
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[2048];
    int count = 0;

    while (fgets(line, sizeof(line), f) && count < max_students) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        trim_whitespace(line);

        Student *s = &arr[count];
        memset(s, 0, sizeof(Student));

        char *token = strtok(line, "|");
        if (!token) continue;
        s->id = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(s->name, token, MAX_NAME_LEN - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(s->email, token, MAX_EMAIL_LEN - 1);

        token = strtok(NULL, "|");
        if (token) s->password_hash = strtoul(token, NULL, 10);

        token = strtok(NULL, "|");
        if (token) strncpy(s->dept, token, MAX_DEPT_LEN - 1);

        token = strtok(NULL, "|");
        if (token) s->year = atoi(token);

        token = strtok(NULL, "|");
        if (token) s->rating = (float)atof(token);

        token = strtok(NULL, "|");
        if (token) s->rating_count = atoi(token);

        token = strtok(NULL, "|");
        if (token) s->completed_sessions = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(s->avatar, token, 127);

        token = strtok(NULL, "|");
        if (token) strncpy(s->bio, token, MAX_BIO_LEN - 1);

        // Parse offered skills
        token = strtok(NULL, "|");
        if (token && strlen(token) > 0) {
            char offered_buf[512];
            strncpy(offered_buf, token, sizeof(offered_buf) - 1);
            char *sub = strtok(offered_buf, ";");
            while (sub && s->skills_offered_count < MAX_SKILLS_PER_STUDENT) {
                char s_name[64] = "", s_cat[32] = "", s_prof[20] = "", s_fmt[20] = "";
                sscanf(sub, "%63[^:]:%31[^:]:%19[^:]:%19[^:]", s_name, s_cat, s_prof, s_fmt);
                if (strlen(s_name) > 0) {
                    student_add_skill_offered(s, s_name, s_cat, s_prof, s_fmt);
                }
                sub = strtok(NULL, ";");
            }
        }

        // Parse wanted skills
        token = strtok(NULL, "|");
        if (token && strlen(token) > 0) {
            char wanted_buf[512];
            strncpy(wanted_buf, token, sizeof(wanted_buf) - 1);
            char *w_sub = strtok(wanted_buf, ";");
            while (w_sub && s->skills_wanted_count < MAX_SKILLS_PER_STUDENT) {
                if (strlen(w_sub) > 0) {
                    student_add_skill_wanted(s, w_sub);
                }
                w_sub = strtok(NULL, ";");
            }
        }

        count++;
    }

    fclose(f);
    return count;
}

bool storage_save_skills(const Skill *arr, int count) {
    if (!arr || count < 0) return false;
    char path[256];
    get_filepath(path, sizeof(path), "skills.dat");
    FILE *f = fopen(path, "w");
    if (!f) return false;

    fprintf(f, "# SkillSwap Skills Record File\n");
    fprintf(f, "# ID|Name|Category|MentorID|MentorName|MentorDept|MentorYear|Proficiency|Format|Rating|CompletedSessions|Popularity\n");

    for (int i = 0; i < count; i++) {
        const Skill *sk = &arr[i];
        fprintf(f, "%d|%s|%s|%d|%s|%s|%d|%s|%s|%.2f|%d|%d\n",
                sk->id, sk->name, sk->category, sk->mentor_id,
                sk->mentor_name, sk->mentor_dept, sk->mentor_year,
                sk->proficiency, sk->format, sk->rating,
                sk->completed_sessions, sk->popularity);
    }

    fclose(f);
    return true;
}

int storage_load_skills(Skill *arr, int max_skills) {
    if (!arr || max_skills <= 0) return 0;
    char path[256];
    get_filepath(path, sizeof(path), "skills.dat");
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[1024];
    int count = 0;

    while (fgets(line, sizeof(line), f) && count < max_skills) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        trim_whitespace(line);

        Skill *sk = &arr[count];
        memset(sk, 0, sizeof(Skill));

        char *token = strtok(line, "|");
        if (!token) continue;
        sk->id = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(sk->name, token, MAX_SKILL_NAME_LEN - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(sk->category, token, MAX_CATEGORY_LEN - 1);

        token = strtok(NULL, "|");
        if (token) sk->mentor_id = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(sk->mentor_name, token, MAX_NAME_LEN - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(sk->mentor_dept, token, MAX_DEPT_LEN - 1);

        token = strtok(NULL, "|");
        if (token) sk->mentor_year = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(sk->proficiency, token, MAX_PROFICIENCY_LEN - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(sk->format, token, MAX_FORMAT_LEN - 1);

        token = strtok(NULL, "|");
        if (token) sk->rating = (float)atof(token);

        token = strtok(NULL, "|");
        if (token) sk->completed_sessions = atoi(token);

        token = strtok(NULL, "|");
        if (token) sk->popularity = atoi(token);

        count++;
    }

    fclose(f);
    return count;
}

bool storage_save_requests(const LearningRequest *arr, int count) {
    if (!arr || count < 0) return false;
    char path[256];
    get_filepath(path, sizeof(path), "requests.dat");
    FILE *f = fopen(path, "w");
    if (!f) return false;

    fprintf(f, "# SkillSwap Requests Record File\n");
    fprintf(f, "# ID|RequesterID|RequesterName|MentorID|MentorName|SkillName|Message|ProposedTime|Status|CreatedAt\n");

    for (int i = 0; i < count; i++) {
        const LearningRequest *r = &arr[i];
        fprintf(f, "%d|%d|%s|%d|%s|%s|%s|%s|%s|%s\n",
                r->id, r->requester_id, r->requester_name,
                r->mentor_id, r->mentor_name, r->skill_name,
                r->message, r->proposed_time, r->status, r->created_at);
    }

    fclose(f);
    return true;
}

int storage_load_requests(LearningRequest *arr, int max_requests) {
    if (!arr || max_requests <= 0) return 0;
    char path[256];
    get_filepath(path, sizeof(path), "requests.dat");
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[1024];
    int count = 0;

    while (fgets(line, sizeof(line), f) && count < max_requests) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        trim_whitespace(line);

        LearningRequest *r = &arr[count];
        memset(r, 0, sizeof(LearningRequest));

        char *token = strtok(line, "|");
        if (!token) continue;
        r->id = atoi(token);

        token = strtok(NULL, "|");
        if (token) r->requester_id = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(r->requester_name, token, MAX_NAME_LEN - 1);

        token = strtok(NULL, "|");
        if (token) r->mentor_id = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(r->mentor_name, token, MAX_NAME_LEN - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(r->skill_name, token, MAX_SKILL_NAME_LEN - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(r->message, token, MAX_MESSAGE_LEN - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(r->proposed_time, token, 63);

        token = strtok(NULL, "|");
        if (token) strncpy(r->status, token, MAX_STATUS_LEN - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(r->created_at, token, 31);

        count++;
    }

    fclose(f);
    return count;
}

bool storage_save_sessions(const SessionRecord *arr, int count) {
    if (!arr || count < 0) return false;
    char path[256];
    get_filepath(path, sizeof(path), "sessions.dat");
    FILE *f = fopen(path, "w");
    if (!f) return false;

    fprintf(f, "# SkillSwap Sessions Record File\n");
    fprintf(f, "# ID|RequestID|MentorID|MentorName|MenteeID|MenteeName|SkillName|Date|DurationMins|Status|HasReview|Rating|Feedback\n");

    for (int i = 0; i < count; i++) {
        const SessionRecord *s = &arr[i];
        fprintf(f, "%d|%d|%d|%s|%d|%s|%s|%s|%d|%s|%d|%.2f|%s\n",
                s->id, s->request_id, s->mentor_id, s->mentor_name,
                s->mentee_id, s->mentee_name, s->skill_name,
                s->date, s->duration_mins, s->status,
                s->has_review ? 1 : 0, s->rating, s->feedback);
    }

    fclose(f);
    return true;
}

int storage_load_sessions(SessionRecord *arr, int max_sessions) {
    if (!arr || max_sessions <= 0) return 0;
    char path[256];
    get_filepath(path, sizeof(path), "sessions.dat");
    FILE *f = fopen(path, "r");
    if (!f) return 0;

    char line[1024];
    int count = 0;

    while (fgets(line, sizeof(line), f) && count < max_sessions) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;
        trim_whitespace(line);

        SessionRecord *s = &arr[count];
        memset(s, 0, sizeof(SessionRecord));

        char *token = strtok(line, "|");
        if (!token) continue;
        s->id = atoi(token);

        token = strtok(NULL, "|");
        if (token) s->request_id = atoi(token);

        token = strtok(NULL, "|");
        if (token) s->mentor_id = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(s->mentor_name, token, MAX_NAME_LEN - 1);

        token = strtok(NULL, "|");
        if (token) s->mentee_id = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(s->mentee_name, token, MAX_NAME_LEN - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(s->skill_name, token, MAX_SKILL_NAME_LEN - 1);

        token = strtok(NULL, "|");
        if (token) strncpy(s->date, token, 31);

        token = strtok(NULL, "|");
        if (token) s->duration_mins = atoi(token);

        token = strtok(NULL, "|");
        if (token) strncpy(s->status, token, MAX_STATUS_LEN - 1);

        token = strtok(NULL, "|");
        if (token) s->has_review = (atoi(token) == 1);

        token = strtok(NULL, "|");
        if (token) s->rating = (float)atof(token);

        token = strtok(NULL, "|");
        if (token) strncpy(s->feedback, token, MAX_MESSAGE_LEN - 1);

        count++;
    }

    fclose(f);
    return count;
}
