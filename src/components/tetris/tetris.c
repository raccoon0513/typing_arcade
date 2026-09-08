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

// 테트리스 7가지 블록 모양 비트마스크 정의
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

// 조작 단어 초기화
void init_target_words(TargetWords* words, WordDifficulty diff) {
    const char* used[4] = { "", "", "", "" };

    const char* w1 = get_unique_random_word(diff, used, 0);
    strcpy(words->left, w1);
    used[0] = words->left;

    const char* w2 = get_unique_random_word(diff, used, 1);
    strcpy(words->right, w2);
    used[1] = words->right;

    const char* w3 = get_unique_random_word(diff, used, 2);
    strcpy(words->rotate, w3);
    used[2] = words->rotate;

    const char* w4 = get_unique_random_word(diff, used, 3);
    strcpy(words->drop, w4);
}

// 충돌 검사
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

// 블록 고정 및 라인 클리어 처리
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

// 화면 렌더링
void render(const TargetWords* words) {
    gotoxy(0, 0);

    // 테트리스 게임 보드 출력
    for (int y = 0; y < BOARD_HEIGHT; y++) {
        printf("│");
        for (int x = 0; x < BOARD_WIDTH; x++) {
            int is_piece = 0;

            // 비트마스크를 이용한 렌더링 검사 (SHAPES 배열 대체)
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    if (blocks[cur_type][cur_rot] & (1 << (15 - (i * 4 + j)))) {
                        if (cur_x + j == x && cur_y + i == y) {
                            is_piece = 1;
                        }
                    }
                }
            }

            if (is_piece) {
                printf("■");
            } else if (board[y][x]) {
                printf("■");
            } else {
                printf("  ");
            }
        }
        printf("│\n");
    }

    // 보드 하단 테두리
    printf("└");
    for (int x = 0; x < BOARD_WIDTH; x++) {
        printf("──");
    }
    printf("┘\n");

    // 우측 UI 패널 출력
    int ui_x = (BOARD_WIDTH * 2) + 6;

    gotoxy(ui_x, 2);
    printf("==========================");
    gotoxy(ui_x, 3);
    printf("     [ COMMAND TARGETS ]   ");
    gotoxy(ui_x, 4);
    printf("==========================");

    gotoxy(ui_x, 6);
    printf(" [LEFT]   : %-12s", words->left);

    gotoxy(ui_x, 8);
    printf(" [RIGHT]  : %-12s", words->right);

    gotoxy(ui_x, 10);
    printf(" [ROTATE] : %-12s", words->rotate);

    gotoxy(ui_x, 12);
    printf(" [DROP]   : %-12s", words->drop);

    gotoxy(ui_x, 14);
    printf("==========================");

    // 하단 점수 및 실시간 타자 입력 버퍼 표시
    gotoxy(0, BOARD_HEIGHT + 2);
    printf("SCORE : %d          ", score);

    gotoxy(0, BOARD_HEIGHT + 4);
    printf("INPUT > %-20s", input_buf);
}

// 메인 게임 루프
void run_tetris(void) {
    memset(board, 0, sizeof(board));
    memset(input_buf, 0, sizeof(input_buf));
    buf_len = 0;
    score = 0;
    game_over = 0;

    TargetWords target_words;
    WordDifficulty current_diff = WORD_EASY;
    init_target_words(&target_words, current_diff);

    cur_type = rand() % 7;
    cur_rot = 0;
    cur_x = 3;
    cur_y = 0;

    clear_screen();
    render(&target_words);

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
            render(&target_words);
        }

        InputResult input_res = process_async_input(input_buf, &buf_len, sizeof(input_buf));

        if (input_res == INPUT_ENTER) {
            if (strcmp(input_buf, "quit") == 0) {
                break;
            }

            if (strcmp(input_buf, target_words.left) == 0) {
                if (!check_collision(cur_x - 1, cur_y, cur_type, cur_rot)) cur_x--;
                
                const char* used[3] = { target_words.right, target_words.rotate, target_words.drop };
                strcpy(target_words.left, get_unique_random_word(current_diff, used, 3));

            } else if (strcmp(input_buf, target_words.right) == 0) {
                if (!check_collision(cur_x + 1, cur_y, cur_type, cur_rot)) cur_x++;
                
                const char* used[3] = { target_words.left, target_words.rotate, target_words.drop };
                strcpy(target_words.right, get_unique_random_word(current_diff, used, 3));

            } else if (strcmp(input_buf, target_words.rotate) == 0) {
                int next_rot = (cur_rot + 1) % 4;
                if (!check_collision(cur_x, cur_y, cur_type, next_rot)) cur_rot = next_rot;
                
                const char* used[3] = { target_words.left, target_words.right, target_words.drop };
                strcpy(target_words.rotate, get_unique_random_word(current_diff, used, 3));

            } else if (strcmp(input_buf, target_words.drop) == 0) {
                while (!check_collision(cur_x, cur_y + 1, cur_type, cur_rot)) {
                    cur_y++;
                }
                lock_piece();
                last_time = GetTickCount();

                const char* used[3] = { target_words.left, target_words.right, target_words.rotate };
                strcpy(target_words.drop, get_unique_random_word(current_diff, used, 3));
            }

            buf_len = 0;
            memset(input_buf, 0, sizeof(input_buf));
            render(&target_words);

        } else if (input_res == INPUT_CHAR_ADDED || input_res == INPUT_BACKSPACE) {
            render(&target_words);
        }

        Sleep(10);
    }

    gotoxy(0, BOARD_HEIGHT + 7);
    printf("\n*** GAME OVER! 최종 점수: %d ***\n", score);
    printf("엔터를 누르면 메인 메뉴로 돌아갑니다...");
    while (_getch() != '\r');
}