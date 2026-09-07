#ifndef UTILS_H
#define UTILS_H

typedef enum {
    INPUT_NONE = 0,
    INPUT_CHAR_ADDED,
    INPUT_BACKSPACE,
    INPUT_ENTER
} InputResult;

// 화면 관련
void init_console(void); //dos창 인코딩utf-8, 커서 숨김, 화면 클리어
void gotoxy(int x, int y); //화면 커서이동(덧씌우기용)
void hide_cursor(void); //커서숨김로직
void clear_screen(void);
InputResult process_async_input(char* buf, int* buf_len, int max_len);

#endif