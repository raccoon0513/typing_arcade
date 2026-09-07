#ifndef TETRIS_H
#define TETRIS_H

#include "../type_config.h" 
#include "../word_db.h"

typedef struct {
    char left[MAX_BUFFER_LEN];
    char right[MAX_BUFFER_LEN];
    char rotate[MAX_BUFFER_LEN];
    char drop[MAX_BUFFER_LEN];
} TargetWords;

void refresh_target_words(TargetWords* words, WordDifficulty diff);

void run_tetris(void);

#endif