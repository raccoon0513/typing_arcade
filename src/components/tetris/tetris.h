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
void handle_typing_command(const char* input_buf, TargetWords* words, WordDifficulty current_diff);
void draw_side_panel(const TargetWords* words, int current_level);
void run_tetris(void);

#endif