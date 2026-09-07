#ifndef WORD_DB_H
#define WORD_DB_H

// 난이도 및 단어 길이 정의
typedef enum {
    WORD_EASY = 0,   // 2~3글자
    WORD_MEDIUM,     // 4글자
    WORD_HARD        // 5글자 이상
} WordDifficulty;

// 해당 난이도의 랜덤 단어를 반환하는 함수
const char* get_random_word(WordDifficulty diff);

#endif