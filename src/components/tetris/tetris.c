// Windows console Tetris -- single-file C program for Visual Studio.
// Create an Empty Project (C++), add this file as tetris.c, Ctrl+F5.
// If precompiled headers are enabled, set them to "Not Using".
// Developer Command Prompt: cl /W4 /TC tetris.c /Fe:tetris.exe
// Type the displayed random four-letter word to execute an action instantly.
// Backspace edits, Enter clears input, Tab pauses, F2 restarts, Esc quits.
// Start with 3 skips; each cleared row restores one skip.
// (maximum 9). One in four pieces starts with a bomb. A bomb occupies one
// mino and rotates with it. Bombs on completed rows erase a clipped 3x3 area
// BEFORE those rows collapse. Blast-hit bombs do not chain-detonate.
// Gravity: 2.5 seconds initially, minimum 0.7 seconds. Use English input mode.
// No additional libraries or assets required.
// Simplified rotation kicks; this is not the official SRS rule system.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef TETRIS_TEST
#include <assert.h>
#else
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <conio.h>
#endif

#define W 10
#define H 20
#define BOMB 16
#define ACTIONS 6
#define ITEM_CAP 9

/* Each tetromino occupies a 4x4 local grid. */
static const char shapes[7][17] = {
    "...." "####" "...." "....", /* I */
    ".##." ".##." "...." "....", /* O */
    ".#.." "###." "...." "....", /* T */
    ".##." "##.." "...." "....", /* S */
    "##.." ".##." "...." "....", /* Z */
    "#..." "###." "...." "....", /* J */
    "..#." "###." "...." "...."  /* L */
};
static int board[H][W];
static int piece, nextPiece, rotation, px, py;
static int bag[7], bagIndex = 7;
static int lines, level, gameOver, paused;
static unsigned long long score;
static int bombCell, nextBombCell, skips, lastBlasts;
static int wordIndex[ACTIONS], typedLength, completedWords;
static char typed[5], notice[80];
enum { LEFT, RIGHT, ROTATE, DOWN, DROP, SKIP };
static const char *actionNames[ACTIONS] = {
    "Left", "Right", "Rotate", "Down", "Drop", "Skip"
};
/* Equal-length, distinct words prevent ambiguous automatic completion. */
static const char *words[] = {
    "book", "tree", "fish", "bird", "star", "moon", "rain", "snow",
    "wind", "fire", "blue", "gold", "pink", "ship", "boat", "frog",
    "bear", "wolf", "lion", "duck", "milk", "cake", "rice", "corn",
    "game", "play", "jump", "walk", "read", "sing", "desk", "lamp",
    "door", "road", "lake", "hill", "leaf", "rose", "sand", "wave"
};
#define WORD_COUNT ((int)(sizeof words / sizeof words[0]))

/* Bounded copy shared by MSVC and the portable logic tests. */
static void copyText(char *destination, size_t capacity, const char *source)
{
    size_t i = 0;
    if (capacity == 0) return;
    while (i + 1 < capacity && source[i] != '\0') {
        destination[i] = source[i];
        ++i;
    }
    destination[i] = '\0';
}

static void clearInput(void) { typedLength = 0; typed[0] = '\0'; }

static void refreshWord(int action)
{
    int candidate, i, valid;
    do {
        candidate = rand() % WORD_COUNT;
        valid = 1;
        for (i = 0; i < ACTIONS; ++i)
            if (wordIndex[i] == candidate) valid = 0;
    } while (!valid);
    wordIndex[action] = candidate;
}

/* Return the unrotated source index, also used for bomb attachment. */
static int sourceIndex(int type, int rot, int x, int y)
{
    int i, oldX;
    if (type == 1) rot = 0;
    for (i = 0; i < rot; ++i) {
        oldX = x; x = y; y = 3 - oldX;
    }
    return y * 4 + x;
}

static int randomBombCell(int type)
{
    int i, n = rand() % 4;
    for (i = 0; i < 16; ++i)
        if (shapes[type][i] == '#' && n-- == 0) return i;
    return -1;
}

static int activeValue(int x, int y)
{
    return (piece + 1) | (sourceIndex(piece, rotation, x, y) == bombCell ? BOMB : 0);
}

static int occupied(int type, int rot, int x, int y)
{
    return shapes[type][sourceIndex(type, rot, x, y)] == '#';
}

