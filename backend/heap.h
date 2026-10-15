#ifndef HEAP_H
#define HEAP_H

#include "common.h"

#define MAX_HEAP_CAPACITY 256

typedef struct {
    int student_id;
    char name[MAX_NAME_LEN];
    char dept[MAX_DEPT_LEN];
    int year;
    char top_skill[MAX_SKILL_NAME_LEN];
    float rating;
    int completed_sessions;
    int reviews_count;
    float score; // Computed priority score
} MentorScore;

typedef struct {
    MentorScore data[MAX_HEAP_CAPACITY];
    int size;
} MaxHeap;

void heap_init(MaxHeap *h);
float heap_calculate_score(float rating, int completed_sessions, int reviews_count);
bool heap_insert(MaxHeap *h, const MentorScore *item);
bool heap_extract_max(MaxHeap *h, MentorScore *out_item);
MentorScore* heap_peek(MaxHeap *h);
int heap_size(const MaxHeap *h);
void heap_sort(MentorScore *arr, int n); // In-place or heap sort demonstration

#endif // HEAP_H
