#ifndef SORTING_H
#define SORTING_H

#include "skill.h"
#include "student.h"

// Quicksort
void quicksort_skills_by_rating(Skill *arr, int low, int high);
void quicksort_skills_by_name(Skill *arr, int low, int high);

// Merge Sort
void mergesort_students_by_name(Student *arr, int left, int right);
void mergesort_students_by_rating(Student *arr, int left, int right);

// Binary Search
int binary_search_skill_by_name(const Skill *arr, int n, const char *target_name);
int binary_search_student_by_id(const Student *arr, int n, int target_id);

#endif // SORTING_H