static int fits(int type, int rot, int x, int y)
{
    int bx, by, gx, gy;
    for (by = 0; by < 4; ++by) {
        for (bx = 0; bx < 4; ++bx) {
            if (!occupied(type, rot, bx, by)) continue;
            gx = x + bx;
            gy = y + by;
            if (gx < 0 || gx >= W || gy >= H) return 0;
            if (gy >= 0 && board[gy][gx]) return 0;
        }
    }
    return 1;
}

static int drawFromBag(void)
{
    int i, j, temp;
    if (bagIndex >= 7) {
        for (i = 0; i < 7; ++i) bag[i] = i;
        for (i = 6; i > 0; --i) {
            j = rand() % (i + 1);
            temp = bag[i]; bag[i] = bag[j]; bag[j] = temp;
        }
        bagIndex = 0;
    }
    return bag[bagIndex++];
}

static void spawn(void)
{
    piece = nextPiece;
    bombCell = nextBombCell;
    nextPiece = drawFromBag();
    nextBombCell = rand() % 4 == 0 ? randomBombCell(nextPiece) : -1;
    clearInput();
    rotation = 0; px = 3; py = 0;
    if (!fits(piece, rotation, px, py)) gameOver = 1;
}

static int clearLines(void)
{
    int full[H] = {0}, erase[H][W] = {{0}};
    int y, x, dx, dy, nx, ny, dest, count = 0;
    lastBlasts = 0;
    /* Snapshot ALL completed rows before any blast mutates the board. */
    for (y = 0; y < H; ++y) {
        full[y] = 1;
        for (x = 0; x < W; ++x)
            if (!board[y][x]) { full[y] = 0; break; }
        count += full[y];
    }
    for (y = 0; y < H; ++y) if (full[y])
        for (x = 0; x < W; ++x) if (board[y][x] & BOMB) {
            ++lastBlasts;
            for (dy = -1; dy <= 1; ++dy) for (dx = -1; dx <= 1; ++dx) {
                nx = x + dx; ny = y + dy;
                if (nx >= 0 && nx < W && ny >= 0 && ny < H) erase[ny][nx] = 1;
            }
        }
    for (y = 0; y < H; ++y) for (x = 0; x < W; ++x)
        if (erase[y][x]) board[y][x] = 0;
    dest = H - 1;
    for (y = H - 1; y >= 0; --y) if (!full[y]) {
        if (dest != y) memcpy(board[dest], board[y], sizeof board[y]);
        --dest;
    }
    while (dest >= 0) memset(board[dest--], 0, sizeof board[0]);
    return count;
}

static void lockPiece(void)
{
    static const int awards[5] = { 0, 100, 300, 500, 800 };
    int x, y, removed;
    /* Check top-out before writing any cells. */
    for (y = 0; y < 4; ++y)
        for (x = 0; x < 4; ++x)
            if (occupied(piece, rotation, x, y) && py + y < 0) {
                gameOver = 1;
                return;
            }
    for (y = 0; y < 4; ++y)
        for (x = 0; x < 4; ++x)
            if (occupied(piece, rotation, x, y))
                board[py + y][px + x] = activeValue(x, y);
    removed = clearLines();
    score += (unsigned long long)awards[removed > 4 ? 4 : removed] * (unsigned long long)level;
    lines += removed;
    skips += removed; if (skips > ITEM_CAP) skips = ITEM_CAP;
    if (removed) snprintf(notice, sizeof notice, "Cleared %d row(s), %d blast(s)! Skips +%d", removed, lastBlasts, removed);
    level = lines / 10 + 1;
    spawn();
}

static int moveDown(void)
{
    if (fits(piece, rotation, px, py + 1)) { ++py; return 1; }
    lockPiece();
    return 0;
}

static void rotatePiece(void)
{
    /* Small horizontal kicks, followed by a one-cell floor kick. */
    static const int kicks[7][2] = {
        {0, 0}, {-1, 0}, {1, 0}, {-2, 0}, {2, 0}, {0, -1}, {0, -2}
    };
    int i, target = (rotation + 1) % 4;
    if (piece == 1) return;
    for (i = 0; i < 7; ++i) {
        if (fits(piece, target, px + kicks[i][0], py + kicks[i][1])) {
            px += kicks[i][0]; py += kicks[i][1]; rotation = target;
            return;
        }
    }
}

static void resetGame(void)
{
    int i;
    memset(board, 0, sizeof board);
    skips = 3; lastBlasts = completedWords = 0;
    notice[0] = '\0'; clearInput();
    for (i = 0; i < ACTIONS; ++i) wordIndex[i] = -1;
    for (i = 0; i < ACTIONS; ++i) refreshWord(i);
    score = 0; lines = 0; level = 1;
    gameOver = 0; paused = 0; bagIndex = 7;
    nextPiece = drawFromBag();
    nextBombCell = rand() % 4 == 0 ? randomBombCell(nextPiece) : -1;
    spawn();
}

