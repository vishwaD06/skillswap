#include "queue.h"

void queue_init(Queue *q) {
    if (!q) return;
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

bool queue_enqueue(Queue *q, const LearningRequest *req) {
    if (!q || !req) return false;
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    if (!node) return false;
    node->data = *req;
    node->next = NULL;
    
    if (q->rear) {
        q->rear->next = node;
        q->rear = node;
    } else {
        q->front = node;
        q->rear = node;
    }
    q->size++;
    return true;
}

bool queue_dequeue(Queue *q, LearningRequest *out_req) {
    if (!q || !q->front) return false;
    QueueNode *temp = q->front;
    if (out_req) {
        *out_req = temp->data;
    }
    q->front = q->front->next;
    if (!q->front) {
        q->rear = NULL;
    }
    free(temp);
    q->size--;
    return true;
}

LearningRequest* queue_peek(const Queue *q) {
    if (!q || !q->front) return NULL;
    return &q->front->data;
}

bool queue_is_empty(const Queue *q) {
    return (!q || q->front == NULL);
}

int queue_size(const Queue *q) {
    return q ? q->size : 0;
}

void queue_free(Queue *q) {
    if (!q) return;
    QueueNode *curr = q->front;
    while (curr) {
        QueueNode *next = curr->next;
        free(curr);
        curr = next;
    }
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

void request_to_json(const LearningRequest *req, char *buffer, size_t buf_size) {
    if (!req || !buffer || buf_size == 0) return;
    snprintf(buffer, buf_size,
        "{\"id\":%d,\"requester_id\":%d,\"requester_name\":\"%s\","
        "\"mentor_id\":%d,\"mentor_name\":\"%s\",\"skill_name\":\"%s\","
        "\"message\":\"%s\",\"proposed_time\":\"%s\",\"status\":\"%s\",\"created_at\":\"%s\"}",
        req->id, req->requester_id, req->requester_name,
        req->mentor_id, req->mentor_name, req->skill_name,
        req->message, req->proposed_time, req->status, req->created_at);
}
