#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "student.h"

typedef struct LLNode {
    Student data;
    struct LLNode *next;
} LLNode;

typedef struct {
    LLNode *head;
    LLNode *tail;
    int size;
} LinkedList;

void ll_init(LinkedList *list);
bool ll_insert_head(LinkedList *list, const Student *student);
bool ll_insert_tail(LinkedList *list, const Student *student);
bool ll_delete_by_id(LinkedList *list, int student_id);
Student* ll_search_by_id(LinkedList *list, int student_id);
Student* ll_search_by_email(LinkedList *list, const char *email);
void ll_display(const LinkedList *list);
void ll_free(LinkedList *list);

#endif // LINKED_LIST_H