/* Return 1 only when the fall timer should restart. */
static int executeAction(int action)
{
    int distance;
    notice[0] = '\0';
    switch (action) {
    case LEFT: if (fits(piece, rotation, px - 1, py)) --px; break;
    case RIGHT: if (fits(piece, rotation, px + 1, py)) ++px; break;
    case ROTATE: rotatePiece(); break;
    case DOWN:
        if (moveDown()) ++score;
        return 1;
    case DROP:
        distance = 0;
        while (fits(piece, rotation, px, py + 1)) { ++py; ++distance; }
        score += (unsigned long long)distance * 2;
        lockPiece(); return 1;
    case SKIP:
        if (skips > 0) { --skips; spawn(); copyText(notice, sizeof notice, "Skipped current piece."); return 1; }
        copyText(notice, sizeof notice, "No skip items. Clear rows to refill."); break;
    }
    return 0;
}

static int typeKey(int key)
{
    int i, matches = 0, resetTimer;
    if (paused || gameOver) return 0;
    if (key == 8) {
        if (typedLength) typed[--typedLength] = '\0';
        return 0;
    }
    if (key == 13) { clearInput(); return 0; }
    if (key >= 'A' && key <= 'Z') key += 'a' - 'A';
    if (key < 'a' || key > 'z' || typedLength == 4) return 0;
    typed[typedLength++] = (char)key; typed[typedLength] = '\0';
    for (i = 0; i < ACTIONS; ++i) {
        if (strncmp(words[wordIndex[i]], typed, (size_t)typedLength) == 0) matches = 1;
        if (strcmp(words[wordIndex[i]], typed) == 0) {
            ++completedWords;
            resetTimer = executeAction(i);
            refreshWord(i); clearInput();
            return resetTimer;
        }
    }
    if (!matches) copyText(notice, sizeof notice, "Typo: Backspace to edit, Enter to clear.");
    else notice[0] = '\0';
    return 0;
}

#ifndef TETRIS_TEST
#define SCREEN_W 88
#define SCREEN_H 29
static HANDLE output;
static CHAR_INFO frame[SCREEN_W * SCREEN_H];
static const WORD colors[8] = {7, 11, 14, 13, 10, 12, 9, 6};

static void putText(int x, int y, const char *s, WORD color)
{
    while (*s && x < SCREEN_W) {
        if (x >= 0 && y >= 0 && y < SCREEN_H) {
            frame[y * SCREEN_W + x].Char.UnicodeChar = (WCHAR)(unsigned char)*s;
            frame[y * SCREEN_W + x].Attributes = color;
        }
        ++x; ++s;
    }
}

/* U+25CF: a solid round bomb, followed by one blank console cell. */
static void putBomb(int x, int y)
{
    putText(x, y, "  ", 14);
    if (x >= 0 && x < SCREEN_W && y >= 0 && y < SCREEN_H)
        frame[y * SCREEN_W + x].Char.UnicodeChar = L'\x25CF';
}

