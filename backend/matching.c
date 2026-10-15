#include "matching.h"

int find_recommendations_for_student(const Student *target_student, 
                                     const Student *all_students, int num_students,
                                     const Graph *network_graph,
                                     MatchRecommendation *results, int max_results) {
    if (!target_student || !all_students || !results || max_results <= 0) return 0;
    
    int count = 0;
    
    for (int i = 0; i < num_students && count < max_results; i++) {
        const Student *other = &all_students[i];
        if (other->id == target_student->id) continue; // Don't match self
        
        // 1. Check if other student offers any skill that target_student wants
        int matching_offered_idx = -1;
        for (int o = 0; o < other->skills_offered_count; o++) {
            for (int w = 0; w < target_student->skills_wanted_count; w++) {
                if (strcasecmp(other->skills_offered[o].skill_name, target_student->skills_wanted[w]) == 0) {
                    matching_offered_idx = o;
                    break;
                }
            }
            if (matching_offered_idx != -1) break;
        }
        
        // If other student teaches what target wants (or if target wanted is empty, fallback to other's top skill)
        if (matching_offered_idx == -1 && target_student->skills_wanted_count > 0) {
            continue; // Not a match for current wanted skills
        }
        
        if (matching_offered_idx == -1 && other->skills_offered_count > 0) {
            matching_offered_idx = 0; // Take their first skill
        }
        
        if (matching_offered_idx == -1) continue;
        
        // 2. Check if this is a mutual 2-way swap:
        // Does target_student offer any skill that other student wants?
        bool is_mutual = false;
        char mutual_skill[MAX_SKILL_NAME_LEN] = "";
        for (int to = 0; to < target_student->skills_offered_count; to++) {
            for (int ow = 0; ow < other->skills_wanted_count; ow++) {
                if (strcasecmp(target_student->skills_offered[to].skill_name, other->skills_wanted[ow]) == 0) {
                    is_mutual = true;
                    strncpy(mutual_skill, target_student->skills_offered[to].skill_name, MAX_SKILL_NAME_LEN - 1);
                    break;
                }
            }
            if (is_mutual) break;
        }
        
        // 3. Compute Match Score
        int score = 40; // Base match for offering wanted skill
        if (is_mutual) score += 35; // Perfect 2-way exchange!
        score += (int)(other->rating * 3.0f); // Up to 15 points
        if (other->completed_sessions > 0) score += 5;
        if (strcasecmp(other->dept, target_student->dept) == 0) score += 5; // Same campus department
        if (score > 100) score = 100;
        
        MatchRecommendation *rec = &results[count++];
        rec->mentor_id = other->id;
        strncpy(rec->mentor_name, other->name, MAX_NAME_LEN - 1);
        strncpy(rec->mentor_dept, other->dept, MAX_DEPT_LEN - 1);
        rec->mentor_year = other->year;
        strncpy(rec->skill_offered, other->skills_offered[matching_offered_idx].skill_name, MAX_SKILL_NAME_LEN - 1);
        strncpy(rec->proficiency, other->skills_offered[matching_offered_idx].proficiency, MAX_PROFICIENCY_LEN - 1);
        rec->mentor_rating = other->rating;
        rec->completed_sessions = other->completed_sessions;
        rec->is_mutual_swap = is_mutual;
        strncpy(rec->mutual_skill_they_want, mutual_skill, MAX_SKILL_NAME_LEN - 1);
        rec->match_score = score;
        
        if (is_mutual) {
            snprintf(rec->reason, sizeof(rec->reason), "Mutual Swap: Teaches %s, wants your %s!", 
                     rec->skill_offered, mutual_skill);
        } else {
            snprintf(rec->reason, sizeof(rec->reason), "Direct Match: Expert in %s (Rating: %.1f)", 
                     rec->skill_offered, other->rating);
        }
    }
    
    // Sort recommendations by match_score descending
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (results[j].match_score < results[j + 1].match_score) {
                MatchRecommendation temp = results[j];
                results[j] = results[j + 1];
                results[j + 1] = temp;
            }
        }
    }
    
    return count;
}

void recommendation_to_json(const MatchRecommendation *m, char *buffer, size_t buf_size) {
    if (!m || !buffer || buf_size == 0) return;
    snprintf(buffer, buf_size,
        "{\"mentor_id\":%d,\"mentor_name\":\"%s\",\"mentor_dept\":\"%s\",\"mentor_year\":%d,"
        "\"skill_offered\":\"%s\",\"proficiency\":\"%s\",\"mentor_rating\":%.2f,"
        "\"completed_sessions\":%d,\"is_mutual_swap\":%s,\"mutual_skill_they_want\":\"%s\","
        "\"match_score\":%d,\"reason\":\"%s\"}",
        m->mentor_id, m->mentor_name, m->mentor_dept, m->mentor_year,
        m->skill_offered, m->proficiency, m->mentor_rating,
        m->completed_sessions, m->is_mutual_swap ? "true" : "false",
        m->mutual_skill_they_want, m->match_score, m->reason);
}
