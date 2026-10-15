#ifndef SKILL_H
#define SKILL_H

#include "common.h"

typedef struct {
    int id;
    char name[MAX_SKILL_NAME_LEN];
    char category[MAX_CATEGORY_LEN];
    int mentor_id;
    char mentor_name[MAX_NAME_LEN];
    char mentor_dept[MAX_DEPT_LEN];
    int mentor_year;
    char proficiency[MAX_PROFICIENCY_LEN];
    char format[MAX_FORMAT_LEN];
    float rating;
    int completed_sessions;
    int popularity;
} Skill;

void skill_init(Skill *sk, int id, const char *name, const char *category,
                int mentor_id, const char *mentor_name, const char *mentor_dept,
                int mentor_year, const char *proficiency, const char *format,
                float rating, int completed_sessions, int popularity);

void skill_to_json(const Skill *sk, char *buffer, size_t buf_size);

#endif // SKILL_H