static void draw(void)
{
    int i, x, y, bx, by, ghostY = py;
    int visible[H][W];
    char text[96];
    COORD size = {SCREEN_W, SCREEN_H}, origin = {0, 0};
    SMALL_RECT region = {0, 0, SCREEN_W - 1, SCREEN_H - 1};
    memcpy(visible, board, sizeof visible);
    if (!gameOver) {
        while (fits(piece, rotation, px, ghostY + 1)) ++ghostY;
        for (y = 0; y < 4; ++y) for (x = 0; x < 4; ++x) {
            if (!occupied(piece, rotation, x, y)) continue;
            bx = px + x; by = ghostY + y;
            if (by >= 0 && by < H && bx >= 0 && bx < W && !visible[by][bx])
                visible[by][bx] = -1;
        }
        for (y = 0; y < 4; ++y) for (x = 0; x < 4; ++x) {
            if (!occupied(piece, rotation, x, y)) continue;
            bx = px + x; by = py + y;
            if (by >= 0 && by < H && bx >= 0 && bx < W)
                visible[by][bx] = activeValue(x, y);
        }
    }
    for (i = 0; i < SCREEN_W * SCREEN_H; ++i) {
        frame[i].Char.UnicodeChar = L' ';
        frame[i].Attributes = 7;
    }
    putText(2, 0, "T Y P I N G   T E T R I S", 15);
    putText(2, 2, "+--------------------+", 7);
    for (y = 0; y < H; ++y) {
        putText(2, y + 3, "|", 7);
        for (x = 0; x < W; ++x) {
            i = visible[y][x];
            putText(3 + x * 2, y + 3,
                i > 0 ? "[]" : (i < 0 ? "::" : " ."),
                i > 0 ? colors[i & 15] : 8);
            if (i > 0 && (i & BOMB)) putBomb(3 + x * 2, y + 3);
        }
        putText(23, y + 3, "|", 7);
    }
    putText(2, 23, "+--------------------+", 7);
    sprintf_s(text, sizeof text, "Score: %llu", score);
    putText(28, 3, text, 15);
    sprintf_s(text, sizeof text, "Lines: %d   Level: %d", lines, level);
    putText(28, 5, text, 15);
    putText(28, 7, "Next:", 15);
    for (y = 0; y < 4; ++y) for (x = 0; x < 4; ++x)
        if (occupied(nextPiece, 0, x, y)) {
            putText(30 + 2 * x, 8 + y, "[]", colors[nextPiece + 1]);
            if (y * 4 + x == nextBombCell) putBomb(30 + 2 * x, 8 + y);
        }
    putText(47, 7, "ACTION       TYPE WORD", 15);
    for (i = 0; i < ACTIONS; ++i) {
        sprintf_s(text, sizeof text, "%-10s   %s", actionNames[i], words[wordIndex[i]]);
        putText(47, 9 + i, text, 11);
    }
    sprintf_s(text, sizeof text, "Skip items: %d (max 9)", skips);
    putText(28, 17, text, 14);
    sprintf_s(text, sizeof text, "Type: %-4s_   Words: %d", typed, completedWords);
    putText(28, 19, text, 15);
    putText(28, 20, "Complete word = action; no Enter needed", 7);
    putText(28, 21, "Backspace: edit   Enter: clear input", 7);
    putText(28, 22, "Tab: pause   F2: restart   Esc: quit", 7);
    putBomb(2, 24);
    putText(4, 24, "= bomb: auto 3x3 blast on row clear   :: = landing   +1 skip per row", 8);
    putText(2, 26, notice, 14);
    if (gameOver) putText(2, 27, "GAME OVER - F2 to restart or Esc to quit.", 12);
    else if (paused) putText(2, 27, "PAUSED - Tab to continue.", 14);
    else putText(2, 27, "Use English input mode. Words change after use; input resets on a new piece.", 7);
    WriteConsoleOutputW(output, frame, size, origin, &region);
}

int run_tetris(void)
{
    HANDLE originalOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursor = {1, FALSE};
    COORD bufferSize = {SCREEN_W, SCREEN_H};
    SMALL_RECT window = {0, 0, SCREEN_W - 1, SCREEN_H - 1};
    ULONGLONG lastFall, now, interval;
    int key, running = 1;
    output = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    if (output == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Please run this program in a Windows console.\n");
        return 1;
    }
    /* Shrink the window first so that the buffer can be resized safely. */
    {
        SMALL_RECT small = {0, 0, 0, 0};
        SetConsoleWindowInfo(output, TRUE, &small);
    }
    SetConsoleScreenBufferSize(output, bufferSize);
    SetConsoleWindowInfo(output, TRUE, &window);
    SetConsoleCursorInfo(output, &cursor);
    if (!SetConsoleActiveScreenBuffer(output)) {
        CloseHandle(output);
        fprintf(stderr, "Could not activate the console screen.\n");
        return 1;
    }
    srand((unsigned int)time(NULL));
    resetGame();
    lastFall = GetTickCount64();
    while (running) {
        /* Process one event per frame so held keys cannot starve gravity. */
        if (_kbhit()) {
            key = _getch();
            if (key == 0 || key == 224) key = 256 + _getch();
            if (key == 27) running = 0;
            else if (key == 256 + 60) {
                resetGame(); lastFall = GetTickCount64();
            } else if (!gameOver && key == 9) {
                paused = !paused; lastFall = GetTickCount64();
            } else if (typeKey(key)) lastFall = GetTickCount64();
        }
        now = GetTickCount64();
        interval = level >= 13 ? 700 : (ULONGLONG)(2500 - (level - 1) * 150);
        if (!paused && !gameOver && now - lastFall >= interval) {
            moveDown(); lastFall = now;
        }
        draw();
        Sleep(16);
    }
    SetConsoleActiveScreenBuffer(originalOutput);
    CloseHandle(output);
    return 0;
}
//#else

#endif
