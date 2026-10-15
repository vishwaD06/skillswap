#include "heap.h"

void heap_init(MaxHeap *h) {
    if (!h) return;
    h->size = 0;
}

float heap_calculate_score(float rating, int completed_sessions, int reviews_count) {
    // Formula: Rating (out of 5) * 20 (up to 100 pts) + completed sessions * 10 pts + reviews * 5 pts
    return (rating * 20.0f) + ((float)completed_sessions * 10.0f) + ((float)reviews_count * 5.0f);
}

static void swap_nodes(MentorScore *a, MentorScore *b) {
    MentorScore temp = *a;
    *a = *b;
    *b = temp;
}

static void heapify_up(MaxHeap *h, int index) {
    while (index > 0) {
        int parent = (index - 1) / 2;
        if (h->data[index].score > h->data[parent].score) {
            swap_nodes(&h->data[index], &h->data[parent]);
            index = parent;
        } else {
            break;
        }
    }
}

static void heapify_down(MaxHeap *h, int index) {
    int size = h->size;
    while (index < size) {
        int left = 2 * index + 1;
        int right = 2 * index + 2;
        int largest = index;
        
        if (left < size && h->data[left].score > h->data[largest].score) {
            largest = left;
        }
        if (right < size && h->data[right].score > h->data[largest].score) {
            largest = right;
        }
        
        if (largest != index) {
            swap_nodes(&h->data[index], &h->data[largest]);
            index = largest;
        } else {
            break;
        }
    }
}

bool heap_insert(MaxHeap *h, const MentorScore *item) {
    if (!h || !item || h->size >= MAX_HEAP_CAPACITY) return false;
    int index = h->size;
    h->data[index] = *item;
    h->size++;
    heapify_up(h, index);
    return true;
}

bool heap_extract_max(MaxHeap *h, MentorScore *out_item) {
    if (!h || h->size <= 0) return false;
    if (out_item) {
        *out_item = h->data[0];
    }
    h->data[0] = h->data[h->size - 1];
    h->size--;
    if (h->size > 0) {
        heapify_down(h, 0);
    }
    return true;
}

MentorScore* heap_peek(MaxHeap *h) {
    if (!h || h->size <= 0) return NULL;
    return &h->data[0];
}

int heap_size(const MaxHeap *h) {
    return h ? h->size : 0;
}

// In-place heap sort implementation
void heap_sort(MentorScore *arr, int n) {
    if (!arr || n <= 1) return;
    MaxHeap h;
    heap_init(&h);
    for (int i = 0; i < n; i++) {
        heap_insert(&h, &arr[i]);
    }
    for (int i = 0; i < n; i++) {
        heap_extract_max(&h, &arr[i]); // Arranged in descending order
    }
}
