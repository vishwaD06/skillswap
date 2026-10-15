#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

typedef struct {
    int id;
    int requester_id;
    char requester_name[MAX_NAME_LEN];
    int mentor_id;
    char mentor_name[MAX_NAME_LEN];
    char skill_name[MAX_SKILL_NAME_LEN];
    char message[MAX_MESSAGE_LEN];
    char proposed_time[64];
    char status[MAX_STATUS_LEN]; // PENDING, ACCEPTED, REJECTED, COMPLETED
    char created_at[32];
} LearningRequest;

typedef struct QueueNode {
    LearningRequest data;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *front;
    QueueNode *rear;
    int size;
} Queue;

void queue_init(Queue *q);
bool queue_enqueue(Queue *q, const LearningRequest *req);
bool queue_dequeue(Queue *q, LearningRequest *out_req);
LearningRequest* queue_peek(const Queue *q);
bool queue_is_empty(const Queue *q);
int queue_size(const Queue *q);
void queue_free(Queue *q);

void request_to_json(const LearningRequest *req, char *buffer, size_t buf_size);

#endif // QUEUE_H
