#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <conio.h>
#include <string.h>

#include "../../utils/utils.h"
#include "tetris.h"
#include "../word_db.h"

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

static const unsigned short blocks[7][4] = {
    { 0x0F00, 0x2222, 0x0F00, 0x2222 }, // I
    { 0x44C0, 0x8E00, 0xC880, 0xE200 }, // J
    { 0x88C0, 0xE800, 0xC440, 0x2E00 }, // L
    { 0xCC00, 0xCC00, 0xCC00, 0xCC00 }, // O
    { 0x6C00, 0x8C40, 0x6C00, 0x8C40 }, // S
    { 0x4E00, 0x4C40, 0xE400, 0x8C80 }, // T
    { 0xC600, 0x4C80, 0xC600, 0x4C80 }  // Z
};

static int board[BOARD_HEIGHT][BOARD_WIDTH] = { 0 };
static int cur_x, cur_y, cur_type, cur_rot;
static char input_buf[20] = { 0 };
static int buf_len = 0;
static int score = 0;
static int game_over = 0;

static int check_collision(int x, int y, int type, int rot) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (blocks[type][rot] & (1 << (15 - (i * 4 + j)))) {
                int nx = x + j;
                int ny = y + i;
                if (nx < 0 || nx >= BOARD_WIDTH || ny >= BOARD_HEIGHT) return 1;
                if (ny >= 0 && board[ny][nx]) return 1;
            }
        }
    }
    return 0;
}

static void lock_piece(void) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (blocks[cur_type][cur_rot] & (1 << (15 - (i * 4 + j)))) {
                if (cur_y + i >= 0) {
                    board[cur_y + i][cur_x + j] = 1;
                }
            }
        }
    }

    for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
        int full = 1;
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (!board[i][j]) { full = 0; break; }
        }
        if (full) {
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < BOARD_WIDTH; j++) board[k][j] = board[k - 1][j];
            }
            for (int j = 0; j < BOARD_WIDTH; j++) board[0][j] = 0;
            i++;
            score += 100;
        }
    }

    cur_type = rand() % 7;
    cur_rot = 0;
    cur_x = 3;
    cur_y = 0;

    if (check_collision(cur_x, cur_y, cur_type, cur_rot)) {
        game_over = 1;
    }
}

static void render(void) {
    gotoxy(0, 0);
    printf("============================\n");
    printf(" SCORE: %d\n", score);
    printf(" command: left, right, turn, drop, quit\n");
    printf(" (입력 후 Enter를 누르세요)\n");
    printf("============================\n");

    for (int i = 0; i < BOARD_HEIGHT; i++) {
        printf("<!");
        for (int j = 0; j < BOARD_WIDTH; j++) {
            int is_piece = 0;
            for (int pi = 0; pi < 4; pi++) {
                for (int pj = 0; pj < 4; pj++) {
                    if (blocks[cur_type][cur_rot] & (1 << (15 - (pi * 4 + pj)))) {
                        if (cur_y + pi == i && cur_x + pj == j) is_piece = 1;
                    }
                }
            }
            if (is_piece || board[i][j]) printf("[]");
            else printf(" .");
        }
        printf("!>\n");
    }
    printf("<!====================!>\n\n");
    printf("현재 입력: %-15s \n", input_buf);
}

// 입력 검사 로직 예시 (tetris.c)
void handle_typing_command(const char* input_buf, TargetWords* words, WordDifficulty current_diff) {
    if (strcmp(input_buf, words->left) == 0) {
        move_block_left();
        strcpy(words->left, get_random_word(current_diff)); // 성공 시 새 단어로 교체!
    } else if (strcmp(input_buf, words->right) == 0) {
        move_block_right();
        strcpy(words->right, get_random_word(current_diff));
    } else if (strcmp(input_buf, words->rotate) == 0) {
        rotate_block();
        strcpy(words->rotate, get_random_word(current_diff));
    } else if (strcmp(input_buf, words->drop) == 0) {
        hard_drop_block();
        strcpy(words->drop, get_random_word(current_diff));
    }
}

void refresh_target_words(TargetWords* words, WordDifficulty diff) {
    strcpy(words->left, get_random_word(diff));
    strcpy(words->right, get_random_word(diff));
    strcpy(words->rotate, get_random_word(diff));
    strcpy(words->drop, get_random_word(diff));
}


void draw_side_panel(const TargetWords* words, int current_level) {
    int start_x = 26;
    
    gotoxy(start_x, 3);
    printf("======================");
    gotoxy(start_x, 4);
    printf("   COMMAND TARGETS   ");
    gotoxy(start_x, 5);
    printf("======================");

    gotoxy(start_x, 7);
    printf("[LEFT]   : %-10s", words->left);
    
    gotoxy(start_x, 9);
    printf("[RIGHT]  : %-10s", words->right);
    
    gotoxy(start_x, 11);
    printf("[ROTATE] : %-10s", words->rotate);
    
    gotoxy(start_x, 13);
    printf("[DROP]   : %-10s", words->drop);
    
    gotoxy(start_x, 15);
    printf("======================");
    gotoxy(start_x, 16);
    printf(" CURRENT LEVEL: %d", current_level);
}

void run_tetris(void) {
    memset(board, 0, sizeof(board));
    memset(input_buf, 0, sizeof(input_buf));
    buf_len = 0;
    score = 0;
    game_over = 0;

    cur_type = rand() % 7;
    cur_rot = 0;
    cur_x = 3;
    cur_y = 0;

    clear_screen();
    render();

    DWORD last_time = GetTickCount();
    DWORD drop_delay = 800;

    while (!game_over) {
        DWORD current_time = GetTickCount();

        if (current_time - last_time > drop_delay) {
            if (!check_collision(cur_x, cur_y + 1, cur_type, cur_rot)) {
                cur_y++;
            } else {
                lock_piece();
            }
            last_time = current_time;
            render();
        }

        InputResult input_res = process_async_input(input_buf, &buf_len, sizeof(input_buf));

        if (input_res == INPUT_ENTER) {
            if (strcmp(input_buf, "left") == 0) {
                if (!check_collision(cur_x - 1, cur_y, cur_type, cur_rot)) cur_x--;
            } else if (strcmp(input_buf, "right") == 0) {
                if (!check_collision(cur_x + 1, cur_y, cur_type, cur_rot)) cur_x++;
            } else if (strcmp(input_buf, "turn") == 0) {
                int next_rot = (cur_rot + 1) % 4;
                if (!check_collision(cur_x, cur_y, cur_type, next_rot)) cur_rot = next_rot;
            } else if (strcmp(input_buf, "drop") == 0) {
                while (!check_collision(cur_x, cur_y + 1, cur_type, cur_rot)) {
                    cur_y++;
                }
                lock_piece();
                last_time = GetTickCount();
            } else if (strcmp(input_buf, "quit") == 0) {
                break;
            }

            buf_len = 0;
            memset(input_buf, 0, sizeof(input_buf));
            render();
        } else if (input_res == INPUT_CHAR_ADDED || input_res == INPUT_BACKSPACE) {
            render();
        }

        Sleep(10);
    }

    gotoxy(0, BOARD_HEIGHT + 7);
    printf("\n*** GAME OVER! 최종 점수: %d ***\n", score);
    printf("엔터를 누르면 메인 메뉴로 돌아갑니다...");
    while (_getch() != '\r');
}