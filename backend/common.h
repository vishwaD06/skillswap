#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <time.h>

#define MAX_NAME_LEN 64
#define MAX_EMAIL_LEN 64
#define MAX_DEPT_LEN 32
#define MAX_BIO_LEN 256
#define MAX_SKILL_NAME_LEN 64
#define MAX_CATEGORY_LEN 32
#define MAX_PROFICIENCY_LEN 20
#define MAX_FORMAT_LEN 20
#define MAX_MESSAGE_LEN 256
#define MAX_STATUS_LEN 20
#define MAX_SKILLS_PER_STUDENT 10
#define MAX_STUDENTS 200
#define MAX_SKILLS 300
#define MAX_REQUESTS 500
#define MAX_SESSIONS 500
#define MAX_REVIEWS 500

// Utility functions
static inline void trim_whitespace(char *str) {
    if (!str) return;
    char *end;
    while (isspace((unsigned char)*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;
    end[1] = '\0';
}

static inline void to_lowercase(char *dest, const char *src) {
    if (!dest || !src) return;
    while (*src) {
        *dest = (char)tolower((unsigned char)*src);
        dest++;
        src++;
    }
    *dest = '\0';
}

// Simple deterministic hash for password (DJB2 with salt)
static inline unsigned long hash_password(const char *password) {
    unsigned long hash = 5381;
    const char *salt = "SkillSwap2026";
    int c;
    while ((c = *salt++)) {
        hash = ((hash << 5) + hash) + c;
    }
    while ((c = *password++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

#endif // COMMON_H
