#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <time.h>
#include <string.h>

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

// 비트마스크를 이용한 테트로미노 정의 (7가지 블록, 4가지 회전 상태)
const unsigned short blocks[7][4] = {
    { 0x0F00, 0x2222, 0x0F00, 0x2222 }, // I
    { 0x44C0, 0x8E00, 0xC880, 0xE200 }, // J
    { 0x88C0, 0xE800, 0xC440, 0x2E00 }, // L
    { 0xCC00, 0xCC00, 0xCC00, 0xCC00 }, // O
    { 0x6C00, 0x8C40, 0x6C00, 0x8C40 }, // S
    { 0x4E00, 0x4C40, 0xE400, 0x8C80 }, // T
    { 0xC600, 0x4C80, 0xC600, 0x4C80 }  // Z
};

int board[BOARD_HEIGHT][BOARD_WIDTH] = { 0 };
int cur_x, cur_y, cur_type, cur_rot;
char input_buf[20] = { 0 };
int buf_len = 0;
int score = 0;
int game_over = 0;

// 콘솔 커서 위치 이동 (화면 깜빡임 방지용)
void gotoxy(int x, int y) {
    COORD pos = { (short)x, (short)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// 콘솔 커서 숨기기
void hide_cursor() {
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

// 충돌 검사 (1: 충돌, 0: 통과)
int check_collision(int x, int y, int type, int rot) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (blocks[type][rot] & (1 << (15 - (i * 4 + j)))) {
                int nx = x + j;
                int ny = y + i;
                if (nx < 0 || nx >= BOARD_WIDTH || ny >= BOARD_HEIGHT) return 1; // 벽이나 바닥
                if (ny >= 0 && board[ny][nx]) return 1; // 기존 블록
            }
        }
    }
    return 0;
}

// 블록을 바닥에 고정하고 줄 삭제 처리
void lock_piece() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (blocks[cur_type][cur_rot] & (1 << (15 - (i * 4 + j)))) {
                if (cur_y + i >= 0) {
                    board[cur_y + i][cur_x + j] = 1;
                }
            }
        }
    }

    // 꽉 찬 줄 확인 및 삭제
    for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
        int full = 1;
        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (!board[i][j]) { full = 0; break; }
        }
        if (full) {
            // 위쪽 블록들을 한 칸씩 내림
            for (int k = i; k > 0; k--) {
                for (int j = 0; j < BOARD_WIDTH; j++) board[k][j] = board[k - 1][j];
            }
            for (int j = 0; j < BOARD_WIDTH; j++) board[0][j] = 0;
            i++; // 내린 후 현재 줄 다시 검사
            score += 100;
        }
    }

    // 새 블록 생성
    cur_type = rand() % 7;
    cur_rot = 0;
    cur_x = 3;
    cur_y = 0;

    // 생성되자마자 충돌하면 게임 오버
    if (check_collision(cur_x, cur_y, cur_type, cur_rot)) {
        game_over = 1;
    }
}

// 화면 렌더링
void render() {
    gotoxy(0, 0);
    printf("============================\n");
    printf(" SCORE: %d\n", score);
    printf(" 명령어: left, right, turn, drop\n");
    printf(" (입력 후 Enter를 누르세요)\n");
    printf("============================\n");

    for (int i = 0; i < BOARD_HEIGHT; i++) {
        printf("<!"); // 왼쪽 벽
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
        printf("!>\n"); // 오른쪽 벽
    }
    printf("<!====================!>\n\n");
    // 입력 버퍼 출력 (공백으로 이전 글자 덮어쓰기)
    printf("현재 입력: %-15s \n", input_buf);
}

int main() {
    srand((unsigned)time(NULL));
    hide_cursor();
    system("cls");

    cur_type = rand() % 7;
    cur_rot = 0;
    cur_x = 3;
    cur_y = 0;

    DWORD last_time = GetTickCount();
    DWORD drop_delay = 800; // 블록 떨어지는 속도 (ms)

    render();

    while (!game_over) {
        DWORD current_time = GetTickCount();

        // 1. 시간 경과에 따른 자동 하강
        if (current_time - last_time > drop_delay) {
            if (!check_collision(cur_x, cur_y + 1, cur_type, cur_rot)) {
                cur_y++;
            } else {
                lock_piece();
            }
            last_time = current_time;
            render();
        }

        // 2. 단어 입력 비동기 처리
        if (_kbhit()) {
            int c = _getch();
            if (c == '\r') { // Enter 키 입력 시 명령어 평가
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
                    last_time = GetTickCount(); // 드롭 타이머 초기화
                }
                
                // 버퍼 비우기
                buf_len = 0;
                memset(input_buf, 0, sizeof(input_buf));
                render();

            } else if (c == '\b') { // Backspace (지우기) 처리
                if (buf_len > 0) {
                    buf_len--;
                    input_buf[buf_len] = '\0';
                }
                render();
            } else if (c >= 32 && c <= 126 && buf_len < 15) { // 일반 문자 입력
                input_buf[buf_len++] = (char)c;
                input_buf[buf_len] = '\0';
                render();
            }
        }
        
        // CPU 과부하 방지
        Sleep(10);
    }

    gotoxy(0, BOARD_HEIGHT + 7);
    printf("\n*** GAME OVER! 최종 점수: %d ***\n", score);
    return 0;
}