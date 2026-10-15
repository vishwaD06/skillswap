#include "sorting.h"

// Quicksort by rating (descending order)
static int partition_rating(Skill *arr, int low, int high) {
    float pivot = arr[high].rating;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (arr[j].rating >= pivot) { // descending
            i++;
            Skill temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    Skill temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return i + 1;
}

void quicksort_skills_by_rating(Skill *arr, int low, int high) {
    if (low < high) {
        int pi = partition_rating(arr, low, high);
        quicksort_skills_by_rating(arr, low, pi - 1);
        quicksort_skills_by_rating(arr, pi + 1, high);
    }
}

// Quicksort by name (ascending alphabetical)
static int partition_name(Skill *arr, int low, int high) {
    char pivot[MAX_SKILL_NAME_LEN];
    strncpy(pivot, arr[high].name, MAX_SKILL_NAME_LEN - 1);
    pivot[MAX_SKILL_NAME_LEN - 1] = '\0';
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (strcasecmp(arr[j].name, pivot) <= 0) {
            i++;
            Skill temp = arr[i];
            arr[i] = arr[j];
            arr[j] = temp;
        }
    }
    Skill temp = arr[i + 1];
    arr[i + 1] = arr[high];
    arr[high] = temp;
    return i + 1;
}

void quicksort_skills_by_name(Skill *arr, int low, int high) {
    if (low < high) {
        int pi = partition_name(arr, low, high);
        quicksort_skills_by_name(arr, low, pi - 1);
        quicksort_skills_by_name(arr, pi + 1, high);
    }
}

// Merge sort by student name
static void merge_by_name(Student *arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    Student *L = (Student *)malloc(sizeof(Student) * n1);
    Student *R = (Student *)malloc(sizeof(Student) * n2);
    if (!L || !R) {
        if (L) free(L);
        if (R) free(R);
        return;
    }
    
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (strcasecmp(L[i].name, R[j].name) <= 0) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    
    free(L);
    free(R);
}

void mergesort_students_by_name(Student *arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergesort_students_by_name(arr, left, mid);
        mergesort_students_by_name(arr, mid + 1, right);
        merge_by_name(arr, left, mid, right);
    }
}

// Merge sort by student rating (descending)
static void merge_by_rating(Student *arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;
    
    Student *L = (Student *)malloc(sizeof(Student) * n1);
    Student *R = (Student *)malloc(sizeof(Student) * n2);
    if (!L || !R) {
        if (L) free(L);
        if (R) free(R);
        return;
    }
    
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];
    
    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i].rating >= R[j].rating) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];
    
    free(L);
    free(R);
}

void mergesort_students_by_rating(Student *arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergesort_students_by_rating(arr, left, mid);
        mergesort_students_by_rating(arr, mid + 1, right);
        merge_by_rating(arr, left, mid, right);
    }
}

// Binary search on sorted skills array
int binary_search_skill_by_name(const Skill *arr, int n, const char *target_name) {
    if (!arr || !target_name || n <= 0) return -1;
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int cmp = strcasecmp(arr[mid].name, target_name);
        if (cmp == 0) return mid;
        if (cmp < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}

// Binary search on student array sorted by ID
int binary_search_student_by_id(const Student *arr, int n, int target_id) {
    if (!arr || n <= 0) return -1;
    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (arr[mid].id == target_id) return mid;
        if (arr[mid].id < target_id) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return -1;
}
