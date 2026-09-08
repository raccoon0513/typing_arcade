#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <conio.h> // _getch() 사용

#include "src/utils/utils.h"
#include "src/components/tetris/tetris.h"

typedef enum {
    STATE_MENU = 0,
    STATE_TETRIS,
    STATE_EXIT
} GameState;

// 매개변수 없이 깔끔하게 화면만 그리는 함수로 변경
static void render_menu(void) {
    clear_screen();
    printf("========================================\n");
    printf("        타자 연습 아케이드            \n");
    printf("========================================\n");
    printf("  1. 테트리스 게임 시작 (입력: tetris 또는 1) \n");
    printf("  2. 종료               (입력: exit 또는 2)   \n");
    printf("========================================\n");
    printf("명령어 입력 > ");
}

int main(void) {
    srand((unsigned)time(NULL));
    
    // 프로그램 시작 시 콘솔 환경 설정 한 번에 완료
    init_console();

    GameState current_state = STATE_MENU;
    char input_buf[20] = { 0 };

    while (current_state != STATE_EXIT) {
        if (current_state == STATE_MENU) {
            render_menu();

            // 동기식 입력 대기: 사용자가 입력을 마치고 엔터를 칠 때까지 여기서 멈춰있음
            if (scanf("%19s", input_buf) != 1) {
                break;
            }

            // scanf로 남은 엔터 찌꺼기(버퍼) 비우기 (테트리스 루프 진입 시 오작동 방지)
            while (getchar() != '\n');

            if (strcmp(input_buf, "tetris") == 0 || strcmp(input_buf, "1") == 0) {
                current_state = STATE_TETRIS;
            } else if (strcmp(input_buf, "exit") == 0 || strcmp(input_buf, "2") == 0) {
                current_state = STATE_EXIT;
            } else {
                printf("\n잘못된 입력입니다. 아무 키나 누르면 메뉴로 돌아갑니다...\n");
                _getch(); // 사용자가 메시지를 읽을 수 있도록 잠깐 대기
            }
        } 
        else if (current_state == STATE_TETRIS) {
            clear_screen();
            run_tetris();
            
            // 테트리스 종료 후 다시 메뉴 상태로 복귀
            current_state = STATE_MENU;
        }
    }

    clear_screen();
    printf("게임을 종료합니다.\n");
    return 0;
}