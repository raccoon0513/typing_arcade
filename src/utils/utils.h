#ifndef UTILS_H
#define UTILS_H

typedef enum {
    INPUT_NONE = 0,
    INPUT_CHAR_ADDED,
    INPUT_BACKSPACE,
    INPUT_ENTER
} InputResult;

// 콘솔 환경 초기화 (UTF-8 설정, 커서 숨김 등)
void init_console(void);

void gotoxy(int x, int y);
void hide_cursor(void);
void clear_screen(void);
InputResult process_async_input(char* buf, int* buf_len, int max_len);

#endif /* UTILS_H */