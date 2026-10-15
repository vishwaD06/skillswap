#include "stack.h"

void stack_init(Stack *s) {
    if (!s) return;
    s->top = NULL;
    s->size = 0;
}

bool stack_push(Stack *s, const ActionEntry *entry) {
    if (!s || !entry) return false;
    StackNode *node = (StackNode *)malloc(sizeof(StackNode));
    if (!node) return false;
    node->data = *entry;
    node->next = s->top;
    s->top = node;
    s->size++;
    return true;
}

bool stack_pop(Stack *s, ActionEntry *out_entry) {
    if (!s || !s->top) return false;
    StackNode *temp = s->top;
    if (out_entry) {
        *out_entry = temp->data;
    }
    s->top = s->top->next;
    free(temp);
    s->size--;
    return true;
}

ActionEntry* stack_peek(const Stack *s) {
    if (!s || !s->top) return NULL;
    return &s->top->data;
}

bool stack_is_empty(const Stack *s) {
    return (!s || s->top == NULL);
}

int stack_size(const Stack *s) {
    return s ? s->size : 0;
}

void stack_free(Stack *s) {
    if (!s) return;
    StackNode *curr = s->top;
    while (curr) {
        StackNode *next = curr->next;
        free(curr);
        curr = next;
    }
    s->top = NULL;
    s->size = 0;
}

void action_to_json(const ActionEntry *entry, char *buffer, size_t buf_size) {
    if (!entry || !buffer || buf_size == 0) return;
    snprintf(buffer, buf_size,
        "{\"id\":%d,\"type\":%d,\"type_name\":\"%s\",\"student_id\":%d,"
        "\"target_id\":%d,\"details\":\"%s\",\"timestamp\":\"%s\"}",
        entry->id, (int)entry->type, entry->type_name, entry->student_id,
        entry->target_id, entry->details, entry->timestamp);
}
