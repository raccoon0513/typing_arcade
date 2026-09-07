#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

void init_console(void) {
    system("chcp 65001 > nul"); // 터미널 인코딩을 UTF-8로 고정
    hide_cursor();              // 커서 숨기기
    clear_screen();             // 화면 초기화
}

void gotoxy(int x, int y) {
    COORD pos = { (short)x, (short)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void hide_cursor(void) {
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void clear_screen(void) {
    system("cls");
}

InputResult process_async_input(char* buf, int* buf_len, int max_len) {
    if (!_kbhit()) return INPUT_NONE; //키다운 없을때 루프 재시작
    int c = _getch(); //키다운감지 및 input 저장
    if (c == '\r') { //"\r" == enter키
        return INPUT_ENTER;
    } else if (c == '\b') { //"\b" == 백스페이스키
        if (*buf_len > 0) {
            (*buf_len)--;
            buf[*buf_len] = '\0';
        }
        return INPUT_BACKSPACE;
    } else if (c >= 32 && c <= 126 && *buf_len < max_len - 1) { //[A~z]이고, 최대 입력버퍼보다 글자수가 적을 때
        //정규식 사용보다 조건으로 치환이 훨씬 빠름 -> 현재 방식 유지
        buf[(*buf_len)++] = (char)c;
        buf[*buf_len] = '\0';
        return INPUT_CHAR_ADDED; //입력창에 표시되는 문자열
    }

    return INPUT_NONE;
}