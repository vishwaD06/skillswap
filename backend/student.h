#ifndef STUDENT_H
#define STUDENT_H

#include "common.h"

typedef struct {
    char skill_name[MAX_SKILL_NAME_LEN];
    char category[MAX_CATEGORY_LEN];
    char proficiency[MAX_PROFICIENCY_LEN]; // Beginner, Intermediate, Advanced, Expert
    char format[MAX_FORMAT_LEN];           // 1-on-1, Online, In-Person, Flexible
} SkillOffered;

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char email[MAX_EMAIL_LEN];
    unsigned long password_hash;
    char dept[MAX_DEPT_LEN];
    int year;
    char bio[MAX_BIO_LEN];
    char avatar[128];
    float rating;
    int rating_count;
    int completed_sessions;
    
    int skills_offered_count;
    SkillOffered skills_offered[MAX_SKILLS_PER_STUDENT];
    
    int skills_wanted_count;
    char skills_wanted[MAX_SKILLS_PER_STUDENT][MAX_SKILL_NAME_LEN];
} Student;

// Student manager functions
void student_init(Student *s, int id, const char *name, const char *email, 
                  const char *password, const char *dept, int year, 
                  const char *bio, const char *avatar);

bool student_verify_password(const Student *s, const char *password);

bool student_add_skill_offered(Student *s, const char *skill_name, 
                               const char *category, const char *proficiency, 
                               const char *format);

bool student_remove_skill_offered(Student *s, const char *skill_name);

bool student_add_skill_wanted(Student *s, const char *skill_name);

bool student_remove_skill_wanted(Student *s, const char *skill_name);

void student_record_session_rating(Student *s, float new_rating);

void student_to_json(const Student *s, char *buffer, size_t buf_size);

#endif // STUDENT_H
