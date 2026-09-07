#include <windows.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>
#include "utils.h"

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
    if (!_kbhit()) return INPUT_NONE;

    int c = _getch();

    if (c == '\r') {
        return INPUT_ENTER;
    } else if (c == '\b') {
        if (*buf_len > 0) {
            (*buf_len)--;
            buf[*buf_len] = '\0';
        }
        return INPUT_BACKSPACE;
    } else if (c >= 32 && c <= 126 && *buf_len < max_len - 1) {
        buf[(*buf_len)++] = (char)c;
        buf[*buf_len] = '\0';
        return INPUT_CHAR_ADDED;
    }

    return INPUT_NONE;
}