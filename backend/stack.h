#ifndef STACK_H
#define STACK_H

#include "common.h"

typedef enum {
    ACTION_ADD_SKILL = 1,
    ACTION_REMOVE_SKILL,
    ACTION_SEND_REQUEST,
    ACTION_CANCEL_REQUEST,
    ACTION_UPDATE_BIO,
    ACTION_SUBMIT_REVIEW
} ActionType;

typedef struct {
    int id;
    ActionType type;
    char type_name[32];
    int student_id;
    int target_id;
    char details[128];
    char timestamp[32];
} ActionEntry;

typedef struct StackNode {
    ActionEntry data;
    struct StackNode *next;
} StackNode;

typedef struct {
    StackNode *top;
    int size;
} Stack;

void stack_init(Stack *s);
bool stack_push(Stack *s, const ActionEntry *entry);
bool stack_pop(Stack *s, ActionEntry *out_entry);
ActionEntry* stack_peek(const Stack *s);
bool stack_is_empty(const Stack *s);
int stack_size(const Stack *s);
void stack_free(Stack *s);
void action_to_json(const ActionEntry *entry, char *buffer, size_t buf_size);

#endif // STACK_H
