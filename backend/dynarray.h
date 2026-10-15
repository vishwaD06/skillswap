#ifndef DYNARRAY_H
#define DYNARRAY_H

#include "common.h"

typedef struct {
    int id;
    int request_id;
    int mentor_id;
    char mentor_name[MAX_NAME_LEN];
    int mentee_id;
    char mentee_name[MAX_NAME_LEN];
    char skill_name[MAX_SKILL_NAME_LEN];
    char date[32];
    int duration_mins;
    char status[MAX_STATUS_LEN]; // COMPLETED, SCHEDULED
    bool has_review;
    float rating;
    char feedback[MAX_MESSAGE_LEN];
} SessionRecord;

typedef struct {
    SessionRecord *data;
    int size;
    int capacity;
} SessionDynArray;

SessionDynArray* dynarray_create(int initial_capacity);
bool dynarray_push(SessionDynArray *da, const SessionRecord *item);
SessionRecord* dynarray_get(SessionDynArray *da, int index);
int dynarray_size(const SessionDynArray *da);
void dynarray_free(SessionDynArray *da);
void session_to_json(const SessionRecord *s, char *buffer, size_t buf_size);

#endif // DYNARRAY_H
