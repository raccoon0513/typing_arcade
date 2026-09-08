// src/components/word_db.c

#include "word_db.h"
#include <stdlib.h>
#include <string.h>

static const char* EASY_WORDS[] = { "go", "up", "at", "to", "on", "in", "by", "hi" };
static const char* MEDIUM_WORDS[] = { "left", "down", "drop", "turn", "move", "fall", "fast", "jump" };
static const char* HARD_WORDS[] = { "rotate", "center", "action", "tetris", "remove", "stream" };

// 두 단어의 첫 글자(접두어)가 같은지 검사
static int is_prefix_conflict(const char* word1, const char* word2) {
    if (!word1 || !word2 || strlen(word1) == 0 || strlen(word2) == 0) return 0;
    // 첫 글자가 같으면 충돌로 판단 (ice vs icecream 패턴 방지)
    return word1[0] == word2[0];
}

// 기존 사용 중인 단어들과 첫 글자가 겹치지 않는 단어를 랜덤 추출
const char* get_unique_random_word(WordDifficulty diff, const char* existing_words[], int existing_count) {
    const char** pool;
    int pool_size = 0;

    switch (diff) {
        case WORD_EASY:
            pool = EASY_WORDS;
            pool_size = sizeof(EASY_WORDS) / sizeof(EASY_WORDS[0]);
            break;
        case WORD_MEDIUM:
            pool = MEDIUM_WORDS;
            pool_size = sizeof(MEDIUM_WORDS) / sizeof(MEDIUM_WORDS[0]);
            break;
        case WORD_HARD:
            pool = HARD_WORDS;
            pool_size = sizeof(HARD_WORDS) / sizeof(HARD_WORDS[0]);
            break;
        default:
            pool = EASY_WORDS;
            pool_size = sizeof(EASY_WORDS) / sizeof(EASY_WORDS[0]);
            break;
    }

    // 시도 횟수 제한 (무한 루프 방지)
    for (int attempt = 0; attempt < 100; attempt++) {
        const char* candidate = pool[rand() % pool_size];
        int conflict = 0;

        for (int i = 0; i < existing_count; i++) {
            if (existing_words[i] && strlen(existing_words[i]) > 0) {
                if (is_prefix_conflict(candidate, existing_words[i])) {
                    conflict = 1;
                    break;
                }
            }
        }

        if (!conflict) {
            return candidate;
        }
    }

    // 충돌 안 나는 단어를 못 찾았을 경우 기본 후보 반환
    return pool[rand() % pool_size];
}