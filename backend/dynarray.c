#include "dynarray.h"

SessionDynArray* dynarray_create(int initial_capacity) {
    if (initial_capacity <= 0) initial_capacity = 16;
    SessionDynArray *da = (SessionDynArray *)malloc(sizeof(SessionDynArray));
    if (!da) return NULL;
    da->data = (SessionRecord *)malloc(sizeof(SessionRecord) * initial_capacity);
    if (!da->data) {
        free(da);
        return NULL;
    }
    da->size = 0;
    da->capacity = initial_capacity;
    return da;
}

bool dynarray_push(SessionDynArray *da, const SessionRecord *item) {
    if (!da || !item) return false;
    if (da->size >= da->capacity) {
        int new_capacity = da->capacity * 2;
        SessionRecord *new_data = (SessionRecord *)realloc(da->data, sizeof(SessionRecord) * new_capacity);
        if (!new_data) return false;
        da->data = new_data;
        da->capacity = new_capacity;
    }
    da->data[da->size] = *item;
    da->size++;
    return true;
}

SessionRecord* dynarray_get(SessionDynArray *da, int index) {
    if (!da || index < 0 || index >= da->size) return NULL;
    return &da->data[index];
}

int dynarray_size(const SessionDynArray *da) {
    return da ? da->size : 0;
}

void dynarray_free(SessionDynArray *da) {
    if (!da) return;
    if (da->data) {
        free(da->data);
    }
    free(da);
}

void session_to_json(const SessionRecord *s, char *buffer, size_t buf_size) {
    if (!s || !buffer || buf_size == 0) return;
    snprintf(buffer, buf_size,
        "{\"id\":%d,\"request_id\":%d,\"mentor_id\":%d,\"mentor_name\":\"%s\","
        "\"mentee_id\":%d,\"mentee_name\":\"%s\",\"skill_name\":\"%s\","
        "\"date\":\"%s\",\"duration_mins\":%d,\"status\":\"%s\","
        "\"has_review\":%s,\"rating\":%.2f,\"feedback\":\"%s\"}",
        s->id, s->request_id, s->mentor_id, s->mentor_name,
        s->mentee_id, s->mentee_name, s->skill_name,
        s->date, s->duration_mins, s->status,
        s->has_review ? "true" : "false", s->rating, s->feedback);
}
