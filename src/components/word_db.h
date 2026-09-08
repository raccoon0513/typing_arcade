// src/components/word_db.h

#ifndef WORD_DB_H
#define WORD_DB_H

typedef enum {
    WORD_EASY = 0,
    WORD_MEDIUM,
    WORD_HARD
} WordDifficulty;

// 기존 단어들과 첫 글자가 겹치지 않는 단어를 뽑는 함수 선언
const char* get_unique_random_word(WordDifficulty diff, const char* existing_words[], int existing_count);

#endif