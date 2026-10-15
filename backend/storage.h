#ifndef STORAGE_H
#define STORAGE_H

#include "student.h"
#include "skill.h"
#include "queue.h"
#include "dynarray.h"

void storage_set_dir(const char *dir_path);
bool storage_save_students(const Student *arr, int count);
int  storage_load_students(Student *arr, int max_students);
bool storage_save_skills(const Skill *arr, int count);
int  storage_load_skills(Skill *arr, int max_skills);
bool storage_save_requests(const LearningRequest *arr, int count);
int  storage_load_requests(LearningRequest *arr, int max_requests);
bool storage_save_sessions(const SessionRecord *arr, int count);
int  storage_load_sessions(SessionRecord *arr, int max_sessions);
void storage_seed_default_data(void);

#endif // STORAGE_H
