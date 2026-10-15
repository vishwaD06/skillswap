#include "linked_list.h"

void ll_init(LinkedList *list) {
    if (!list) return;
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

bool ll_insert_head(LinkedList *list, const Student *student) {
    if (!list || !student) return false;
    LLNode *node = (LLNode *)malloc(sizeof(LLNode));
    if (!node) return false;
    node->data = *student;
    node->next = list->head;
    list->head = node;
    if (!list->tail) {
        list->tail = node;
    }
    list->size++;
    return true;
}

bool ll_insert_tail(LinkedList *list, const Student *student) {
    if (!list || !student) return false;
    LLNode *node = (LLNode *)malloc(sizeof(LLNode));
    if (!node) return false;
    node->data = *student;
    node->next = NULL;
    
    if (list->tail) {
        list->tail->next = node;
        list->tail = node;
    } else {
        list->head = node;
        list->tail = node;
    }
    list->size++;
    return true;
}

bool ll_delete_by_id(LinkedList *list, int student_id) {
    if (!list || !list->head) return false;
    LLNode *curr = list->head;
    LLNode *prev = NULL;
    
    while (curr) {
        if (curr->data.id == student_id) {
            if (prev) {
                prev->next = curr->next;
            } else {
                list->head = curr->next;
            }
            if (curr == list->tail) {
                list->tail = prev;
            }
            free(curr);
            list->size--;
            return true;
        }
        prev = curr;
        curr = curr->next;
    }
    return false;
}

Student* ll_search_by_id(LinkedList *list, int student_id) {
    if (!list) return NULL;
    LLNode *curr = list->head;
    while (curr) {
        if (curr->data.id == student_id) {
            return &curr->data;
        }
        curr = curr->next;
    }
    return NULL;
}

Student* ll_search_by_email(LinkedList *list, const char *email) {
    if (!list || !email) return NULL;
    LLNode *curr = list->head;
    while (curr) {
        if (strcasecmp(curr->data.email, email) == 0) {
            return &curr->data;
        }
        curr = curr->next;
    }
    return NULL;
}

void ll_display(const LinkedList *list) {
    if (!list) return;
    printf("[Linked List] Size: %d\n", list->size);
    LLNode *curr = list->head;
    int index = 0;
    while (curr) {
        printf("  Node %d: ID=%d, Name=%s, Dept=%s, Rating=%.2f\n",
               index++, curr->data.id, curr->data.name, curr->data.dept, curr->data.rating);
        curr = curr->next;
    }
}

void ll_free(LinkedList *list) {
    if (!list) return;
    LLNode *curr = list->head;
    while (curr) {
        LLNode *next = curr->next;
        free(curr);
        curr = next;
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}
