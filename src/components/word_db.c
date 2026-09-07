#include "word_db.h"
#include <stdlib.h>

// 길이/난이도별 단어 목록
static const char* EASY_WORDS[] = {
    "a", "in", "go", "up", "at", "to", "on"
};

static const char* MEDIUM_WORDS[] = {
    "left", "down", "drop", "turn", "move", "fall", "fast"
};

static const char* HARD_WORDS[] = {
    "rotate", "center", "action", "tetris", "remove"
};

const char* get_random_word(WordDifficulty diff) {
    switch (diff) {
        case WORD_EASY: {
            int count = sizeof(EASY_WORDS) / sizeof(EASY_WORDS[0]);
            return EASY_WORDS[rand() % count];
        }
        case WORD_MEDIUM: {
            int count = sizeof(MEDIUM_WORDS) / sizeof(MEDIUM_WORDS[0]);
            return MEDIUM_WORDS[rand() % count];
        }
        case WORD_HARD: {
            int count = sizeof(HARD_WORDS) / sizeof(HARD_WORDS[0]);
            return HARD_WORDS[rand() % count];
        }
        default:
            return "go";
    }
}