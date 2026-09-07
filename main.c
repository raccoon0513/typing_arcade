#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "src/utils/utils.h"
#include "src/components/tetris/tetris.h"

typedef enum {
    STATE_MENU = 0,
    STATE_TETRIS,
    STATE_EXIT
} GameState;

static void render_menu(const char* input_buf) {
    gotoxy(0, 0);
    printf("========================================\n");
    printf("         타자 연습 아케이드            \n");
    printf("========================================\n");
    printf("  1. 테트리스 게임 시작 (입력: tetris)  \n");
    printf("  2. 종료               (입력: exit)    \n");
    printf("========================================\n");
    printf("명령어 입력: %-15s \n", input_buf);
}

int main(void) {
    srand((unsigned)time(NULL));
    
    // 프로그램 시작 시 콘솔 환경 설정 한 번에 완료
    init_console();

    GameState current_state = STATE_MENU;
    char input_buf[20] = { 0 };
    int buf_len = 0;

    while (current_state != STATE_EXIT) {
        if (current_state == STATE_MENU) {
            render_menu(input_buf);

            InputResult res = process_async_input(input_buf, &buf_len, sizeof(input_buf));

            if (res == INPUT_ENTER) {
                if (strcmp(input_buf, "tetris") == 0 || strcmp(input_buf, "1") == 0) {
                    current_state = STATE_TETRIS;
                } else if (strcmp(input_buf, "exit") == 0 || strcmp(input_buf, "2") == 0) {
                    current_state = STATE_EXIT;
                }

                buf_len = 0;
                memset(input_buf, 0, sizeof(input_buf));
                clear_screen();
            } else if (res == INPUT_CHAR_ADDED || res == INPUT_BACKSPACE) {
                render_menu(input_buf);
            }
        } else if (current_state == STATE_TETRIS) {
            run_tetris();
            current_state = STATE_MENU;
            clear_screen();
        }

        Sleep(10);
    }

    clear_screen();
    printf("게임을 종료합니다.\n");
    return 0;
}