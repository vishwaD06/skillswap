#ifndef MATCHING_H
#define MATCHING_H

#include "student.h"
#include "skill.h"
#include "graph.h"

typedef struct {
    int mentor_id;
    char mentor_name[MAX_NAME_LEN];
    char mentor_dept[MAX_DEPT_LEN];
    int mentor_year;
    char skill_offered[MAX_SKILL_NAME_LEN];
    char proficiency[MAX_PROFICIENCY_LEN];
    float mentor_rating;
    int completed_sessions;
    bool is_mutual_swap;              // Both students teach what the other wants!
    char mutual_skill_they_want[MAX_SKILL_NAME_LEN];
    int match_score;                  // 0 to 100 percentage
    char reason[128];
} MatchRecommendation;

int find_recommendations_for_student(const Student *target_student, 
                                     const Student *all_students, int num_students,
                                     const Graph *network_graph,
                                     MatchRecommendation *results, int max_results);

void recommendation_to_json(const MatchRecommendation *m, char *buffer, size_t buf_size);

#endif // MATCHING_H
